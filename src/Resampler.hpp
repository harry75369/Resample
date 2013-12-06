#ifndef __RESAMPLER_HPP__
#define __RESAMPLER_HPP__

#include "Config.hpp"
#include "cvMedianCut.hpp"

#include <string>
#include <opencv2/opencv.hpp>

PRJ_BEGIN

class Resampler {
public:
  Resampler(SizeType nc)
    : _nColors(nc)
  {
  }

  virtual ~Resampler() {}

  virtual bool open(const std::string & filename)
  {
    _input = cv::imread(filename, CV_LOAD_IMAGE_COLOR);

    if ( _input.data ) {
      _output = _input.clone();
      return true;
    }
    return false;
  }

  virtual void load(const cv::Mat & mat)
  {
    _input = mat.clone();
    if ( _input.channels() != 3 )
    {
      cv::cvtColor(_input, _input, CV_GRAY2BGR);
    }
    _output = _input.clone();
  }

  virtual void resample(SizeType w, SizeType h) = 0;

  virtual bool save(const std::string & filename)
  {
    ASSERT(_output.data);
    return cv::imwrite(filename, _output);
  }

  const cv::Mat & getInput() const
  {
    return _input;
  }

  const cv::Mat & getOutput() const
  {
    return _output;
  }

protected:
  static void reduce_color(SizeType nColors, cv::Mat & image)
  {
#if 0
    // naive color quantization
    if ( nColors )
    {
      ASSERT(image.channels()==3);
      typedef unsigned char unchar;
      unchar d = 256 / nColors * 4;
      for ( int i=0; i < image.cols; i++ )
        for ( int j=0; j < image.rows; j++ )
        {
          unchar b = image.at<cv::Vec3b>(j, i)[0];
          unchar g = image.at<cv::Vec3b>(j, i)[1];
          unchar r = image.at<cv::Vec3b>(j, i)[2];
          b = b / d * d;
          g = g / d * d;
          r = r / d * d;
          image.at<cv::Vec3b>(j, i) = cv::Vec3b(b, g, r);
        }
    }
#endif

#if 1
    // color quantization using median cut
    if ( nColors )
    {
      cvMedianCut cut(nColors);
      std::vector<cv::Vec3b> data;
      for ( int i=0; i < image.cols; i++ )
        for ( int j=0; j < image.rows; j++ )
        {
          data.push_back(image.at<cv::Vec3b>(j, i));
        }
      cut.process(data);
      for ( int i=0; i < image.cols; i++ )
        for ( int j=0; j < image.rows; j++ )
        {
          auto res = cut.getResult(i*image.rows+j);
          INFO("(%u, %u) = (%u, %u, %u)", i, j, res[0], res[1], res[2]);
          image.at<cv::Vec3b>(j, i) = res;
        }
    }
#endif
  }

protected:
  cv::Mat _input;
  cv::Mat _output;
  SizeType _nColors; ///< number of colors after resampling

};

PRJ_END

#endif //__RESAMPLER_HPP__
