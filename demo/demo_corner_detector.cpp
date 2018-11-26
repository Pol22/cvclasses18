/* Demo application for Computer Vision Library.
 * @file
 * @date 2018-09-05
 * @author Anonymous
 */

#include <cvlib.hpp>
#include <opencv2/opencv.hpp>

#include "utils.hpp"

int demo_corner_detector(int argc, char* argv[])
{
    //cv::VideoCapture cap(0);
	cv::VideoCapture cap("video (8).mp4");
    if (!cap.isOpened())
        return -1;

    const auto main_wnd = "orig";
    const auto demo_wnd = "demo";

    cv::namedWindow(main_wnd);
    cv::namedWindow(demo_wnd);

	cv::Mat frame, frame_gray;
	auto detector = cvlib::corner_detector_fast::create();
    std::vector<cv::KeyPoint> corners;

    utils::fps_counter fps;
    while (cv::waitKey(1) != 27) // ESC
    {
        cap >> frame;
        cv::imshow(main_wnd, frame);
		cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
        detector->detect(frame_gray, corners);
        cv::drawKeypoints(frame, corners, frame, cv::Scalar(0, 0, 255));
        utils::put_fps_text(frame, fps);
		utils::put_number_of_keypoints(frame, corners.size());
        cv::imshow(demo_wnd, frame);
    }
	detector->clear();
    cv::destroyWindow(main_wnd);
    cv::destroyWindow(demo_wnd);

    return 0;
}
