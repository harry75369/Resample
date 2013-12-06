#ifndef __MEDIAN_CUT_HPP__
#define __MEDIAN_CUT_HPP__

#include "Config.hpp"

#include <vector>
#include <map>
#include <algorithm>

PRJ_BEGIN

template <typename T>
class MedianCut {
protected:
  struct Cluster {
    const std::vector<T> * data;
    std::vector<SizeType> indices;

    Cluster(const std::vector<T> * data, const std::vector<SizeType> & indices=std::vector<SizeType>())
      : data(data), indices(indices)
    {
    }

    bool operator>(const Cluster & other) const
    {
      SizeType s1 = this->indices.size();
      SizeType s2 = other.indices.size();
      if ( s1 > s2 )
      {
        return true;
      }
      return false;
    }

    bool operator<(const Cluster & other) const
    {
      return other>(*this);
    }

    const T & getMinimum(SizeType component) const
    {
      SizeType idx = 0;
      const T * minimum = &(*data)[indices[0]];
      for ( SizeType i=1; i < indices.size(); i++ )
      {
        const T * t = &(*data)[indices[i]];
        if ( (*minimum)[component] > (*t)[component] )
        {
          idx = i;
          minimum = t;
        }
      }
      return (*minimum);
    }

    const T & getMaximum(SizeType component) const
    {
      SizeType idx = 0;
      const T * maximum = &(*data)[indices[0]];
      for ( SizeType i=1; i < indices.size(); i++ )
      {
        const T * t = &(*data)[indices[i]];
        if ( (*maximum)[component] < (*t)[component] )
        {
          idx = i;
          maximum = t;
        }
      }
      return (*maximum);
    }

  };

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
    // set data
    _data = &data;

    // clean up
    _clusters.clear();
    _results.clear();

    // init first cluster with indices
    std::vector<SizeType> indices;
    for ( SizeType i=0; i < data.size(); i++ )
    {
      indices.push_back(i);
    }
    _clusters.push_back(Cluster(_data, indices));
    std::make_heap(_clusters.begin(), _clusters.end());

    // main loop
    while ( _clusters.size() < _nClusters )
    {
      Cluster c1(_data), c2(_data);
      std::pop_heap(_clusters.begin(), _clusters.end());
      split(_clusters.back(), c1, c2);
      _clusters.pop_back();
      _clusters.push_back(c1);
      std::push_heap(_clusters.begin(), _clusters.end());
      _clusters.push_back(c2);
      std::push_heap(_clusters.begin(), _clusters.end());
    }

    // generate results
    generate_results();
  }

  /** find median
   *
   * for odd list, it picks the middle one
   * for even list, it prefers the right middle one
   */
  template <typename TT>
  static TT findMedian(std::vector<TT> t, SizeType l, SizeType r)
  {
    if ( r-l < 2 ) return t[l];
    TT pivot = t[l];
    SizeType i, idx;
    for ( i=idx=l+1; i < r; i++ )
    {
      if ( t[i] < pivot )
      {
        std::swap(t[idx++], t[i]);
      }
    }
    std::swap(t[idx-1], t[l]);
    if ( idx <= t.size()/2 )
    {
      return findMedian(t, idx, r);
    }
    else
    {
      return findMedian(t, l, idx);
    }
  }

protected:
  virtual void split(const Cluster & c, Cluster & c1, Cluster & c2) = 0;
  virtual void generate_results() = 0;

protected:
  const SizeType _nClusters;
  const std::vector<T> * _data;
  std::vector<Cluster> _clusters;
  std::map<SizeType, T> _results;

};

PRJ_END

#endif //__MEDIAN_CUT_HPP__
