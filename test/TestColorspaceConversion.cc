#include "AbstractionResampler.hpp"

#include <opencv2/opencv.hpp>

USE_PRJ_NAMESPACE;

int main(int argc, const char * argv[])
{
  cv::Mat image;
  image = cv::imread(argv[1], 1);

  if( argc != 2 || !image.data )
  {
    printf("No image data \n");
    return -1;
  }

  cv::namedWindow("Origin", CV_WINDOW_AUTOSIZE);
  cv::imshow("Origin", image);

  cv::Mat lab;
  AbstractionResampler::bgr2lab(image, lab);
  cv::namedWindow("Lab", CV_WINDOW_AUTOSIZE);
  cv::imshow("Lab", lab);
#if 0
  for ( int j=0; j < lab.rows; j++ )
  {
    for ( int i=0; i < lab.cols; i++ )
    {
      cv::Vec3f c = lab.at<cv::Vec3f>(j, i);
      printf("(%.2f, %.2f, %.2f) ", c[0], c[1], c[2]);
    }
    printf("\n");
  }
#endif
  cv::Mat bgr;
  AbstractionResampler::lab2bgr(lab, bgr);
  cv::namedWindow("BGR", CV_WINDOW_AUTOSIZE);
  cv::imshow("BGR", bgr);

  cv::waitKey(0);

  return 0;
}
