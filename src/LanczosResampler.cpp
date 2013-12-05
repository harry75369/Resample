#include "LanczosResampler.hpp"

USE_PRJ_NAMESPACE;

void LanczosResampler::resample(SizeType w, SizeType h)
{
  ASSERT(_input.data);
  cv::resize(_input, _output, cv::Size(w, h), 0, 0, cv::INTER_LANCZOS4);
  reduce_color();
}
