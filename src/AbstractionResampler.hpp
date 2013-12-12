/**
 * This class implements the paper
 *
 * "Pixelated Image Abstration"
 *
 * by Gerstner, NPAR 2012.
 */
#ifndef __ABSTRACTION_RESAMPLER_HPP__
#define __ABSTRACTION_RESAMPLER_HPP__

#include "Resampler.hpp"

#include <vector>
#include <map>
#include <set>

PRJ_BEGIN

class AbstractionResampler : public Resampler {
protected:
  struct SuperPixel {
    cv::Vec2f position; // normalized
    cv::Vec3f color;    // normalized
    const SizeType id;
    SuperPixel(SizeType id)
      : id(id)
    {}
  };

public:
  AbstractionResampler(SizeType nc=0)
    : Resampler(nc)
  {
  }

  virtual ~AbstractionResampler() {}

  virtual void resample(SizeType w, SizeType h);

protected:
  void initialize(SizeType w, SizeType h);
  bool is_done();
  void iterate();
  void finalize();

public:
  void visualizeSuperpixel(cv::Mat & output);

public:
  static inline void bgr2lab(cv::Mat & in, cv::Mat & out)
  {
    // bgr2lab conversion includes two steps:
    // 1. convert data type from CV_8UC3 to CV_32FC3 with scaling
    // 2. convert color space from BGR to Lab
    in.convertTo(out, CV_32FC3, 1./255);
    cv::cvtColor(out, out, CV_BGR2Lab);
  }
  static inline void lab2bgr(cv::Mat & in, cv::Mat & out)
  {
    // lab2bgr conversion includes two steps:
    // 1. convert color space from Lab to BGR
    // 2. convert data type from CV_32FC3 to CV_8UC3 with scaling
    cv::cvtColor(in, out, CV_Lab2BGR);
    out.convertTo(out, CV_8UC3, 255.0);
  }
  static inline cv::Vec3b lab2bgr(cv::Vec3f lab)
  {
    cv::Mat holder(cv::Size(1,1),CV_32FC3);
    holder.at<cv::Vec3f>(0,0) = lab;
    cv::Mat holder2;
    lab2bgr(holder, holder2);
    return holder2.at<cv::Vec3b>(0,0);
  }
  static inline cv::Vec3f bgr2lab(cv::Vec3b bgr)
  {
    cv::Mat holder(cv::Size(1,1),CV_8UC3);
    holder.at<cv::Vec3b>(0,0) = bgr;
    cv::Mat holder2;
    bgr2lab(holder, holder2);
    return holder2.at<cv::Vec3f>(0,0);
  }

protected:
  SizeType _input_width;
  SizeType _input_height;
  SizeType _output_width;
  SizeType _output_height;
  cv::Mat _input_lab;
  cv::Mat _output_lab;
  SizeType _iteration;
  Real _range;
  std::vector<cv::Vec3f> _palette;
  std::vector<SuperPixel> _superpixels;
  std::vector<std::vector<SizeType> > _pixel_map;

};

PRJ_END

#endif //__ABSTRACTION_RESAMPLER_HPP__
