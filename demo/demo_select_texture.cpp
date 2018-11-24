/* Demo application for Computer Vision Library.
 * @file
 * @date 2018-09-05
 * @author Anonymous
 */

#include <opencv2/opencv.hpp>
#include "../cvlib/src/select_texture.cpp"
#include <cvlib.hpp>

namespace
{
void mouse(int event, int x, int y, int flags, void* param)
{
    user_data& data = *reinterpret_cast<user_data*>(param);
	
    if (event == CV_EVENT_LBUTTONDOWN)
    {
        data.tl = {x, y};
    }
    else if (event == CV_EVENT_RBUTTONDOWN)
    {
        data.br = {x, y};
    }

	if (event == CV_EVENT_RBUTTONDOWN || event == CV_EVENT_LBUTTONDOWN)
	{
		// calc descriptor
		const cv::Rect roi = { data.tl, data.br };
		int kernel_size = std::min(roi.height, roi.width) / 2;
		if (kernel_size % 2 == 0)
			kernel_size -= 1;
		// create Gabor filters
		data.filters.clear();
		createFilters(data.filters, kernel_size);
		// eval convolution
		std::vector<cv::Mat> intConv(data.filters.size());
		std::vector<cv::Mat> intSqConv(data.filters.size());
		evalFilters(data.image, data.filters, intConv, intSqConv);
	}
}
} // namespace

int demo_select_texture(int argc, char* argv[])
{
	//cv::VideoCapture cap(0);
	cv::VideoCapture cap("video (8).mp4");
    if (!cap.isOpened())
        return -1;

    user_data data;

    data.wnd = "origin";
    const auto demo_wnd = "demo";

    int eps = 1;
    cv::namedWindow(data.wnd);
    cv::namedWindow(demo_wnd);
    cv::createTrackbar("eps", demo_wnd, &eps, 200);

    cv::setMouseCallback(data.wnd, mouse, &data);

    cv::Mat frame_gray;
    while (cv::waitKey(30) != 27) // ESC
    {
        cap >> data.image;

		if (data.image.empty())
			break;

        cv::cvtColor(data.image, frame_gray, cv::COLOR_BGR2GRAY);
        const cv::Rect roi = {data.tl, data.br};
        if (roi.area())
        {
            const auto mask = cvlib::select_texture(frame_gray, roi, eps, &data);
            const auto segmented = mask.clone();
            frame_gray.copyTo(segmented, mask);
            cv::imshow(demo_wnd, segmented);
            cv::rectangle(data.image, data.tl, data.br, cv::Scalar(0, 0, 255));
        }
		
        cv::imshow(data.wnd, data.image);
    }

    cv::destroyWindow(data.wnd);
    cv::destroyWindow(demo_wnd);

    return 0;
}
