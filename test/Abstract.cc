#include "AbstractionResampler.hpp"
#include "NearestResampler.hpp"

#include <opencv2/opencv.hpp>

USE_PRJ_NAMESPACE;

int main(int argc, char * argv[])
{
  if ( argc < 2 )
  {
    INFO("Please give me an image.");
    return -1;
  }

  AbstractionResampler resampler(8);
  ASSERT_MSG(resampler.open(argv[1]), "No image data");

  SizeType w0 = resampler.getInput().cols;
  SizeType h0 = resampler.getInput().rows;
  SizeType w1 = w0 / 24;
  SizeType h1 = h0 / 24;

  resampler.resample(w1, h1);

  cv::namedWindow("Origin", CV_WINDOW_AUTOSIZE);
  cv::imshow("Origin", resampler.getInput());

  cv::namedWindow("SuperPixel", CV_WINDOW_AUTOSIZE);
  cv::Mat superpixel;
  resampler.visualizeSuperpixel(superpixel);
  cv::imshow("SuperPixel", superpixel);

  NearestResampler recoverer;
  recoverer.load(resampler.getOutput());
  recoverer.resample(w0, h0);
  recoverer.save("abstracted.png");
  resampler.save("abstracted_small.png");
  cv::namedWindow("Abstracted", CV_WINDOW_AUTOSIZE);
  cv::imshow("Abstracted", recoverer.getOutput());

  cv::waitKey(0);

  return 0;
}
