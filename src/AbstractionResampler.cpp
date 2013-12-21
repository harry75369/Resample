#include "AbstractionResampler.hpp"

#include <cmath>

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

void AbstractionResampler::initialize(const SizeType w, const SizeType h)
{
  INFO("initialize()");

  // prepare input and output
  _input_width = _input.cols;
  _input_height = _input.rows;
  _output_width = w;
  _output_height = h;
  _input_area = Real(_input_width*_input_height);
  _output_area = Real(_output_width*_output_height);
  bgr2lab(_input, _input_lab);
  _output_lab = cv::Mat(cv::Size(w, h), CV_32FC3, cv::Scalar(0.0));

  // prepare intermediate variables
  _converged = false;
  _palette_maxed = false;
  _iteration = 0;
  _range = std::sqrt(_input_area/_output_area);

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
  for ( SizeType i=0; i < _output_width; i++ )
    for ( SizeType j=0; j < _output_height; j++ )
    {
      SuperPixel p(i*h+j);
      p.position = cv::Vec2f((i+0.5)/_output_width, (j+0.5)/_output_height);
      p.color = cv::Vec3f(0.0, 0.0, 0.0);
      _superpixels.push_back(p);

      // build mapping from pixels to this superpixel
      for ( SizeType x=i*sx; x < (i+1)*sx; x++ )
        for ( SizeType y=j*sy; y < (j+1)*sy; y++ )
        {
          _pixel_map[x][y] = p.id;
        }
    }
  update_superpixels();

  // init palette
  cv::Vec3f first_color(0.0, 0.0, 0.0);
  for ( SizeType k=0; k < _superpixels.size(); k++ )
  {
    first_color += _superpixels[k].color;
  }
  first_color /= _output_area;
  _palette.clear();
  _palette.push_back(first_color);

  // init MCDA
  _prob_o = 1.0 / _output_area;
  _prob_c.clear();
  _prob_c.push_back(0.5);
  _prob_c.push_back(0.5);
  _prob_co.clear();
  _prob_co.push_back(std::vector<Real>(_output_width*_output_height, 0.5));
  _prob_co.push_back(std::vector<Real>(_output_width*_output_height, 0.5));
  _palette.push_back(first_color + 0.8 * get_max_eigen(0).first);
  _sub_superpixel_pairs.clear();
  _sub_superpixel_pairs.push_back(std::pair<SizeType,SizeType>(0,1));
  _temperature = 1.1 * std::sqrt(2*get_max_eigen(0).second);
}

bool AbstractionResampler::is_done()
{
  INFO("is_done()");

  if ( _iteration > 100 )
  {
    return true;
  }
  return _converged;
}

void AbstractionResampler::iterate()
{
  INFO("iterate(): %lu", _iteration++);

  remap_pixels();
  update_superpixels();

  associate_superpixels();
  Real err = refine_palette();
  if ( err < 1.0 )
  {
    if ( _temperature <= 1.0 )
    {
      _converged = true;
    }
    else
    {
      _temperature = std::max(1.0, 0.7*_temperature);
    }
    expand_palette();
  }
}

void AbstractionResampler::finalize()
{
  INFO("finalize()");

  const std::vector<cv::Vec3f> & averaged_palette = get_averaged_palette();
  for ( SizeType i=0; i < _output_width; i++ )
    for ( SizeType j=0; j < _output_height; j++ )
    {
      const SuperPixel & sp = _superpixels[i*_output_height+j];
      _output_lab.at<cv::Vec3f>(j, i) = averaged_palette[sp.assoc];
	  _output_lab.at<cv::Vec3f>(j, i)[1] *= 1.1;
	  _output_lab.at<cv::Vec3f>(j, i)[2] *= 1.1;
    }
  lab2bgr(_output_lab, _output);
}

void AbstractionResampler::visualizeSuperpixel(cv::Mat & output)
{
  INFO("visualizeSuperpixel()");

#ifdef ALL_NEIGHBORS
  const int n_neighbors = 8;
  const int dx[n_neighbors] = {-1, -1,  0,  1, 1, 1, 0, -1};
  const int dy[n_neighbors] = { 0, -1, -1, -1, 0, 1, 1,  1};
#else
  const int n_neighbors = 5;
  const int dx[n_neighbors] = {-1,  0,  1, 1, 1};
  const int dy[n_neighbors] = {-1, -1, -1, 0, 1};
#endif
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
  const std::vector<cv::Vec3f> & averaged_palette = get_averaged_palette();
  for ( SizeType k=0; k < _superpixels.size(); k++ )
  {
    const SizeType x = _superpixels[k].position[0]*_input_width;
    const SizeType y = _superpixels[k].position[1]*_input_height;
    const SizeType x0 = std::max(Real(0), x-_range);
    const SizeType y0 = std::max(Real(0), y-_range);
    const SizeType x1 = std::min(Real(_input_width), x+_range);
    const SizeType y1 = std::min(Real(_input_height), y+_range);
    const cv::Vec3f color = averaged_palette[_superpixels[k].assoc];
    for ( SizeType i=x0; i < x1; i++ )
      for ( SizeType j=y0; j < y1; j++ )
      {
        const Real d = slic_distance(i, j, cv::Vec2f(x, y), color);
        if ( dmap[i][j] > d || dmap[i][j] < 0 )
        {
          dmap[i][j] = d;
          _pixel_map[i][j] = k;
        }
      }
  }
}

void AbstractionResampler::update_superpixels()
{
  INFO("update_superpixels()");

  std::vector<SizeType> counter;
  const std::vector<SuperPixel> superpixels = _superpixels;
  for ( SizeType k=0; k < _superpixels.size(); k++ )
  {
    _superpixels[k].position = cv::Vec2f(0.0, 0.0);
    _superpixels[k].color = cv::Vec3f(0.0, 0.0, 0.0);
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
    if ( counter[k] )
    {
      _superpixels[k].position /= Real(counter[k]);
      _superpixels[k].color /= Real(counter[k]);
    }
    else
    {
      _superpixels[k].position = superpixels[k].position;
      SizeType x = superpixels[k].position[0] * _input_width;
      SizeType y = superpixels[k].position[1] * _input_height;
      _superpixels[k].color = _input_lab.at<cv::Vec3f>(y, x);
    }
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

void AbstractionResampler::associate_superpixels()
{
  INFO("associate_superpixels()");

  const SizeType palette_size = _palette.size();
  std::vector<Real> new_prob_c(palette_size, 0.0);
  _prob_co = std::vector<std::vector<Real> >(palette_size,
      std::vector<Real>(_output_width*_output_height, 0.0));
  const Real overT = -1.0/_temperature;

  for ( SizeType k=0; k < _superpixels.size(); k++ )
  {
    SizeType best_index = palette_size;
    Real best_error;
    std::vector<Real> probs;
    const cv::Vec3f pixel = _superpixels[k].color;
    Real sum_prob(0);

    for ( SizeType i=0; i < palette_size; i++ )
    {
      Real color_error = cv::norm(_palette[i], pixel);
      Real prob = _prob_c[i] * std::exp(color_error*overT);
      probs.push_back(prob);
      sum_prob += prob;
      if ( best_index == palette_size || color_error < best_error )
      {
        best_index = i;
        best_error = color_error;
      }
    }
    _superpixels[k].assoc = best_index;
    for ( SizeType i=0; i < palette_size; i++ )
    {
      Real norm_prob = probs[i] / sum_prob;
      _prob_co[i][k] = norm_prob;
      new_prob_c[i] += norm_prob * _prob_o;
    }
  }
  _prob_c.swap(new_prob_c);
}

Real AbstractionResampler::refine_palette()
{
  INFO("refine_palette()");

  std::vector<cv::Vec3d> color_sums(_palette.size(), cv::Vec3d(0.0, 0.0, 0.0));

  for ( SizeType k=0; k < _superpixels.size(); k++ )
  {
    for ( SizeType i=0; i < _palette.size(); i++ )
    {
      color_sums[i] += _superpixels[k].color * _prob_co[i][k] * _prob_o;
    }
  }
  Real palette_error(0);
  for ( SizeType i=0; i < color_sums.size(); i++ )
  {
    ASSERT(_prob_c[i] > 0);
    cv::Vec3d color = _palette[i];
    cv::Vec3d new_color = color_sums[i] / _prob_c[i];
    palette_error += cv::norm(color, new_color);
    _palette[i] = new_color;
  }

  return palette_error;
}

void AbstractionResampler::expand_palette()
{
  INFO("expand_palette()");

  if ( _palette_maxed ) return;

  std::vector<std::pair<Real, SizeType> > splits;
  SizeType num_subclusters = _sub_superpixel_pairs.size();
  for ( SizeType index=0; index < num_subclusters; index++ )
  {
    SizeType index_1 = _sub_superpixel_pairs[index].first;
    SizeType index_2 = _sub_superpixel_pairs[index].second;
    cv::Vec3f color_1 = _palette[index_1];
    cv::Vec3f color_2 = _palette[index_2];

    Real cluster_error = cv::norm(color_1, color_2);
    if ( cluster_error > 1.6 )
    {
      splits.push_back(std::pair<Real, SizeType>(cluster_error, index));
    }
    else
    {
      _palette[index_2] += get_max_eigen(index_1).first * 0.8;
    }
  }

  std::sort(splits.begin(), splits.end());
  for ( auto i=splits.rbegin(); i != splits.rend(); i++ )
  {
    split_color((*i).second);
    if ( _palette.size() >= 2*_nColors )
    {
      condense_palette();
      break;
    }
  }
}

void AbstractionResampler::split_color(SizeType index)
{
  INFO("split_color()");

  const SizeType index_1 = _sub_superpixel_pairs[index].first;
  const SizeType index_2 = _sub_superpixel_pairs[index].second;
  const SizeType next_index1 = _palette.size();
  const SizeType next_index2 = _palette.size()+1;
  const cv::Vec3f color_1 = _palette[index_1];
  const cv::Vec3f color_2 = _palette[index_2];
  const cv::Vec3f subcluster_color1 = color_1 + get_max_eigen(index_1).first * 0.8;
  const cv::Vec3f subcluster_color2 = color_2 + get_max_eigen(index_2).first * 0.8;

  _palette.push_back(subcluster_color1);
  _sub_superpixel_pairs[index].second = next_index1;
  _prob_c[index_1] *= 0.5;
  _prob_c.push_back(_prob_c[index_1]);
  _prob_co.push_back(_prob_co[index_1]);

  _palette.push_back(subcluster_color2);
  const std::pair<SizeType, SizeType> new_pair(index_2, next_index2);
  _sub_superpixel_pairs.push_back(new_pair);
  _prob_c[index_2] *= 0.5;
  _prob_c.push_back(_prob_c[index_2]);
  _prob_co.push_back(_prob_co[index_2]);
}

void AbstractionResampler::condense_palette()
{
  INFO("condense_palette()");

  _palette_maxed = true;
  std::vector<cv::Vec3f> old_palette = _palette;
  std::vector<cv::Vec3f> new_palette;
  std::vector<std::vector<Real> > new_prob_co;
  std::vector<Real> new_prob_c;
  for ( SizeType j = 0; j < _sub_superpixel_pairs.size(); j++ )
  {
    const SizeType index_1 = _sub_superpixel_pairs[j].first;
    const SizeType index_2 = _sub_superpixel_pairs[j].second;
    Real weight_1 = _prob_c[index_1];
    Real weight_2 = _prob_c[index_2];
    const Real total_weight = weight_1+weight_2;
    weight_1 /= total_weight;
    weight_2 /= total_weight;
    new_palette.push_back((old_palette[index_1] * weight_1) +
                          (old_palette[index_2] * weight_2));
    new_prob_c.push_back(_prob_c[index_1] + _prob_c[index_2]);
    new_prob_co.push_back(_prob_co[index_1]);

    for ( SizeType k=0; k < _superpixels.size(); k++ )
    {
      if ( _superpixels[k].assoc == index_1
        || _superpixels[k].assoc == index_2 )
      {
        _superpixels[k].assoc = j;
      }
    }
  }
  _palette.swap(new_palette);
  _prob_c.swap(new_prob_c);
  _prob_co.swap(new_prob_co);
}

Real AbstractionResampler::slic_distance(SizeType i, SizeType j, const cv::Vec2f & pos, const cv::Vec3f & spcolor) const
{
  Real dx = Real(i) - pos[0];
  Real dy = Real(j) - pos[1];
  cv::Vec3f color = _input_lab.at<cv::Vec3f>(j, i);
  Real color_error = cv::norm(color, spcolor);
  Real dist_error = cv::norm(cv::Vec2f(dx, dy));
  return color_error + 45.0 / _range * dist_error;
}

std::pair<cv::Vec3f, Real> AbstractionResampler::get_max_eigen(SizeType pidx)
{
  //for every output pixel
  cv::Mat matrix(cv::Size(3,3), CV_64FC1, cv::Scalar(0.0));
  Real sum(0);
  for ( SizeType y = 0; y < _output_height; y++ )
    for ( SizeType x = 0; x < _output_width; x++ ) {
      //get prob(output pixel|palette color)
      Real prob_oc = _prob_co[pidx][x*_output_height+y] * _prob_o / _prob_c[pidx];
      sum += prob_oc;
      //construct 3x3 matrix and add to sum
      cv::Vec3d color_error = _palette[pidx] - _superpixels[x*_output_height+y].color;
      color_error[0] = std::abs(color_error[0]);
      color_error[1] = std::abs(color_error[1]);
      color_error[2] = std::abs(color_error[2]);
      matrix.at<double>(0,0) += prob_oc*color_error[0]*color_error[0];
      matrix.at<double>(1,0) += prob_oc*color_error[1]*color_error[0];
      matrix.at<double>(2,0) += prob_oc*color_error[2]*color_error[0];
      matrix.at<double>(0,1) += prob_oc*color_error[0]*color_error[1];
      matrix.at<double>(1,1) += prob_oc*color_error[1]*color_error[1];
      matrix.at<double>(2,1) += prob_oc*color_error[2]*color_error[1];
      matrix.at<double>(0,2) += prob_oc*color_error[0]*color_error[2];
      matrix.at<double>(1,2) += prob_oc*color_error[1]*color_error[2];
      matrix.at<double>(2,2) += prob_oc*color_error[2]*color_error[2];
    }

  //get critical temperature = largest eigenvalue of convariance matrix
  cv::Mat values;
  cv::Mat vectors;
  cv::eigen(matrix,values, vectors);

  cv::Vec3f eVec = cv::Vec3f(vectors.at<double>(0,0),
    vectors.at<double>(0,1),
    vectors.at<double>(0,2));
  float len = norm(eVec);
  if(len > 0)
    eVec *= (1.0/len);
  float eVal = abs(values.at<double>(0,0));

  return std::pair<cv::Vec3f, float>(eVec, eVal);
}

std::vector<cv::Vec3f> AbstractionResampler::get_averaged_palette()
{
  std::vector<cv::Vec3f> averaged_palette = _palette;
  if ( !_palette_maxed ) {
    for( SizeType i = 0; i< _sub_superpixel_pairs.size(); ++i )
    {
      const SizeType index_1 = _sub_superpixel_pairs[i].first;
      const SizeType index_2 = _sub_superpixel_pairs[i].second;
      const cv::Vec3f color_1 = _palette[index_1];
      const cv::Vec3f color_2 = _palette[index_2];
      Real weight_1 = _prob_c[index_1];
      Real weight_2 = _prob_c[index_2];
      const Real total_weight = weight_1+weight_2;
      weight_1 /= total_weight;
      weight_2 /= total_weight;

      const cv::Vec3f average_color(weight_1*color_1[0]+weight_2*color_2[0],
                                    weight_1*color_1[1]+weight_2*color_2[1],
                                    weight_1*color_1[2]+weight_2*color_2[2]);

      averaged_palette[index_1] = average_color;
      averaged_palette[index_2] = average_color;
    }
  }
  return averaged_palette;
}
