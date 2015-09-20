#include <cv/base/cfg_parser/cfg_parser.h>
#include <cv/base/utils.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

using namespace cv::base;

struct Options {
  int cell_size;
  int block_size;
  int num_orientations;
};

static void usage(const char* progname) {
  std::cout << "Usage: " << progname << " <cfg>" << std::endl;
}

static bool configure(ConfigParser& cfg, Options* options) {
  // read HOG settings
  bool success = true;
  options->cell_size = cfg.get("cell_size", 8, "HOG");
  options->block_size = cfg.get("block_size", 16, "HOG");
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

  // TEST
  cv::resize(left, left, cv::Size(128, 128));

  // extract the gradients
  cv::Mat_<float> x_grad, y_grad, thetas, mags;
  computeGradients(left, &x_grad, &y_grad, &thetas, &mags);

  // compute the orientation histograms
  std::vector<float> descriptor;
  computeHOGDescriptor(thetas, mags,
                      options.cell_size, options.block_size, options.num_orientations,
                      &descriptor);

  return 0;
}