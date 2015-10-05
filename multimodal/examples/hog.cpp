#include <cv/base/io.h>
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

  const char* hname = "..//..//data//homography.txt";
  cv::Mat H;
  if (!readHomography(hname, &H)) {
    std::cout << "Error reading Homography" << std::endl;
    return 1;
  };

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
  int size = 64 * 3;
  cv::Rect roi_ir(left.cols / 2 - size/2, left.rows / 2 - size/2, size, size);
  cv::Mat object(left(roi_ir));
  computeScaleFromH(H, size, &settings);
  cv::Rect roi_vis(right.cols / 4, right.rows/4, right.cols /2, right.rows/2);
  cv::Mat scene(right(roi_vis));

  cv::Rect rect;
  findClosestHOG(object, scene, settings, &rect);

  cv::rectangle(scene, rect, cv::Scalar(255, 253, 34), 1);
  cv::imwrite("right.png", scene);
  cv::imwrite("patch.png", object);

  return 0;
}