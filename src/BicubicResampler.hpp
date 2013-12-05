#ifndef __BICUBIC_RESAMPLER_HPP__
#define __BICUBIC_RESAMPLER_HPP__

#include "Resampler.hpp"

PRJ_BEGIN

class BicubicResampler : public Resampler {
public:
  BicubicResampler(SizeType nc=0)
    : Resampler(nc)
  {
  }

  virtual ~BicubicResampler() {}

  virtual void resample(SizeType w, SizeType h);

};

PRJ_END

#endif //__BICUBIC_RESAMPLER_HPP__
