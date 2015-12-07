// Program to generate training data for Mechanical Turk to show
// salient key points on infrared image and request user to annotate
// correspondences on the visible light image

#include <cv/base/io.h>
#include <cv/base/utils.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/nonfree/nonfree.hpp>

#include <chrono>
#include <iostream>

using namespace cv::base;

struct Options {
  std::string ir_dir;
  std::string vis_dir;
  std::string result_dir;
  int n_keypoints;
};

struct KeyPointComparator {
  const bool operator()(const cv::KeyPoint& k1, const cv::KeyPoint& k2) {
    return k1.response > k2.response;
  }
};

static void usage(const char* progname) {
  std::cout << "Usage: " << progname << " <cfg>" << std::endl;
}

static bool configure(ConfigParser& cfg, Options* options)
{
  bool success = true;
  std::string default("");
  int default_keypoints = 20;
  options->ir_dir = cfg.get("ir_dir", default, "Paths");
  success &= options->ir_dir != default;
  options->vis_dir = cfg.get("vis_dir", default, "Paths");
  success &= options->vis_dir != default;
  options->result_dir = cfg.get("result_dir", default, "Paths");
  success &= options->result_dir != default;
  options->n_keypoints = cfg.get("n_keypoints", default_keypoints, "Keypoints");
  return success;
}

static bool closeTo(cv::Point2f pt, const std::vector<cv::Point2f>& points, int proximity)
{
  for (size_t i = 0; i < points.size(); ++i) {
    cv::Point2f point(points[i]);
    float dist = std::sqrt((point.x - pt.x) * (point.x - pt.x) + (point.y - pt.y) * (point.y - pt.y));
    if (dist <= proximity) {
      return true;
    }
  }
  return false;
}

static void drawSalientKeyPoints(const cv::Mat& image, const int n_keypoints, cv::Mat* result)
{
  int minHessian = 0; // this needs to be low because ir image can have little detail
  cv::SurfFeatureDetector detector(minHessian);
  std::vector<cv::KeyPoint> keypoints;

  detector.detect(image, keypoints);

  // sort the key points by their response and draw the top n_keypoints
  std::sort(keypoints.begin(), keypoints.end(), KeyPointComparator());

  std::vector<cv::Point2f> saved_keypoints;
  int k_idx = 1;
  for (int i = 0; i < std::min(n_keypoints, (int)keypoints.size()); ++i) {
    cv::Point2f pt(keypoints[i].pt);
    if (closeTo(pt, saved_keypoints, 40)) {
      continue;
    }
    saved_keypoints.push_back(pt);
    // draw a circle and put a number beside it
    cv::circle(*result, (cv::Point)pt, 3, cv::Scalar(10, 10, 10));
    cv::putText(*result, std::to_string(k_idx), pt + cv::Point2f(10, 0), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0));
    k_idx++;
  }
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

  char fname[256];
  sprintf(fname, "%s\\offsets.txt", options.result_dir.c_str());
  FILE* file = fopen(fname, "w");

  // load up the images in the directory
  std::vector<std::string> ir_images, vis_images;
  readFilesInDirectory(options.ir_dir.c_str(), &ir_images, "png");
  for (size_t i = 0; i < ir_images.size(); ++i)
  {
    char eo_filename[256];
    char ir_filename[256];
    sprintf(eo_filename, "%s\\%s", options.vis_dir.c_str(), ir_images[i].c_str());
    sprintf(ir_filename, "%s\\%s", options.ir_dir.c_str(), ir_images[i].c_str());

    cv::Mat ir_img = cv::imread(ir_filename, -1);
    cv::Mat vis_img = cv::imread(eo_filename, -1);
    cv::Mat salient_ir(ir_img.clone());
    if (!vis_img.data || !ir_img.data) {
      std::cout << "Error loading " << ir_images[i] << std::endl;
      continue;
    }

    // draw salient key points on infrared image
    cv::cvtColor(ir_img, ir_img, CV_BGR2GRAY);
    cv::normalize(ir_img, ir_img, 0, 255, cv::NORM_MINMAX);
    drawSalientKeyPoints(ir_img, options.n_keypoints, &salient_ir);

    // concatenate ir to vis and save
    cv::Mat result;
    concatImages(salient_ir, vis_img, &result);

    cv::imwrite(options.result_dir + "\\" + ir_images[i].c_str(), result);
    fprintf(file, "%d,%d\n", i, ir_img.cols);
  }

  fclose(file);
  return 0;
}