/* Demo application for Computer Vision Library.
 * @file
 * @date 2018-09-05
 * @author Anonymous
 */

#include <cvlib.hpp>
#include <opencv2/opencv.hpp>

#include "utils.hpp"

int demo_feature_descriptor(int argc, char* argv[])
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
	auto detector_Fast = cv::FastFeatureDetector::create();
    auto detector_a = cvlib::corner_detector_fast::create();
    auto detector_b = cv::KAZE::create();
    std::vector<cv::KeyPoint> corners;
    cv::Mat descriptors;

    utils::fps_counter fps;
    int pressed_key = 0;
    while (pressed_key != 27) // ESC
    {
        cap >> frame;
        cv::imshow(main_wnd, frame);
		cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
        detector_a->detect(frame_gray, corners);
        cv::drawKeypoints(frame, corners, frame, cv::Scalar(0, 0, 255));

        utils::put_fps_text(frame, fps);
		utils::put_number_of_keypoints(frame, corners.size());
        cv::imshow(demo_wnd, frame);

        pressed_key = cv::waitKey(30);
        if (pressed_key == ' ') // space
        {
            cv::FileStorage file("descriptor.json", cv::FileStorage::WRITE | cv::FileStorage::FORMAT_JSON);

            detector_a->compute(frame, corners, descriptors);
            file << detector_a->getDefaultName() << descriptors;

            detector_b->compute(frame, corners, descriptors);
            file << "detector_b" << descriptors;

            std::cout << "Dump descriptors complete! \n";
        }

        std::cout << "Feature points: " << corners.size() << "\r";
    }

    cv::destroyWindow(main_wnd);
    cv::destroyWindow(demo_wnd);

    return 0;
}
