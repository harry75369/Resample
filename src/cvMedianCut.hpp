#ifndef __CV_MEDIAN_CUT_HPP__
#define __CV_MEDIAN_CUT_HPP__

#include "MedianCut.hpp"

#include <opencv2/opencv.hpp>

PRJ_BEGIN

class cvMedianCut : public MedianCut<cv::Vec3b> {
protected:
  typedef cv::Vec3b T;
  using MedianCut<T>::Cluster;

public:
  cvMedianCut(SizeType nClusters)
    : MedianCut<T>(nClusters)
  {
  }
  virtual ~cvMedianCut() {}

protected:
  virtual void split(const Cluster & c, const std::vector<T> & data, Cluster & c1, Cluster & c2)
  {
    SizeType b_median = 0;
    SizeType g_median = 0;
    SizeType r_median = 0;
    for ( SizeType i=0; i < c.size(); i++ )
    {
      const T & d = data[i];
      b_median += d[0];
      g_median += d[1];
      r_median += d[2];
    }
    b_median /= c.size();
    g_median /= c.size();
    r_median /= c.size();
    for ( SizeType i=0; i < c.size(); i++ )
    {
      if ( data[i][0] > b_median )
      {
        c1.push_back(i);
      }
      else
      {
        c2.push_back(i);
      }
    }
  }

  virtual void post_process(const std::vector<T> & data)
  {
    for ( SizeType i=0; i < _clusters.size(); i++ )
    {
      SizeType b = 0;
      SizeType g = 0;
      SizeType r = 0;
      for ( SizeType j=0; j < _clusters[i].size(); j++ )
      {
        b += data[_clusters[i][j]][0];
        g += data[_clusters[i][j]][1];
        r += data[_clusters[i][j]][2];
      }
      b /= _clusters[i].size();
      g /= _clusters[i].size();
      r /= _clusters[i].size();
      for ( SizeType j=0; j < _clusters[i].size(); j++ )
      {
        _results.insert(std::pair<SizeType, T>(_clusters[i][j], T(b,g,r)));
      }
    }
  }
};

PRJ_END

#endif //__CV_MEDIAN_CUT_HPP__
