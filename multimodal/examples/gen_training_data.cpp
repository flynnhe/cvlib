#include <cv/base/io.h>
#include <cv/base/utils.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <iostream>

using namespace cv::base;

struct Options {
  std::string hpath;
  std::string ir_dir;
  std::string vis_dir;
};

static void usage(const char* progname) {
  std::cout << "Usage: " << progname << " <cfg>" << std::endl;
}

static bool configure(ConfigParser& cfg, Options* options)
{
  bool success = true;
  std::string default("");
  options->hpath = cfg.get("hpath", default, "Paths");
  success &= options->hpath != default;
  options->ir_dir = cfg.get("ir_dir", default, "Paths");
  success &= options->ir_dir != default;
  options->vis_dir = cfg.get("vis_dir", default, "Paths");
  success &= options->vis_dir != default;
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
  if (!success) {
    std::cout << "Error reading config" << std::endl;
    return 1;
  }

  // load up the images in the directory
  std::vector<std::string> ir_images, vis_images;
  readFilesInDirectory(options.ir_dir.c_str(), &ir_images, "png");
  for (size_t i = 0; i < ir_images.size(); ++i)
  {
    char filename[256];
    char* test = "Argaerg";
    sprintf(filename, "%s\\%s", options.vis_dir.c_str(), ir_images[i].c_str());

    cv::Mat vis_img = cv::imread(filename, -1);
    if (!vis_img.data) {
      std::cout << "No matching vis image for ir image " << ir_images[i] << std::endl;
      continue;
    }
  }

  return 0;
}