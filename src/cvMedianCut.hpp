#ifndef __CV_MEDIAN_CUT_HPP__
#define __CV_MEDIAN_CUT_HPP__

#include "MedianCut.hpp"

#include <opencv2/opencv.hpp>

PRJ_BEGIN

class cvMedianCut : public MedianCut<cv::Vec3b> {
protected:
  typedef cv::Vec3b T;
  typedef unsigned char unchar;
  using MedianCut<T>::Cluster;

public:
  cvMedianCut(SizeType nClusters)
    : MedianCut<T>(nClusters)
  {
  }
  virtual ~cvMedianCut() {}

protected:
  virtual void split(const Cluster & c, Cluster & c1, Cluster & c2)
  {
    // first, determine which dimension to split
    SizeType component = 0;
    unchar max_l = c.getMaximum(0)[0] - c.getMinimum(0)[0];
    for ( SizeType i=1; i < 3; i++ )
    {
      unchar l = c.getMaximum(i)[i] - c.getMinimum(i)[i];
      if ( l > max_l )
      {
        max_l = l;
        component = i;
      }
    }

    // second, find median of that dimension
    unchar median = 0;
    std::vector<unchar> t;
    for ( SizeType i=0; i < c.indices.size(); i++ )
    {
      t.push_back((*_data)[c.indices[i]][component]);
    }
    median = findMedian<unchar>(t, 0, t.size());

    // last, split according to the median
    for ( SizeType i=0; i < c.indices.size(); i++ )
    {
      if ( (*_data)[c.indices[i]][component] >= median )
      {
        c1.indices.push_back(c.indices[i]);
      }
      else
      {
        c2.indices.push_back(c.indices[i]);
      }
    }
  }

  virtual void generate_results()
  {
    for ( SizeType i=0; i < _clusters.size(); i++ )
    {
      SizeType size = _clusters[i].indices.size();
      SizeType b = 0;
      SizeType g = 0;
      SizeType r = 0;
      for ( SizeType j=0; j < size; j++ )
      {
        b += (*_data)[_clusters[i].indices[j]][0];
        g += (*_data)[_clusters[i].indices[j]][1];
        r += (*_data)[_clusters[i].indices[j]][2];
      }
      b /= size;
      g /= size;
      r /= size;
      for ( SizeType j=0; j < size; j++ )
      {
        _results[_clusters[i].indices[j]] =  T(b,g,r);
      }
    }
  }

};

PRJ_END

#endif //__CV_MEDIAN_CUT_HPP__
