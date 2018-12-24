/* Demo application for Computer Vision Library.
 * @file
 * @date 2018-11-25
 * @author Anonymous
 */

#include <cvlib.hpp>
#include <opencv2/opencv.hpp>

#include "utils.hpp"

int demo_feature_matching(int argc, char* argv[])
{
    // cv::VideoCapture cap(0);
    cv::VideoCapture cap("TestVideo.mp4");
    if (!cap.isOpened())
        return -1;

    const auto main_wnd = "orig";
    const auto demo_wnd = "demo";

    cv::namedWindow(main_wnd);
    cv::namedWindow(demo_wnd);

    //auto detector = cv::AKAZE::create(); // \todo use your detector from cvlib
    auto detector = cvlib::corner_detector_fast::create();
    auto matcher = cvlib::descriptor_matcher(1.2f); //\todo add trackbar to demo_wnd to tune ratio value

    /// \brief helper struct for tidy code
    struct img_features
    {
        cv::Mat img;
        std::vector<cv::KeyPoint> corners;
        cv::Mat descriptors;
    };

    img_features ref;
    img_features test;
    std::vector<std::vector<cv::DMatch>> pairs;

    cv::Mat main_frame;
    cv::Mat demo_frame;
    cv::Mat gray;
    utils::fps_counter fps;
    int pressed_key = 0;
    while (pressed_key != 27) // ESC
    {
        cap >> test.img;
        cv::cvtColor(test.img, gray, cv::COLOR_BGR2GRAY);
        detector->detect(gray, test.corners);
        cv::drawKeypoints(test.img, test.corners, main_frame);
        cv::imshow(main_wnd, main_frame);

        pressed_key = cv::waitKey(30);
        if (pressed_key == ' ') // space
        {
            ref.img = gray.clone();
            detector->detectAndCompute(ref.img, cv::Mat(), ref.corners, ref.descriptors);
        }

        if (ref.corners.empty())
        {
            continue;
        }

        detector->compute(test.img, test.corners, test.descriptors);
        //\todo add trackbar to demo_wnd to tune threshold value
        matcher.radiusMatch(test.descriptors, ref.descriptors, pairs, 100.0f);
        cv::drawMatches(test.img, test.corners, ref.img, ref.corners, pairs, demo_frame);

        utils::put_fps_text(demo_frame, fps);
        cv::imshow(demo_wnd, demo_frame);
    }

    cv::destroyWindow(main_wnd);
    cv::destroyWindow(demo_wnd);

    return 0;
}
