#ifndef __RESAMPLER_HPP__
#define __RESAMPLER_HPP__

#include "Config.hpp"

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
  void reduce_color()
  {
#if 0
    if ( _nColors )
    {
      ASSERT(_output.channels()==3);
      typedef unsigned char unchar;
      unchar d = 256 / _nColors * 4;
      for ( int i=0; i < _output.cols; i++ )
        for ( int j=0; j < _output.rows; j++ )
        {
          unchar b = _output.at<cv::Vec3b>(j, i)[0];
          unchar g = _output.at<cv::Vec3b>(j, i)[1];
          unchar r = _output.at<cv::Vec3b>(j, i)[2];
          b = b / d * d;
          g = g / d * d;
          r = r / d * d;
          _output.at<cv::Vec3b>(j, i) = cv::Vec3b(b, g, r);
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
