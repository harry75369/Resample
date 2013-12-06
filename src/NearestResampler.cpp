#include "NearestResampler.hpp"

USE_PRJ_NAMESPACE;

void NearestResampler::resample(SizeType w, SizeType h)
{
  ASSERT(_input.data);
  cv::resize(_input, _output, cv::Size(w, h), 0, 0, cv::INTER_NEAREST);
  reduce_color(_nColors, _output);
}
