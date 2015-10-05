#ifndef INCLUDED_IMAGE_UTILS_H
#define INCLUDED_IMAGE_UTILS_H

#include <cv/base/cfg_parser/cfg_parser.h>
#include <opencv2/core/core.hpp>

namespace cv {
  namespace base {

    struct HOGSettings {
      int cells_per_block_w;
      int cells_per_block_h;
      int cells_per_image_w;
      int cells_per_image_h;
      int num_orientations;
      int min_scale;
      int max_scale;
      bool configure(ConfigParser& cfg);
    };

    template <typename T> T PI() { return (T) 3.141592653589793238462643383279502884e+00; }
    template <typename T> T INV_PI() { return (T) 0.31830988618379067153776752674503; }

    template <typename T> T DEG_TO_RAD(const T& x) { return (T)(x*PI<T>() / 180.0); }
    template <typename T> T RAD_TO_DEG(const T& x) { return (x*180.0 / PI<T>()); }

    void padImage(const cv::Mat& img, cv::Mat* result, int x, int y);

    float getL2Distance(const std::vector<float>& vec1, const std::vector<float>& vec2);

    void normalizeVectorL2(const std::vector<float> vec, std::vector<float>* res,
                           bool l2_hys = false, float l2_hys_threshold = 0.2f);

    void computeGradients(const cv::Mat& image, cv::Mat_<float>* x_grad,
                          cv::Mat_<float>* y_grad, cv::Mat_<float>* thetas,
                          cv::Mat_<float>* mags);

    void computeHOGDescriptor(const cv::Mat_<float>& thetas, const cv::Mat_<float> mags,
                              int cell_size_w, int cell_size_h, int block_w_in_cells, int block_h_in_cells, int num_orientations,
                              std::vector<float>* descriptor);

    void findClosestHOG(const cv::Mat& object, const cv::Mat& image,
                        const HOGSettings& settings,
                        cv::Rect* rect);

    void computeScaleFromH(const cv::Mat& H, int size, HOGSettings* settings);
  } // end namespace base
} // end namespace cv

#endif
