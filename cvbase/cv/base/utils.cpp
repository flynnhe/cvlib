#include "utils.h"

// pad an image by copying the pixels on the border
void cv::base::padImage(const cv::Mat& img, cv::Mat* result, int x, int y)
{
  cv::Mat temp(img.rows + 2 * y, img.cols + 2 * x, img.type());
  img.copyTo(temp(cv::Rect(x, y, img.cols, img.rows)));
  int w = img.cols;
  int h = img.rows;

  // pad the top and bottom parts of image
  for (int j = 0; j < y; ++j) {
    const unsigned char* src_ptr_t = (const unsigned char*)img.ptr(0);
    const unsigned char* src_ptr_b = (const unsigned char*)img.ptr(h - 1);
    unsigned char* row_ptr_t = (unsigned char*)temp.ptr(j) + x;
    unsigned char* row_ptr_b = (unsigned char*)temp.ptr(j + h + y) + x;
    for (int i = 0; i < w; ++i) {
      *row_ptr_t++ = *src_ptr_t++;
      *row_ptr_b++ = *src_ptr_b++;
    }
  }
  // pad the left and right parts of image
  for (int j = 0; j < h; ++j) {
    const unsigned char* src_ptr_l = (const unsigned char*)img.ptr(j);
    const unsigned char* src_ptr_r = (const unsigned char*)img.ptr(j) + w - 1;
    unsigned char* row_ptr_l = (unsigned char*)temp.ptr(j + y);
    unsigned char* row_ptr_r = (unsigned char*)temp.ptr(j + y) + w + x;
    for (int i = 0; i < x; ++i) {
      *row_ptr_l++ = *src_ptr_l;
      *row_ptr_r++ = *src_ptr_r;
    }
  }
  // fill in the four corners
  const unsigned char tl = img.at<uchar>(0, 0);
  const unsigned char tr = img.at<uchar>(0, w - 1);
  const unsigned char bl = img.at<uchar>(h - 1, 0);
  const unsigned char br = img.at<uchar>(h - 1, w - 1);
  for (int j = 0; j < y; ++j) {
    unsigned char* row_ptr_tl = (unsigned char*)temp.ptr(j);
    unsigned char* row_ptr_tr = (unsigned char*)temp.ptr(j) + w + x;
    unsigned char* row_ptr_bl = (unsigned char*)temp.ptr(j + h + y);
    unsigned char* row_ptr_br = (unsigned char*)temp.ptr(j + h + y) + w + x;
    for (int i = 0; i < x; ++i) {
      *row_ptr_tl++ = tl;
      *row_ptr_tr++ = tr;
      *row_ptr_bl++ = bl;
      *row_ptr_br++ = br;
    }
  }
  *result = temp.clone();
}

// compute the gradients orientations and magnitudes for an image
void cv::base::computeGradients(const cv::Mat& image, cv::Mat_<float>* x_grad,
                      cv::Mat_<float>* y_grad, cv::Mat_<float>* thetas,
                      cv::Mat_<float>* mags)
{
  int fx[] = { -1, 0, 1 };
  int fy[] = { -1, 0, 1 };
  cv::Mat local_image;
  padImage(image, &local_image, 1, 1);
  *x_grad = cv::Mat(image.rows, image.cols, CV_32FC1);
  *y_grad = cv::Mat(image.rows, image.cols, CV_32FC1);
  *thetas = cv::Mat(image.rows, image.cols, CV_32FC1);
  *mags = cv::Mat(image.rows, image.cols, CV_32FC1);
  for (int y = 0; y < x_grad->rows; ++y) {
    const uchar* src_ptr = local_image.ptr<uchar>(y);
    float* dst_ptr_x = x_grad->ptr<float>(y);
    float* dst_ptr_y = y_grad->ptr<float>(y);
    float* dst_ptr_t = thetas->ptr<float>(y);
    float* dst_ptr_m = mags->ptr<float>(y);
    for (int x = 0; x < x_grad->cols; ++x) {
      *dst_ptr_x = 0.5f * (fx[0] * (*(src_ptr)) + fx[1] * (*(src_ptr + 1)) + fx[2] * (*(src_ptr + 2)));
      *dst_ptr_y = 0.5f * (fy[0] * (*(src_ptr)) + fy[1] * (*(src_ptr + local_image.cols)) + fy[2] * (*(src_ptr + 2 * local_image.cols)));
      *dst_ptr_t++ = atan2(*(dst_ptr_y), *(dst_ptr_x));
      *dst_ptr_m++ = sqrt((*(dst_ptr_x))*(*(dst_ptr_x)) + (*(dst_ptr_y))*(*(dst_ptr_y)));
      dst_ptr_x++;
      dst_ptr_y++;
      src_ptr++;
    }
  }
}

// compute the HOG descriptor for a given image
void cv::base::computeHOGDescriptor(const cv::Mat_<float>& thetas, const cv::Mat_<float> mags,
                                    int cell_size, int block_size, int num_orientations,
                                    std::vector<float>* descriptor)
{
  assert(thetas.cols % cell_size == 0);
  assert(thetas.rows % cell_size == 0);
  assert(block_size > cell_size);

  int w = thetas.cols;
  int h = thetas.rows;
  int step_x = cell_size;
  int step_y = cell_size;
  int bin_size = (int)(180.0f / num_orientations);

  for (int y = 0; y + step_y < h; ++y) {
    for (int x = 0; x + step_x < w; ++x) {
      // compute the histogram for the (cell_size x cell_size) cell starting at x, y
      std::vector<float> hist(num_orientations, 0.0f);
      for (int offset_y = 0; offset_y < cell_size; ++offset_y) {
        for (int offset_x = 0; offset_x < cell_size; ++offset_x) {
          float mag = mags(y + offset_y, x + offset_x);
          int deg = (int)cv::base::RAD_TO_DEG<double>(thetas(y + offset_y, x + offset_x));
          if (deg > 180) {
            deg -= 180;
          }
          if (deg < 0) {
            deg += 180;
          }
          deg %= 180;
          int excess = (int)(deg % bin_size);
          int nearest_orientation = deg - excess;
          float portion = 1.0f * excess / bin_size; // put this portion of magnitude into lower bin and put remainder into next bin
          int idx = (nearest_orientation / bin_size);
          if (idx < 0) idx = num_orientations + idx;
          hist[idx] += portion * mag;
          hist[(idx+1)%num_orientations] += (1 - portion) * mag;
        }
      }
    }
  }
}