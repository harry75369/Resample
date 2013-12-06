#ifndef __MEDIAN_CUT_HPP__
#define __MEDIAN_CUT_HPP__

#include "Config.hpp"

#include <vector>
#include <map>

PRJ_BEGIN

template <typename T>
class MedianCut {
protected:
  typedef std::vector<SizeType> Cluster;

public:
  MedianCut(SizeType nClusters)
    : _nClusters(nClusters)
  {
#if 0
    // the number of clusters should be power of 2.
    // if not, promote it to be power of 2.
    // e.g. 0 => 1, 3 => 4, 5 => 8
    while ( !_nClusters || (_nClusters & (_nClusters-1)) )
    {
      _nClusters++;
    }
    INFO("MedianCut: number of clusters = %u", _nClusters);
#endif
  }
  virtual ~MedianCut() {}

  inline T & getResult(const SizeType & index)
  {
    return _results[index];
  }

  void process(const std::vector<T> & data)
  {
    // clean up
    _clusters.clear();
    _results.clear();

    // init indices
    std::vector<SizeType> indices;
    for ( SizeType i=0; i < data.size(); i++ )
    {
      indices.push_back(i);
    }
    _clusters.push_back(indices);

    // main loop
    while ( _clusters.size() < _nClusters )
    {
      Cluster c1, c2;
      split(_clusters[0], data, c1, c2);
      _clusters.erase(_clusters.begin());
      _clusters.push_back(c1);
      _clusters.push_back(c2);
    }

    // post processing
    post_process(data);
  }

protected:
  virtual void split(const Cluster & c, const std::vector<T> & data, Cluster & c1, Cluster & c2) = 0;
  virtual void post_process(const std::vector<T> & data) = 0;

protected:
  SizeType _nClusters;
  std::vector<Cluster> _clusters;
  std::map<SizeType, T> _results;

};

PRJ_END

#endif //__MEDIAN_CUT_HPP__
