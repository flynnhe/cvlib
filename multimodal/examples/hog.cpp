#include <cv/base/cfg_parser/cfg_parser.h>
#include <cv/base/utils.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <iostream>

using namespace cv::base;

struct Options {
  int cells_per_block;
  int cells_per_image_w;
  int cells_per_image_h;
  int num_orientations;
};

static void usage(const char* progname) {
  std::cout << "Usage: " << progname << " <cfg>" << std::endl;
}

static bool configure(ConfigParser& cfg, Options* options) {
  // read HOG settings
  bool success = true;
  options->cells_per_block = cfg.get("cells_per_block", 2, "HOG");
  options->cells_per_image_w = cfg.get("cells_per_image_w", 8, "HOG");
  options->cells_per_image_h = cfg.get("cells_per_image_h", 8, "HOG");
  options->num_orientations = cfg.get("num_orientations", 9, "HOG");
  return success;
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

  Options options;
  bool success = configure(cfg, &options);

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
  findClosestHOG(patch, left, options.num_orientations,
                 options.cells_per_block, options.cells_per_image_h,
                 options.cells_per_image_w,
                 &rect);

  cv::rectangle(left, rect, cv::Scalar(255, 253, 34), 1);
  cv::imwrite("left.png", left);
  cv::imwrite("patch.png", patch);

  return 0;
}