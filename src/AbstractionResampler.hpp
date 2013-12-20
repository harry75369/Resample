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

PRJ_BEGIN

class AbstractionResampler : public Resampler {
protected:
  struct SuperPixel {
    cv::Vec2f position; // normalized
    cv::Vec3f color;
    const SizeType id;
    SizeType  assoc;
    SuperPixel(SizeType id)
      : id(id), assoc(0)
    {}
  };

public:
  AbstractionResampler(SizeType nc)
    : Resampler(nc)
  {
  }

  virtual ~AbstractionResampler() {}

  virtual void resample(SizeType w, SizeType h);

protected:
  void initialize(const SizeType w, const SizeType h);
  bool is_done();
  void iterate();
  void finalize();

public:
  void visualizeSuperpixel(cv::Mat & output);

protected:
  void remap_pixels();
  void update_superpixels();
  void associate_superpixels();
  Real refine_palette();
  void expand_palette();
  void split_color(SizeType index);
  void condense_palette();
  Real slic_distance(SizeType i, SizeType j, const cv::Vec2f & pos, const cv::Vec3f & spcolor) const;
  std::pair<cv::Vec3f, Real> get_max_eigen(SizeType pidx);
  std::vector<cv::Vec3f> get_averaged_palette();

public:
  static inline void bgr2lab(const cv::Mat & in, cv::Mat & out)
  {
    // bgr2lab conversion includes two steps:
    // 1. convert data type from CV_8UC3 to CV_32FC3 with scaling
    // 2. convert color space from BGR to Lab
    in.convertTo(out, CV_32FC3, 1./255);
    cv::cvtColor(out, out, CV_BGR2Lab);
  }
  static inline void lab2bgr(const cv::Mat & in, cv::Mat & out)
  {
    // lab2bgr conversion includes two steps:
    // 1. convert color space from Lab to BGR
    // 2. convert data type from CV_32FC3 to CV_8UC3 with scaling
    cv::cvtColor(in, out, CV_Lab2BGR);
    out.convertTo(out, CV_8UC3, 255.0);
  }
  static inline cv::Vec3b lab2bgr(const cv::Vec3f & lab)
  {
    cv::Mat holder(cv::Size(1,1),CV_32FC3);
    holder.at<cv::Vec3f>(0,0) = lab;
    cv::Mat holder2;
    lab2bgr(holder, holder2);
    return holder2.at<cv::Vec3b>(0,0);
  }
  static inline cv::Vec3f bgr2lab(const cv::Vec3b & bgr)
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
  Real _input_area;
  Real _output_area;
  cv::Mat _input_lab;
  cv::Mat _output_lab;
  bool _converged;
  bool _palette_maxed;
  SizeType _iteration;
  Real _range;
  std::vector<SuperPixel> _superpixels;
  std::vector<std::vector<SizeType> > _pixel_map;
  std::vector<cv::Vec3f> _palette;
  Real _prob_o;
  std::vector<Real> _prob_c;
  std::vector<std::vector<Real> > _prob_co;
  std::vector<std::pair<SizeType, SizeType> > _sub_superpixel_pairs;
  Real _temperature;

};

PRJ_END

#endif //__ABSTRACTION_RESAMPLER_HPP__
