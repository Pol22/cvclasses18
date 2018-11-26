/* FAST corner detector algorithm implementation.
 * @file
 * @date 2018-10-16
 * @author Anonymous
 */

#include "cvlib.hpp"

#include <ctime>

namespace cvlib
{
// static
cv::Ptr<corner_detector_fast> corner_detector_fast::create()
{
    return cv::makePtr<corner_detector_fast>();
}

int corner_detector_fast::getShift(const int& index, const int& width)
{
    switch (index)
    {
        case 0: return -3 * width;
        case 1: return -3 * width + 1;
        case 2: return -2 * width + 2;
        case 3: return -width + 3;
        case 4: return 3;
        case 5: return width + 3;
        case 6: return 2 * width + 2;
        case 7: return 3 * width + 1;
        case 8: return 3 * width;
        case 9: return 3 * width - 1;
        case 10: return 2 * width - 2;
        case 11: return width - 3;
        case 12: return -3;
        case 13: return -width - 3;
        case 14: return -2 * width - 2;
        case 15: return -3 * width - 1;
        defaut: return -1;
    }
}

char corner_detector_fast::checkDarkerOrBrighter(const uchar* pixel, const uchar* neighbour)
{
    if (*neighbour <= *pixel - threshold)
        return -1;
    else if (*pixel + threshold <= *neighbour)
        return 1;
    else
        return 0;
}

bool corner_detector_fast::highSpeedTest(const uchar* pixel, const int& width)
{
    int shift_top = getShift(0);
    int shift_bottom = getShift(8);
    char check1 = checkDarkerOrBrighter(pixel, pixel[shift_top]) + checkDarkerOrBrighter(pixel, pixel[shift_bottom]);
    if (check1 == 1 || check1 == -1)
        return false;
    int shift_right = getShift(4);
    int shift_left = getShift(12);
    // check second
    if(checkDarkerOrBrighter(pixel, pixel[shift_right]) && )
}

void corner_detector_fast::detect(cv::InputArray input, CV_OUT std::vector<cv::KeyPoint>& keypoints, cv::InputArray /*mask = cv::noArray()*/)
{
    keypoints.clear();
    cv::Mat image = input.getMat();
    int height = image.size().height;
    int width = image.size().width;

    uchar* img  = image.data;

    long shift = 0;
    for(long j = 3; j < height - 3; j++)
    {
        for(long i = 3; i < width - 3; i++)
        {
            shift = j * width + i;
            img[shift]
        }
    }
}

void corner_detector_fast::compute(cv::InputArray, std::vector<cv::KeyPoint>& keypoints, cv::OutputArray descriptors)
{
    std::srand(unsigned(std::time(0))); // \todo remove me
    // \todo implement any binary descriptor
    const int desc_length = 2;
    descriptors.create(static_cast<int>(keypoints.size()), desc_length, CV_32S);
    auto desc_mat = descriptors.getMat();
    desc_mat.setTo(0);

    int* ptr = reinterpret_cast<int*>(desc_mat.ptr());
    for (const auto& pt : keypoints)
    {
        for (int i = 0; i < desc_length; ++i)
        {
            *ptr = std::rand();
            ++ptr;
        }
    }
}

void corner_detector_fast::detectAndCompute(cv::InputArray, cv::InputArray, std::vector<cv::KeyPoint>&, cv::OutputArray descriptors, bool /*= false*/)
{
    // \todo implement me
}
} // namespace cvlib
