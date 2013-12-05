#ifndef __NEAREST_RESAMPLER_HPP__
#define __NEAREST_RESAMPLER_HPP__

#include "Resampler.hpp"

PRJ_BEGIN

class NearestResampler : public Resampler {
public:
  NearestResampler(SizeType nc=0)
    : Resampler(nc)
  {
  }

  virtual ~NearestResampler() {}

  virtual void resample(SizeType w, SizeType h);

};

PRJ_END

#endif // __NEAREST_RESAMPLER_HPP__
