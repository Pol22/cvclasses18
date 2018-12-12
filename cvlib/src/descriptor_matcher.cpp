/* Descriptor matcher algorithm implementation.
 * @file
 * @date 2018-11-25
 * @author Anonymous
 */

#include "cvlib.hpp"

namespace cvlib
{
void descriptor_matcher::knnMatchImpl(cv::InputArray queryDescriptors, std::vector<std::vector<cv::DMatch>>& matches, int k /*unhandled*/,
                                      cv::InputArrayOfArrays masks /*unhandled*/, bool compactResult /*unhandled*/)
{
    if (trainDescCollection.empty())
        return;

    if (queryDescriptors.getMat().cols != trainDescCollection[0].cols)
        return;

    int* q_desc = queryDescriptors.getMat().ptr<int>();
    int* t_desc = trainDescCollection[0].ptr<int>();

    const int desc_num = queryDescriptors.getMat().rows;
    const int train_desc_num = trainDescCollection[0].rows;

    matches.resize(desc_num);

    std::vector<cv::DMatch> ind(train_desc_num); // create DMatch vector

    static const int desc_length = queryDescriptors.getMat().cols;
    int shift = 0;
    int* cur_q_desc;
    for (int i = 0; i < desc_num; ++i)
    {
        // \todo implement Ratio of SSD check.
        // \todo matches[i].emplace_back(i, rnd.uniform(0, t_desc.rows), FLT_MAX);
        shift = i * desc_length;
        cur_q_desc = &q_desc[shift];

        matches[i] = ind;
        

    }
}

void descriptor_matcher::radiusMatchImpl(cv::InputArray queryDescriptors, std::vector<std::vector<cv::DMatch>>& matches, float /*maxDistance*/,
                                         cv::InputArrayOfArrays masks /*unhandled*/, bool compactResult /*unhandled*/)
{
    // \todo implement matching with "maxDistance"
    knnMatchImpl(queryDescriptors, matches, 1, masks, compactResult);
}
} // namespace cvlib
