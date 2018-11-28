/* Split and merge segmentation algorithm implementation.
 * @file
 * @date 2018-09-18
 * @author Anonymous
 */

#include "cvlib.hpp"

#include <iostream>

namespace cvlib
{
int compareGMM(const void* _gmm1, const void* _gmm2)
{
	GMM gmm1 = *(GMM*)_gmm1;
	GMM gmm2 = *(GMM*)_gmm2;

	if (gmm1.significants < gmm2.significants)
		return 1;
	else if (gmm1.significants == gmm2.significants)
		return 0;
	else
		return -1;
}

void motion_segmentation::reinit()
{
	delete[] modes;
	bg_model.release();
	modes_per_pixel.release();
}

void motion_segmentation::createModel(cv::Size size)
{
	bg_threshold = 0.9f; // Tbf threshold
	m_variance = 36.0f; // sigma for new mode

	modes = new GMM[size.height * size.width * max_modes]();
	modes_per_pixel = cv::Mat::zeros(size, CV_8UC1);
	bg_model = cv::Mat::zeros(size, CV_8UC3);
}

void motion_segmentation::setVarThreshold(int threshold)
{
	alpha = 1.0 / threshold;
}

int motion_segmentation::substractPixel(long posPixel, const cv::Scalar pixelRGB, uchar& numModes)
{
	long pos;
	bool fitsPDF = false;
	float sumWeights;
	// create first mode
	if (!numModes)
	{
		pos = posPixel + numModes;
		modes[pos].mean[0] = pixelRGB.val[0];
		modes[pos].mean[1] = pixelRGB.val[1];
		modes[pos].mean[2] = pixelRGB.val[2];
		modes[pos].variance = m_variance;
		modes[pos].weight = 1;
		++numModes;
		return FOREGROUND;
	}

	// check distance of pixel from each bg model
	sumWeights = 0.0f;
	for (int mode_i = 0; mode_i < numModes; mode_i++)
	{
		pos = posPixel + mode_i;
		float dR = modes[pos].mean[0] - pixelRGB.val[0];
		float dG = modes[pos].mean[1] - pixelRGB.val[1];
		float dB = modes[pos].mean[2] - pixelRGB.val[2];
		float dist = dR * dR + dG * dG + dB * dB;
		float var_m = modes[pos].variance;

		// if distance < 2.5 sigma
		if (std::sqrt(dist) < 2.5 * std::sqrt(var_m))
		{
			fitsPDF = true;
			// updating mode distribution
			modes[pos].weight = (1 - alpha) * modes[pos].weight + alpha;

			modes[pos].mean[0] = (1 - alpha) * modes[pos].mean[0] + alpha * pixelRGB.val[0];
			modes[pos].mean[1] = (1 - alpha) * modes[pos].mean[1] + alpha * pixelRGB.val[1];
			modes[pos].mean[2] = (1 - alpha) * modes[pos].mean[2] + alpha * pixelRGB.val[2];
			modes[pos].variance = (1 - alpha) * modes[pos].variance + alpha * dist;
		}
		else // distance >= 2.5 sigma
		{
			modes[pos].weight = (1 - alpha) * modes[pos].weight;
		}
		sumWeights += modes[pos].weight;
	}

	// not fit in PDF
	if (!fitsPDF)
	{
		// create new mode
		if (numModes < max_modes)
		{
			++numModes;
		}
		else // update significants
		{
			for (int mode_i = 0; mode_i < numModes; mode_i++)
			{
				pos = posPixel + mode_i;
				modes[pos].significants = modes[pos].weight / std::sqrt(modes[pos].variance);
			}
			// sort modes on significance order
			qsort(&modes[posPixel], numModes, sizeof(GMM), compareGMM);
		}

		pos = posPixel + numModes - 1;
		modes[pos].mean[0] = pixelRGB.val[0];
		modes[pos].mean[1] = pixelRGB.val[1];
		modes[pos].mean[2] = pixelRGB.val[2];
		modes[pos].variance = m_variance;
		modes[pos].weight = alpha;

		// sum for renormalization weights
		sumWeights = 0;
		for (int mode_i = 0; mode_i < numModes; mode_i++)
		{
			pos = posPixel + mode_i;
			sumWeights += modes[pos].weight;
		}
	}

	// renormalization weights
	for (int mode_i = 0; mode_i < numModes; mode_i++)
	{
		pos = posPixel + mode_i;
		modes[pos].weight /= sumWeights;
		modes[pos].significants = modes[pos].weight / std::sqrt(modes[pos].variance);
	}
	// sort modes on significance order
	qsort(&modes[posPixel], numModes, sizeof(GMM), compareGMM);

	// find argmin
	fitsPDF = false;
	float wtSum = 0;
	for (int mode_i = 0; mode_i < numModes; mode_i++)
	{
		pos = posPixel + mode_i;
		float dR = modes[pos].mean[0] - pixelRGB.val[0];
		float dG = modes[pos].mean[1] - pixelRGB.val[1];
		float dB = modes[pos].mean[2] - pixelRGB.val[2];
		float dist = dR * dR + dG * dG + dB * dB;
		float var_m = modes[pos].variance;

		wtSum += modes[pos].weight;

		if (std::sqrt(dist) < 2.5 * std::sqrt(var_m))
		{
			fitsPDF = true;
			return BACKGROUND;
		}
		if (wtSum > bg_threshold)
		{
			return FOREGROUND;
			//break;
		}
	}

	return BACKGROUND;
}

void motion_segmentation::apply(cv::InputArray input, cv::OutputArray fgmask, double)
{
	cv::Mat image = input.getMat();
	if (bg_model.empty())
		createModel(image.size());
	
	cv::Mat output = cv::Mat::zeros(image.size(), CV_8UC1);

	cv::Scalar s;
	long posPixel;
	uchar m;
	unsigned int height = image.size().height;
	unsigned int width = image.size().width;
	int res_pixel = 0;

	for (unsigned int j = 0; j < height; j++)
	{
		for (unsigned int i = 0; i < width; i++)
		{
			posPixel = (j * width + i) * max_modes; // ge

			s = image.at<cv::Vec3b>(j, i); // pixel value
			m = modes_per_pixel.at<uchar>(j, i); // number used modes
			res_pixel = substractPixel(posPixel, s, m);
			modes_per_pixel.at<uchar>(j, i) = m;
			output.at<uchar>(j, i) = res_pixel;
		}
	}

	// TODO
	// alpha update
	//number_of_frames++;
	//if(alpha > 1.0 / 500)
	//	alpha = 1.0 / number_of_frames;
    cv::Mat element = 
		getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(2, 2));
    cv::erode(output, output, element);
    cv::dilate(output, output, element);
	fgmask.assign(output);
}
} // namespace cvlib
