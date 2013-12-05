#ifndef __BILINEAR_RESAMPLER_HPP__
#define __BILINEAR_RESAMPLER_HPP__

#include "Resampler.hpp"

PRJ_BEGIN

class BilinearResampler : public Resampler {
public:
  BilinearResampler(SizeType nc=0)
    : Resampler(nc)
  {
  }

  virtual ~BilinearResampler() {}

  virtual void resample(SizeType w, SizeType h);

};

PRJ_END

#endif // __BILINEAR_RESAMPLER_HPP__
