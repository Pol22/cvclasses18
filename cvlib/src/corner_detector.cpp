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

corner_detector_fast::corner_detector_fast()
{
		double sigma = 1.0 / 25.0 * S * S;
		for (int i = 0; i < all_length; i++)
		{
				test_points1[i] = cv::Point2f(rng.gaussian(sigma), rng.gaussian(sigma));
				test_points2[i] = cv::Point2f(rng.gaussian(sigma), rng.gaussian(sigma));
		}
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
    cv::Mat feature_mask = cv::Mat::zeros(image.size(), CV_8U);

		if (width == 0)
		{
				end_j = image.size().height - 3;
				width = image.size().width;
				end_i = width - 3;
				int_circle_pixels[0] = 0;
		}

    uchar* img  = image.data;
		uchar* feature_mask_ptr = feature_mask.data;
		cv::Rect roi_mask_rect;
		cv::Point roi_mask_shift = cv::Point(roi_mask_size / 2, roi_mask_size / 2);
		cv::Mat roi_mask;

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

						for (int circle_i = number_of_circle_pixels + 1; circle_i < number_of_circle_pixels + number_non_similar_pixels + 1; circle_i++)
						{
								int_circle_pixels[circle_i] = int_circle_pixels[circle_i - 1] + circle_pixels[circle_i % number_of_circle_pixels];
						}

						for (int circle_i = 1; circle_i < number_of_circle_pixels + 1; circle_i++)
						{
								char diff = int_circle_pixels[circle_i - 1 + number_non_similar_pixels] - int_circle_pixels[circle_i - 1];
								if (diff == number_non_similar_pixels || diff == -number_non_similar_pixels)
								{
										if(feature_mask_ptr[shift] == 0)
										{
												keypoints.emplace_back(cv::KeyPoint(i, j, 1));
												roi_mask_rect = cv::Rect(i, j, roi_mask_size, roi_mask_size);
												roi_mask_rect -= roi_mask_shift;
												roi_mask = feature_mask(roi_mask_rect);
												roi_mask = 1;
												i += roi_mask_size / 2;
										}
										break;
								}
						}
				}
					}
    }
}

bool corner_detector_fast::pointOnImage(const cv::Mat& image, const cv::Point2f& point)
{
		if (point.x > 0.0 && point.x < image.rows && point.y > 0.0 && point.y < image.cols)
				return true;
		return false;
}

int corner_detector_fast::twoPointsTest(const cv::Mat& image, const cv::Point2f& point1, const cv::Point2f& point2, const int& num)
{
		if (pointOnImage(image, point1) && pointOnImage(image, point2) && image.at<uchar>(point1) < image.at<uchar>(point2))
		{
				return 1 << num;
		}
		return 0;
}

void corner_detector_fast::binaryTest(const cv::Mat& image, const cv::Point2f& keypoint, int* descriptor)
{
		for (int i = 0; i < all_length; i++)
		{
				descriptor[i / 32] += twoPointsTest(image, keypoint + test_points1[i], keypoint + test_points2[i], i % 32);
		}
}

/*
 * @brief BRIEF Features
 */
void corner_detector_fast::compute(cv::InputArray input, std::vector<cv::KeyPoint>& keypoints, cv::OutputArray descriptors)
{
		cv::Mat image;
		cv::GaussianBlur(input.getMat(), image, cv::Size(9, 9), 2.0);

		descriptors.create(static_cast<int>(keypoints.size()), desc_length, CV_32S);
		cv::Mat desc_mat = descriptors.getMat();

		int* desc_ptr = desc_mat.ptr<int>();

		const int keypoints_num = keypoints.size();
		int shift = 0;
		for (int i = 0; i < keypoints_num; i++)
		{
				shift = i * desc_length;
				binaryTest(image, keypoints[i].pt, &desc_ptr[shift]);
		}
}

void corner_detector_fast::detectAndCompute(cv::InputArray input, cv::InputArray mask, std::vector<cv::KeyPoint>& keypoints, cv::OutputArray descriptors, bool /*= false*/)
{
		detect(input, keypoints, mask);
		compute(input, keypoints, descriptors);
}
} // namespace cvlib
