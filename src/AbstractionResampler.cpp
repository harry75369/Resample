#include "AbstractionResampler.hpp"

#include <cmath>

//#define RANDOM_SUPERPIXELS
#ifdef RANDOM_SUPERPIXELS
#include <stdlib.h>
#include <time.h>
#endif

USE_PRJ_NAMESPACE;

void AbstractionResampler::resample(SizeType w, SizeType h)
{
  INFO("resample()");
  initialize(w, h);

  while ( !is_done() )
  {
    iterate();
  }

  finalize();
}

void AbstractionResampler::initialize(SizeType w, SizeType h)
{
  INFO("initialize()");
#ifdef RANDOM_SUPERPIXELS
  srand(time(0));
#endif

  // prepare input and output
  _input_width = _input.cols;
  _input_height = _input.rows;
  _output_width = w;
  _output_height = h;
  _input_area = (Real)_input_width*_input_height;
  _output_area = (Real)_output_width*_output_height;
  bgr2lab(_input, _input_lab);
  _output_lab = cv::Mat(cv::Size(w, h), CV_32FC3, cv::Scalar(0.f));

  // prepare intermediate variables
  _iteration = 0;
  _range = std::sqrt(_input_area/_output_area);

  // init palette
  cv::Vec3f first_color(.0f, .0f, .0f);
  for ( SizeType i=0; i < _input_width; i++ )
    for ( SizeType j=0; j < _input_height; j++ )
    {
      first_color += _input_lab.at<cv::Vec3f>(j, i);
    }
  _palette.clear();
  _palette.push_back(first_color/_input_area);

  // init superpixels and pixel map
  _superpixels.clear();
  _pixel_map.clear();
  for ( SizeType i=0; i < _input_width; i++ )
  {
    std::vector<SizeType> col;
    for ( SizeType j=0; j < _input_height; j++)
    {
      col.push_back(0);
    }
    _pixel_map.push_back(col);
  }
  const Real sx = (Real)_input_width / _output_width;
  const Real sy = (Real)_input_height / _output_height;
  for ( SizeType i=0; i < w; i++ )
    for ( SizeType j=0; j < h; j++ )
    {
#ifndef RANDOM_SUPERPIXELS
      SuperPixel p(i*h+j);
      p.position = cv::Vec2f((i+.5f)/w, (j+.5f)/h);
      p.color = _palette[0];
      _superpixels.push_back(p);

      // build mapping from pixels to this superpixel
      for ( SizeType x=i*sx; x < (i+1)*sx; x++ )
        for ( SizeType y=j*sy; y < (j+1)*sy; y++ )
        {
          _pixel_map[x][y] = p.id;
        }
#else
      SuperPixel p(i*h+j);
      p.position = cv::Vec2f(Real(rand())/RAND_MAX, Real(rand())/RAND_MAX);
      p.color = _palette[0];
      _superpixels.push_back(p);
#endif
    }
  update_superpixels();
}

bool AbstractionResampler::is_done()
{
  INFO("is_done()");
  if ( _iteration < 1 )
  {
    return false;
  }
  return true;
}

void AbstractionResampler::iterate()
{
  INFO("iterate()");
  _iteration++;

  remap_pixels();
  update_superpixels();
}

void AbstractionResampler::finalize()
{
  INFO("finalize()");
  lab2bgr(_output_lab, _output);
}

void AbstractionResampler::visualizeSuperpixel(cv::Mat & output)
{
  INFO("visualizeSuperpixel()");
  const int n_neighbors = 8;
  const int dx[n_neighbors] = {-1, -1,  0,  1, 1, 1, 0, -1};
  const int dy[n_neighbors] = { 0, -1, -1, -1, 0, 1, 1,  1};
  output = _input.clone(); // in BGR space with type CV_8UC3

  // palette color
  //if ( false )
  {
    for ( SizeType id=0; id < _palette.size(); id++ )
    {
      for ( int i=0; i < output.cols; i++ )
        for ( int j=0; j < output.rows; j++ )
          if ( _pixel_map[i][j] == id )
          {
            output.at<cv::Vec3b>(j, i) = lab2bgr(_palette[id]);
          }
    }
  }

  // contour
  for ( int i=0; i < output.cols; i++ )
    for ( int j=0; j < output.rows; j++ )
    {
      SizeType id = _pixel_map[i][j];
      SizeType cnt = 0;
      for ( int k=0; k < n_neighbors; k++ )
      {
        int x = i + dx[k];
        int y = j + dy[k];
        if ( 0 <= x && x < output.cols &&
             0 <= y && y < output.rows &&
             _pixel_map[x][y] != id )
        {
          cnt++;
        }
      }
      if ( cnt > 1 )
      {
        output.at<cv::Vec3b>(j, i) = cv::Vec3b(0, 0, 255);
      }
    }

  // superpixel center
  //if ( false )
  {
    for ( SizeType i=0; i < _superpixels.size(); i++ )
    {
      const SuperPixel & sp = _superpixels[i];
      const int x = (int)(sp.position[0]*_input_width);
      const int y = (int)(sp.position[1]*_input_height);
      output.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 255, 0);
      for ( int k=0; k < n_neighbors; k++ )
      {
        int xx = x+dx[k];
        int yy = y+dy[k];
        if ( 0 <= xx && xx < output.cols &&
             0 <= yy && yy < output.rows )
        {
          output.at<cv::Vec3b>(yy, xx) = cv::Vec3b(0, 255, 0);
        }
      }
    }
  }
}

void AbstractionResampler::remap_pixels()
{
  INFO("remap_pixels()");
  std::vector<std::vector<Real> > dmap;
  for ( SizeType i=0; i < _input_width; i++ )
  {
    std::vector<Real> col;
    for ( SizeType j=0; j < _input_height; j++ )
    {
      col.push_back(Real(-1));
    }
    dmap.push_back(col);
  }
  std::vector<std::vector<bool> > fmap;
  for ( SizeType i=0; i < _input_width; i++ )
  {
    std::vector<bool> col;
    for ( SizeType j=0; j < _input_height; j++ )
    {
      col.push_back(false);
    }
    fmap.push_back(col);
  }
  for ( SizeType k=0; k < _superpixels.size(); k++ )
  {
#ifndef RANDOM_SUPERPIXELS
    const SizeType x = _superpixels[k].position[0]*_input_width;
    const SizeType y = _superpixels[k].position[1]*_input_height;
    const SizeType x0 = std::max(Real(0), x-_range);
    const SizeType y0 = std::max(Real(0), y-_range);
    const SizeType x1 = std::min(Real(_input_width), x+_range);
    const SizeType y1 = std::min(Real(_input_height), y+_range);
    for ( SizeType i=x0; i < x1; i++ )
      for ( SizeType j=y0; j < y1; j++ )
#else
    for ( SizeType i=0; i < _input_width; i++ )
      for ( SizeType j=0; j < _input_height; j++ )
#endif
      {
        fmap[i][j] = true;
        Real d = slic_distance(i, j, _superpixels[k]);
        if ( dmap[i][j] > d || dmap[i][j] < 0 )
        {
          dmap[i][j] = d;
          _pixel_map[i][j] = k;
        }
      }
  }
  for ( SizeType i=0; i < _input_width; i++ )
    for ( SizeType j=0; j < _input_height; j++ )
      if ( !fmap[i][j] )
      {
        WARN("not covered: (%lu, %lu)", i, j);
      }
}

void AbstractionResampler::update_superpixels()
{
  INFO("update_superpixels()");
  std::vector<SizeType> counter;
  for ( SizeType k=0; k < _superpixels.size(); k++ )
  {
    _superpixels[k].position = cv::Vec2f(0.0, 0.0);
    _superpixels[k].color = cv::Vec3f(0.0, 0.0);
    counter.push_back(0);
  }
  for ( SizeType i=0; i < _input_width; i++ )
    for ( SizeType j=0; j < _input_height; j++ )
    {
      SizeType id = _pixel_map[i][j];
      _superpixels[id].position += cv::Vec2f(Real(i)/_input_width, Real(j)/_input_height);
      _superpixels[id].color += _input_lab.at<cv::Vec3f>(j, i);
      counter[id]++;
    }
  for ( SizeType k=0; k < _superpixels.size(); k++ )
  {
    ASSERT(counter[k]);
    _superpixels[k].position /= Real(counter[k]);
    _superpixels[k].color /= Real(counter[k]);
  }

  // position smoothing
  std::vector<std::vector<cv::Vec2f> > p, newp;
  for ( SizeType i=0; i < _output_width; i++ )
  {
    std::vector<cv::Vec2f> col, newcol;
    for ( SizeType j=0; j < _output_height; j++ )
    {
      col.push_back(_superpixels[i*_output_height+j].position);
      newcol.push_back(cv::Vec2f(0.0, 0.0));
    }
    p.push_back(col);
    newp.push_back(newcol);
  }
  for ( SizeType i=0; i < _output_width; i++ )
    for ( SizeType j=0; j < _output_height; j++ )
    {
      Real c(0);
      if ( i > 0 ) { c+=1.0; newp[i][j] += p[i-1][j]; }
      if ( j > 0 ) { c+=1.0; newp[i][j] += p[i][j-1]; }
      if ( i < _output_width-1 ) { c+=1.0; newp[i][j] += p[i+1][j]; }
      if ( j < _output_height-1 ) { c+=1.0; newp[i][j] += p[i][j+1]; }
      ASSERT(c>Real(0));
      newp[i][j] /= c;
    }
  for ( SizeType i=0; i < _output_width; i++ )
    for ( SizeType j=0; j < _output_height; j++ )
    {
      _superpixels[i*_output_height+j].position = 0.6 * p[i][j] + 0.4 * newp[i][j];
    }

  // color smoothing
  cv::Mat c(cv::Size(_output_width, _output_height), CV_32FC3);
  cv::Mat newc;
  for ( SizeType i=0; i < _output_width; i++ )
    for ( SizeType j=0; j < _output_height; j++ )
    {
      c.at<cv::Vec3f>(j, i) = _superpixels[i*_output_height+j].color;
    }
  cv::bilateralFilter(c, newc, 3, 0, 0);
  for ( SizeType i=0; i < _output_width; i++ )
    for ( SizeType j=0; j < _output_height; j++ )
    {
      _superpixels[i*_output_height+j].color = newc.at<cv::Vec3f>(j, i);
    }
}

Real AbstractionResampler::slic_distance(SizeType i, SizeType j, SizeType id) const
{
  const SuperPixel sp = _superpixels[id];
  return slic_distance(i, j, sp);
}

Real AbstractionResampler::slic_distance(SizeType i, SizeType j, const SuperPixel & sp) const
{
  Real dx = Real(i) - sp.position[0]*_input_width;
  Real dy = Real(j) - sp.position[1]*_input_height;
#ifdef RANDOM_SUPERPIXELS
  return std::abs(dx) + std::abs(dy); // voronoi diagram with manhattan distance
  //return cv::norm(cv::Vec2f(dx, dy)); // voronoi diagram with euclidean distance
#else
  cv::Vec3f color = _input_lab.at<cv::Vec3f>(j, i);
  Real color_error = cv::norm(color, sp.color);
  Real dist_error = cv::norm(cv::Vec2f(dx, dy));
  return color_error + 45.0 / _range * dist_error;
#endif
}
