/* Descriptor matcher algorithm implementation.
 * @file
 * @date 2018-11-25
 * @author Anonymous
 */

#include "cvlib.hpp"
#include <cstdlib>

namespace cvlib
{

int descriptor_matcher::hamming_distance(int* x1, int* x2)
{
    int result = 0;
    int val = 0;


    for (int i = 0; i < desc_length; i++)
    {
        val = *x1 ^ *x2;
        while (val != 0)
        {
            result++;
            val &= val - 1;
        }
        x1++;
        x2++;
    }
    return result;
}

void descriptor_matcher::knnMatchImpl(cv::InputArray queryDescriptors, std::vector<std::vector<cv::DMatch>>& matches, int k,
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

    std::vector<cv::DMatch> temp(train_desc_num); // create DMatch vector

    desc_length = queryDescriptors.getMat().cols;
    int shift_q = 0;
    int shift_t = 0;
    int* cur_q_desc;
    int* cur_t_desc;
    int distance = 0;
    for (int i = 0; i < desc_num; ++i)
    {
        shift_q = i * desc_length;
        cur_q_desc = &q_desc[shift_q];

        for (int j = 0; j < train_desc_num; ++j)
        {
            shift_t = j * desc_length;
            cur_t_desc = &t_desc[shift_t];
            distance = hamming_distance(cur_q_desc, cur_t_desc);
            temp[j].distance = distance;
            temp[j].queryIdx = i;
            temp[j].trainIdx = j;
        }
        std::sort(temp.begin(), temp.end());

        // Ratio of SSD check ???
        if (temp[0].distance / temp[1].distance > 0) //ratio_)
        {
            if (temp.size() > k)
              temp.erase(temp.begin() + k, temp.end());
            matches.push_back(std::move(temp));
            temp.clear();
            temp.resize(train_desc_num);
        }
    }
}

void descriptor_matcher::radiusMatchImpl(cv::InputArray queryDescriptors, std::vector<std::vector<cv::DMatch>>& matches, float maxDistance,
                                         cv::InputArrayOfArrays masks /*unhandled*/, bool compactResult /*unhandled*/)
{
    std::vector<std::vector<cv::DMatch>> temp_matches;
    knnMatchImpl(queryDescriptors, temp_matches, 1, masks, compactResult);

    for (auto& match : temp_matches)
    {
        if (match[0].distance < maxDistance)
        {
            matches.push_back(std::move(match));
        }
    }

    temp_matches.clear();
}
} // namespace cvlib
