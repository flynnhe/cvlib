#include <cv/base/utils.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <iostream>

using namespace cv::base;

static void usage(const char* progname) {
  std::cout << "Usage: " << progname << " <cfg>" << std::endl;
}

int main(int argc, const char** argv)
{
  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }

  ConfigParser cfg(argv[1]);
  if (!cfg.isValid()) {
    std::cout << "Invalid cfg file" << std::endl;
    return 1;
  }

  HOGSettings settings;
  bool success = settings.configure(cfg);

  std::string leftpath = cfg.get<std::string>("left", "", "Paths");
  std::string rightpath = cfg.get<std::string>("right", "", "Paths");

  // load the input images
  cv::Mat left = cv::imread(leftpath, CV_LOAD_IMAGE_GRAYSCALE);
  cv::Mat right = cv::imread(rightpath, CV_LOAD_IMAGE_GRAYSCALE);
  if (!left.data || !right.data) {
    std::cout << "Error loading input images" << std::endl;
    return 1;
  }

  // select a random patch in the image and find the closest match
  // in the full image
  cv::resize(left, left, cv::Size(256, 256));
  cv::Mat patch(left(cv::Rect(35, 43, 128, 128)));

  cv::Rect rect;
  findClosestHOG(patch, left, settings, &rect);

  /*cv::rectangle(left, rect, cv::Scalar(255, 253, 34), 1);
  cv::imwrite("left.png", left);
  cv::imwrite("patch.png", patch);*/

  return 0;
}