/* Demo application for Computer Vision Library.
 * @file
 * @date 2018-09-05
 * @author Anonymous
 */

#include <cvlib.hpp>
#include <opencv2/opencv.hpp>

int demo_motion_segmentation(int argc, char* argv[])
{
	//cv::VideoCapture cap(0);
	cv::VideoCapture cap("IMG_0243.MOV");
    if (!cap.isOpened())
        return -1;

    //auto mseg = cv::createBackgroundSubtractorMOG2(); // \todo use cvlib::motion_segmentation
	auto mseg = cvlib::motion_segmentation();
    const auto main_wnd = "orig";
    const auto demo_wnd = "demo";

    int threshold = 200;
	int last_threshold = threshold;
    cv::namedWindow(main_wnd);
    cv::namedWindow(demo_wnd);
    cv::createTrackbar("th", demo_wnd, &threshold, 500);

    cv::Mat frame;
    cv::Mat frame_mseg;
    while (cv::waitKey(30) != 27) // ESC
    {
        cap >> frame;
		cv::resize(frame, frame, cv::Size(), 1.0 / 4, 1.0 / 4);
        cv::imshow(main_wnd, frame);
		if (threshold != last_threshold && threshold != 0)
		{
			mseg.setVarThreshold(threshold);
			//mseg.reinit();
			last_threshold = threshold;
		}
        mseg.apply(frame, frame_mseg);
        if (!frame_mseg.empty())
            cv::imshow(demo_wnd, frame_mseg);
    }

    cv::destroyWindow(main_wnd);
    cv::destroyWindow(demo_wnd);

    return 0;
}
