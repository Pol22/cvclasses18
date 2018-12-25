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
    int ratio = 12;
    int radius_threshold = 100;
    cv::createTrackbar("ratio / 10", demo_wnd, &ratio, 100);
    cv::createTrackbar("radius threshold", demo_wnd, &radius_threshold, 256);


    auto detector = cvlib::corner_detector_fast::create();
    auto matcher = cvlib::descriptor_matcher(float(ratio) / 10);

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

        pressed_key = cv::waitKey(1);
        if (pressed_key == ' ') // space
        {
            ref.img = gray.clone();
            detector->detectAndCompute(ref.img, cv::Mat(), ref.corners, ref.descriptors);
            matcher.set_ratio(float(ratio) / 10);
        }

        if (ref.corners.empty())
        {
            continue;
        }

        detector->compute(test.img, test.corners, test.descriptors);
        matcher.radiusMatch(test.descriptors, ref.descriptors, pairs,
            static_cast<float>(radius_threshold));
        cv::drawMatches(gray, test.corners, ref.img, ref.corners, pairs, demo_frame);

        utils::put_fps_text(demo_frame, fps);
        cv::imshow(demo_wnd, demo_frame);
    }

    cv::destroyWindow(main_wnd);
    cv::destroyWindow(demo_wnd);

    return 0;
}
