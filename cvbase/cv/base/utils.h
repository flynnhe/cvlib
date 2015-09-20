#ifndef INCLUDED_IMAGE_UTILS_H
#define INCLUDED_IMAGE_UTILS_H

#include <opencv2/core/core.hpp>

namespace cv {
  namespace base {
    template <typename T> T PI() { return (T) 3.141592653589793238462643383279502884e+00; }
    template <typename T> T INV_PI() { return (T) 0.31830988618379067153776752674503; }

    template <typename T> T DEG_TO_RAD(const T& x) { return (T)(x*PI<T>() / 180.0); }
    template <typename T> T RAD_TO_DEG(const T& x) { return (x*180.0 / PI<T>()); }

    void padImage(const cv::Mat& img, cv::Mat* result, int x, int y);

    void computeGradients(const cv::Mat& image, cv::Mat_<float>* x_grad,
                          cv::Mat_<float>* y_grad, cv::Mat_<float>* thetas,
                          cv::Mat_<float>* mags);

    void computeHOGDescriptor(const cv::Mat_<float>& thetas, const cv::Mat_<float> mags,
                              int cell_size, int block_size, int num_orientations,
                              std::vector<float>* descriptor);
  } // end namespace base
} // end namespace cv

#endif
