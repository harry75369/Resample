#include "AbstractionResampler.hpp"

#include <cmath>

USE_PRJ_NAMESPACE;

void AbstractionResampler::resample(SizeType w, SizeType h)
{
  initialize(w, h);

  while ( !is_done() )
  {
    iterate();
  }

  finalize();
}

void AbstractionResampler::initialize(SizeType w, SizeType h)
{
  // prepare input and output
  _input_width = _input.cols;
  _input_height = _input.rows;
  _output_width = w;
  _output_height = h;
  bgr2lab(_input, _input_lab);
  _output_lab = cv::Mat(cv::Size(w, h), CV_32FC3, cv::Scalar(1.f));

  // prepare intermediate variables
  _iteration = 0;
  _range = std::sqrt((Real)_input_width*_input_height/((Real)w*h));

  // init palette
  cv::Vec3f first_color(.0f, .0f, .0f);
  for ( SizeType i=0; i < _input_width; i++ )
    for ( SizeType j=0; j < _input_height; j++ )
    {
      first_color += _input_lab.at<cv::Vec3f>(j, i);
    }
  _palette.clear();
  _palette.push_back(first_color/Real(_input_width*_input_height));

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
  const SizeType sx = _input_width / _output_width;
  const SizeType sy = _input_height / _output_height;
  for ( SizeType i=0; i < w; i++ )
    for ( SizeType j=0; j < h; j++ )
    {
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
    }
}

bool AbstractionResampler::is_done()
{
  if ( _iteration < 1 )
  {
    return false;
  }
  return true;
}

void AbstractionResampler::iterate()
{
  _iteration++;
}

void AbstractionResampler::finalize()
{
  lab2bgr(_output_lab, _output);
}

void AbstractionResampler::visualizeSuperpixel(cv::Mat & output)
{
  const int n_neighbors = 8;
  const int dx[n_neighbors] = {-1, -1,  0,  1, 1, 1, 0, -1};
  const int dy[n_neighbors] = { 0, -1, -1, -1, 0, 1, 1,  1};
  output = _input.clone(); // in BGR space with type CV_8UC3

  // superpixel center
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

  // palette color
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
