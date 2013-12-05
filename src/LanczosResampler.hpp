#ifndef __LANCZOS_RESAMPLER_HPP__
#define __LANCZOS_RESAMPLER_HPP__

#include "Resampler.hpp"

PRJ_BEGIN

class LanczosResampler : public Resampler {
public:
  LanczosResampler(SizeType nc=0)
    : Resampler(nc)
  {
  }

  virtual ~LanczosResampler() {}

  virtual void resample(SizeType w, SizeType h);

};

PRJ_END

#endif // __LANCZOS_RESAMPLER_HPP__
