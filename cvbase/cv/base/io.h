#ifndef INCLUDED_IO_H
#define INCLUDED_IO_H

#include <opencv2/core/core.hpp>

#include <fstream>

namespace cv {
  namespace base {
    bool readHomography(const char* filename, cv::Mat* H);
  } // end namespace base
} // end namespace cv

#endif
