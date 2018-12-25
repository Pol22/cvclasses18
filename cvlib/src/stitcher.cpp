/* Descriptor matcher algorithm implementation.
 * @file
 * @date 2018-12-25
 * @author Pavel
 */

#include "cvlib.hpp"

namespace cvlib
{
void Stitcher::setReference(cv::Mat& img)
{
    img.copyTo(ref_img);
    cv::Mat gray;
    cv::cvtColor(ref_img, gray, cv::COLOR_BGR2GRAY);
    detector->detectAndCompute(gray, cv::noArray(), ref_keypoints, ref_descriptors);
}

cv::Mat Stitcher::stitch(cv::Mat& img)
{
    cv::Mat gray, descriptors;
    std::vector<cv::KeyPoint> keypoints;
    std::vector<std::vector<cv::DMatch>> matches;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    detector->detectAndCompute(gray, cv::noArray(), keypoints, descriptors);
    matcher.radiusMatch(descriptors, ref_descriptors, matches, max_distance);

    if (matches.size() <= 3)
    {
        return cv::Mat();
    }
    else
    {
        std::vector<cv::Point2f> obj;
        std::vector<cv::Point2f> scene;
        for (const auto& match: matches)
        {
            obj.push_back(ref_keypoints[match[0].queryIdx].pt);
            scene.push_back(keypoints[match[0].trainIdx].pt);
        }
        // Homography matrix
        cv::Mat H = cv::findHomography(obj, scene, cv::RANSAC);

        if (!H.empty())
        {
            // warpPerspective
        }
    }

}

} // namespace cvlib