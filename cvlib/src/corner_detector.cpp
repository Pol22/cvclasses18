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

int corner_detector_fast::getShift(const int& index) const
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
        defaut: return 0;
    }
}

char corner_detector_fast::checkDarkerOrBrighter(const uchar* pixel, const uchar* neighbour) const
{
    if (*neighbour <= *pixel - threshold)
        return -1;
    else if (*pixel + threshold <= *neighbour)
        return 1;
    else
        return 0;
}

bool corner_detector_fast::highSpeedTest(const uchar* pixel) const
{
	char check1 = checkDarkerOrBrighter(pixel, &pixel[getShift(0)]);
	char check2 = checkDarkerOrBrighter(pixel, &pixel[getShift(8)]);
    if (check1 == 0 && check2 == 0)
        return false;

	char check3 = checkDarkerOrBrighter(pixel, &pixel[getShift(4)]);
	char check4 = checkDarkerOrBrighter(pixel, &pixel[getShift(12)]);
	char result_check = check1 + check2 + check3 + check4;
	if (result_check == 3 || result_check == -3)
		return true;

	return false;
}

void corner_detector_fast::detect(cv::InputArray input, CV_OUT std::vector<cv::KeyPoint>& keypoints, cv::InputArray /*mask = cv::noArray()*/)
{
    keypoints.clear();
    cv::Mat image = input.getMat();
	if (width == 0)
	{
		end_j = image.size().height - 2;
		width = image.size().width;
		end_i = width - 2;
		int_circle_pixels[0] = 0;
	}

    uchar* img  = image.data;

    int shift = 0;
    for(int j = 3; j < end_j; j += 1)
    {
        for(int i = 3; i < end_i; i += 1)
        {
            shift = j * width + i;
			if (highSpeedTest(&img[shift]))
			{
				for (int circle_i = 1; circle_i < number_of_circle_pixels + 1; circle_i++)
				{
					circle_pixels[circle_i] = checkDarkerOrBrighter(&img[shift], &img[shift + getShift(circle_i - 1)]);
					int_circle_pixels[circle_i] = int_circle_pixels[circle_i - 1] + circle_pixels[circle_i];
				}
				for (int circle_i = number_of_circle_pixels + 1; circle_i < number_of_circle_pixels + number_non_similar_pixels; circle_i++)
				{
					int_circle_pixels[circle_i] = int_circle_pixels[circle_i - 1] + circle_pixels[circle_i % number_of_circle_pixels];
				}
				for (int circle_i = 1; circle_i < number_of_circle_pixels; circle_i++)
				{
					char diff = int_circle_pixels[circle_i - 1 + number_non_similar_pixels] - int_circle_pixels[circle_i - 1];
					if (diff == number_non_similar_pixels || diff == -number_non_similar_pixels)
					{
						if (!keypoints.empty())
						{
							cv::Point2f last_keypoint = keypoints.back().pt;
							// easy non max suppresion
							if (abs(last_keypoint.x - float(i)) + abs(last_keypoint.y - float(j)) < 5.0f)
								break;
						}
						keypoints.emplace_back(float(i), float(j), 1.0f);
						break;
					}
				}
			}
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
