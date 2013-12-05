#include "NearestResampler.hpp"
#include "BilinearResampler.hpp"
#include "BicubicResampler.hpp"
#include "LanczosResampler.hpp"

#include <string>
#include <opencv2/opencv.hpp>

USE_PRJ_NAMESPACE;

#define NRESAMPLERS 4
const char * resampler_names[NRESAMPLERS] = {
  "Nearest",
  "Bilinear",
  "Bicubic",
  "Lanczos"
};

int main(int argc, char * argv[])
{
  Resampler * resamplers[NRESAMPLERS];

  if ( argc < 2 )
  {
    INFO("Please give me an image.");
    return -1;
  }

  resamplers[0] = new NearestResampler(8);
  resamplers[1] = new BilinearResampler(8);
  resamplers[2] = new BicubicResampler(8);
  resamplers[3] = new LanczosResampler(8);

  for ( int i=0; i < NRESAMPLERS; i++ )
  {
    resamplers[i]->open(argv[1]);
  }

  SizeType w0 = resamplers[0]->getInput().cols;
  SizeType h0 = resamplers[0]->getInput().rows;
  SizeType w1 = w0 / 12;
  SizeType h1 = h0 / 12;

  for ( int i=0; i < NRESAMPLERS; i++ )
  {
    resamplers[i]->resample(w1, h1);
  }

  cv::namedWindow("Origin", CV_WINDOW_AUTOSIZE);
  cv::imshow("Origin", resamplers[0]->getInput());

  NearestResampler recoverer;
  for ( int i=0; i < NRESAMPLERS; i++ )
  {
    std::string name = std::string(resampler_names[i]);
    recoverer.load(resamplers[i]->getOutput());
    recoverer.resample(w0, h0);
    recoverer.save(name+".png");
    resamplers[i]->save(name+"_small.png");
    cv::namedWindow(name, CV_WINDOW_AUTOSIZE);
    cv::imshow(name, recoverer.getOutput());
  }

  cv::waitKey(0);

  return 0;
}
