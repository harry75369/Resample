#include "MedianCut.hpp"

#include <vector>
#include <iostream>
#include <algorithm>

USE_PRJ_NAMESPACE;

int main()
{
  int i;
  std::vector<int> t;

  while ( std::cin >> i )
  {
    t.push_back(i);
  }

  std::cout << "median = " << MedianCut<int>::findMedian<int>(t, 0, t.size()) << std::endl;

  std::make_heap(t.begin(), t.end());
  std::sort_heap(t.begin(), t.end());
  for ( auto i=t.begin(); i != t.end(); i++ )
  {
    std::cout << *i << " ";
  }
  std::cout << std::endl;

  return 0;
}
