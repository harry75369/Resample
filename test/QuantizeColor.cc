#include "cvMedianCut.hpp"

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <vector>

USE_PRJ_NAMESPACE;

int main(int argc, char * argv[])
{
  cv::Mat image;
  image = cv::imread(argv[1], 1);

  if ( argc != 2 || !image.data )
  {
    printf("No image data \n");
    return -1;
  }

  cv::Mat origin = image.clone();
  cv::namedWindow("Original", CV_WINDOW_AUTOSIZE);
  cv::imshow("Original", origin);

  std::vector<cv::Vec3b> data;
  cvMedianCut cut(8);
  for ( int i=0; i < image.cols; i++ )
    for ( int j=0; j < image.rows; j++ )
    {
      data.push_back(image.at<cv::Vec3b>(j, i));
    }
  cut.process(data);
  for ( int i=0; i < image.cols; i++ )
    for ( int j=0; j < image.rows; j++ )
    {
      image.at<cv::Vec3b>(j, i) = cut.getResult(i*image.rows+j);
    }

  cv::namedWindow("Quantized", CV_WINDOW_AUTOSIZE);
  cv::imshow("Quantized", image);
  cv::imwrite("quantized.png", image);

  cv::waitKey(0);

  return 0;
}
