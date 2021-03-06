/* Computer Vision Functions.
 * @file
 * @date 2018-09-05
 * @author Anonymous
 */

#ifndef __CVLIB_HPP__
#define __CVLIB_HPP__

#include <opencv2/opencv.hpp>

namespace cvlib
{
/// \brief Split and merge algorithm for image segmentation
/// \param image, in - input image
/// \param stddev, in - threshold to treat regions as homogeneous
/// \return segmented image
cv::Mat split_and_merge(const cv::Mat& image, double stddev);
cv::Mat only_split(const cv::Mat& image, double stddev);
    /// \brief Segment texuture on passed image according to sample in ROI
/// \param image, in - input image
/// \param roi, in - region with sample texture on passed image
/// \param eps, in - threshold parameter for texture's descriptor distance
/// \return binary mask with selected texture
cv::Mat select_texture(const cv::Mat& image, const cv::Rect& roi, double eps, void* data);


struct GMM
{
	float weight = 0.0;
	float mean[3] = { 0.0, 0.0, 0.0 }; // mean RGB
	float variance = 0.0;
	float significants = 0.0; // weight / variance = which Gaussians shoud be part of bg
};

/// \brief Motion Segmentation algorithm
class motion_segmentation : public cv::BackgroundSubtractor
{
public:
    /// \brief ctor
	motion_segmentation() {}
	~motion_segmentation()
	{ 
		if (modes != nullptr)
			delete[] modes;
	}

	void reinit();

	void setVarThreshold(int threshold);

    /// \see cv::BackgroundSubtractor::apply
    void apply(cv::InputArray image, cv::OutputArray fgmask, double learningRate = -1) override;

    /// \see cv::BackgroundSubtractor::BackgroundSubtractor
    void getBackgroundImage(cv::OutputArray backgroundImage) const override
    {
        //backgroundImage.assign(bg_model_);
    }

private:
	void createModel(cv::Size size);
	int substractPixel(long posPixel, const cv::Scalar pixelRGB, uchar& numModes);

	float bg_threshold;
	float m_variance;

	GMM* modes; // gaussian mixture model
	cv::Mat modes_per_pixel;
	cv::Mat bg_model;
	float alpha = 1.0 / 50;
	unsigned number_of_frames = 2;
	const int max_modes = 3;
    
	static const int BACKGROUND = 0;
	static const int FOREGROUND = 255;
};

/// \brief FAST corner detection algorithm
class corner_detector_fast : public cv::Feature2D
{
public:
	corner_detector_fast();
    /// \brief Fabrique method for creating FAST detector
    static cv::Ptr<corner_detector_fast> create();

    /// \see Feature2d::detect
    virtual void detect(cv::InputArray image, CV_OUT std::vector<cv::KeyPoint>& keypoints, cv::InputArray mask = cv::noArray()) override;

    /// \see Feature2d::compute
    virtual void compute(cv::InputArray image, std::vector<cv::KeyPoint>& keypoints, cv::OutputArray descriptors) override;

    /// \see Feature2d::detectAndCompute
    virtual void detectAndCompute(cv::InputArray image, cv::InputArray mask, std::vector<cv::KeyPoint>& keypoints, cv::OutputArray descriptors,
                                  bool useProvidedKeypoints = false) override;

    /// \see Feature2d::getDefaultName
    virtual cv::String getDefaultName() const override
    {
        return "FAST_Binary";
    }
private:
  int getShift(const int& index) const;
  char checkDarkerOrBrighter(const uchar* pixel, const uchar* neighbour) const;
  bool highSpeedTest(const uchar* pixel) const;

	bool pointOnImage(const cv::Mat& image, const cv::Point2f& point);
	int twoPointsTest(const cv::Mat& image, const cv::Point2f& point1, const cv::Point2f& point2, const int& num);
	void binaryTest(const cv::Mat& image, const cv::Point2f& keypoint, int* descriptor);
	// detector
	static const int number_of_circle_pixels = 16;
	static const int number_non_similar_pixels = 12;
	static const uchar threshold = 30;
	static const int roi_mask_size = 7;
	char circle_pixels[number_of_circle_pixels + 1];
	char int_circle_pixels[number_of_circle_pixels + number_non_similar_pixels + 1];
	int width = 0;
	int end_j;
	int end_i;
	// extractor
	static const int S = 15;
	static const int desc_length = 8; // 32 * 8 = 256
	static const int all_length = desc_length * 32; // 256
	cv::Point2f test_points1[all_length];
	cv::Point2f test_points2[all_length];
	cv::RNG rng;
};

/// \brief Descriptor matched based on ratio of SSD
class descriptor_matcher : public cv::DescriptorMatcher
{
    public:
    /// \brief ctor
    descriptor_matcher(float ratio = 1.5) : ratio_(ratio) {}

    /// \brief setup ratio threshold for SSD filtering
    void set_ratio(float r)
    {
        ratio_ = r;
    }

    protected:
    /// \see cv::DescriptorMatcher::knnMatchImpl
    void knnMatchImpl(cv::InputArray queryDescriptors, std::vector<std::vector<cv::DMatch>>& matches, int k,
                              cv::InputArrayOfArrays masks = cv::noArray(), bool compactResult = false) override;

    /// \see cv::DescriptorMatcher::radiusMatchImpl
    void radiusMatchImpl(cv::InputArray queryDescriptors, std::vector<std::vector<cv::DMatch>>& matches, float maxDistance,
                                 cv::InputArrayOfArrays masks = cv::noArray(), bool compactResult = false) override;

    /// \see cv::DescriptorMatcher::isMaskSupported
    bool isMaskSupported() const override
    {
        return false;
    }

    /// \see cv::DescriptorMatcher::isMaskSupported
    cv::Ptr<cv::DescriptorMatcher> clone(bool emptyTrainData = false) const override
    {
        cv::Ptr<cv::DescriptorMatcher> copy = new descriptor_matcher(*this);
        if (emptyTrainData)
        {
            copy->clear();
        }
        return copy;
    }

private:
    int hamming_distance(int* x1, int* x2);

    float ratio_;
    int desc_length = 0;
};

/// \brief Stitcher for merging images into big one
class Stitcher
{
public:
    Stitcher() = default;
    ~Stitcher() { detector.release(); }
    void setReference(cv::Mat& img);
    cv::Mat stitch(cv::Mat& img);

private:
    cv::Ptr<cvlib::corner_detector_fast> detector = cvlib::corner_detector_fast::create();
    cvlib::descriptor_matcher matcher;
    cv::Mat ref_img;
    std::vector<cv::KeyPoint> ref_keypoints;
    cv::Mat ref_descriptors;
    static constexpr float max_distance = 100.0f;
};
} // namespace cvlib

#endif // __CVLIB_HPP__
