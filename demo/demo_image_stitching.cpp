/* Demo application for Computer Vision Library.
 * @file
 * @date 2018-11-25
 * @author Anonymous
 */

#include <cvlib.hpp>
#include <opencv2/opencv.hpp>

#include "utils.hpp"

int demo_image_stitching(int argc, char* argv[])
{
    // cv::VideoCapture cap(0);
    cv::VideoCapture cap("TestVideo.mp4");
    if (!cap.isOpened())
        return -1;

    const auto main_wnd = "orig";
    const auto demo_wnd = "demo";

    cv::namedWindow(main_wnd);
    cv::namedWindow(demo_wnd);


    cvlib::Stitcher stitcher;

    cv::Mat frame, stitched_img;
    utils::fps_counter fps;
    int pressed_key = 0;
    bool pressed_flag = false;
    while (pressed_key != 27) // ESC
    {
        cap >> frame;
        cv::imshow(main_wnd, frame);

        pressed_key = cv::waitKey(1);
        if (pressed_key == ' ') // space
        {
            stitcher.setReference(frame);
            pressed_flag = true;
        }

        if (!pressed_flag)
        {
            continue;
        }

        stitched_img = stitcher.stitch(frame);

        utils::put_fps_text(stitched_img, fps);
        cv::imshow(demo_wnd, stitched_img);
    }

    cv::destroyWindow(main_wnd);
    cv::destroyWindow(demo_wnd);

    return 0;
}
