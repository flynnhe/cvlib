#include "utils.h"

// read in the HOG settings
bool cv::base::HOGSettings::configure(ConfigParser& cfg) {
  bool success = true;
  cells_per_block = cfg.get("cells_per_block", 2, "HOG");
  cells_per_image_w = cfg.get("cells_per_image_w", 8, "HOG");
  cells_per_image_h = cfg.get("cells_per_image_h", 8, "HOG");
  num_orientations = cfg.get("num_orientations", 9, "HOG");
  return success;
}

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

// get the L2 distance between two vectors
float cv::base::getL2Distance(const std::vector<float>& vec1, const std::vector<float>& vec2)
{
  assert(vec1.size() == vec2.size());
  float dist = 0.0f;
  for (int i = 0; i < (int)vec1.size(); ++i) {
    dist += (vec2[i] - vec1[i])*(vec2[i] - vec1[i]);
  }
  return dist;
}

// normalize a vector to have unit length
void cv::base::normalizeVectorL2(const std::vector<float> vec, std::vector<float>* res,
                                 bool l2_hys, float l2_hys_threshold)
{
  float mag = 0.0f;
  float eps = 1e-9f;
  res->clear();
  for (int i = 0; i < (int)vec.size(); ++i) {
    mag += vec[i] * vec[i];
  }
  mag = sqrt(mag + eps);
  for (int i = 0; i < (int)vec.size(); ++i) {
    if (l2_hys) {
      res->push_back(std::min(l2_hys_threshold, vec[i] / mag));
    }
    else {
      res->push_back(vec[i] / mag);
    }
  }
  if (l2_hys) {
    normalizeVectorL2(*res, res, false, 0.0f);
  }
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
  assert(block_size % 2 == 0);

  int w = thetas.cols;
  int h = thetas.rows;
  int step_x = cell_size;
  int step_y = cell_size;
  float PI = cv::base::PI<float>();
  float bin_size = PI / num_orientations;
  int cells_per_block = (block_size / cell_size) * (block_size / cell_size);

  int desc_size = num_orientations * cells_per_block * ((w / cell_size) - 1) * ((h / cell_size) - 1);
  descriptor->clear();
  descriptor->reserve(desc_size);

  // compute the per-cell histograms
  std::vector< std::vector<float> > hists;
  for (int y = 0; y + step_y <= h; y += step_y) {
    for (int x = 0; x + step_x <= w; x += step_x) {
      // compute the histogram for the (cell_size x cell_size) cell starting at x, y
      std::vector<float> hist(num_orientations, 0.0f);
      for (int offset_y = 0; offset_y < cell_size; ++offset_y) {
        for (int offset_x = 0; offset_x < cell_size; ++offset_x) {
          float rad = fmod(thetas(y + offset_y, x + offset_x), PI); // unsigned orientation
          if (rad < 0.0) { rad += PI; }
          int idx = (int)(rad / bin_size); // left idx
          float nearest_center = (idx + 0.5f) * bin_size;
          float portion;
          int idx_lo, idx_hi;
          // do linear interpolation between bins
          if (rad >= nearest_center) {
            portion = (rad - nearest_center) / bin_size;
            idx_lo = idx;
            idx_hi = (idx_lo + 1) % num_orientations;
          }
          else {
            portion = (nearest_center - rad) / bin_size;
            idx_lo = idx - 1;
            if (idx_lo < 0) idx_lo += num_orientations;
            idx_hi = (idx_lo + 1) % num_orientations;
          }
          float mag = mags(y + offset_y, x + offset_x);
          hist[idx_lo] += (1 - portion) * mag;
          hist[idx_hi] += portion * mag;
        }
      }
      hists.push_back(hist);
    }
  }

  // do block normalization
  int total_x_cells = w / cell_size;
  int total_y_cells = h / cell_size;
  int num_cells_per_block = block_size / cell_size; // number of cells in block (in both dirs)
  for (int y = 0; y + num_cells_per_block <= total_y_cells; ++y) {
    for (int x = 0; x + num_cells_per_block <= total_x_cells; ++x) {
      std::vector<float> hist;
      for (int c1 = 0; c1 < num_cells_per_block; ++c1) {
        int y_idx = (y + c1) * total_x_cells;
        for (int c2 = 0; c2 < num_cells_per_block; ++c2) {
          int x_idx = x + c2;
          hist.insert(hist.end(), hists[x_idx + y_idx].begin(),
                                  hists[x_idx + y_idx].end());
        }
      }
      normalizeVectorL2(hist, &hist, true, 0.2f);
      descriptor->insert(descriptor->end(), hist.begin(), hist.end());
    }
  }
}

// Given a HOG descriptor of an object to be found,
// finds the closest HOG match in the whole image
void cv::base::findClosestHOG(const cv::Mat& object,
                              const cv::Mat& image,
                              const HOGSettings& settings,
                              cv::Rect* rect)
{
  // extract the gradients of object
  cv::Mat_<float> x_grad_obj, y_grad_obj, thetas_obj, mags_obj;
  computeGradients(object, &x_grad_obj, &y_grad_obj, &thetas_obj, &mags_obj);

  int cell_size = object.cols / 8;
  int block_size = 2 * cell_size;

  // compute the object HOG descriptor
  std::vector<float> descriptor_obj;
  computeHOGDescriptor(thetas_obj, mags_obj,
    cell_size, block_size, settings.num_orientations,
    &descriptor_obj);

  // compute gradients for the whole image
  cv::Mat_<float> x_grad_im, y_grad_im, thetas_im, mags_im;
  computeGradients(image, &x_grad_im, &y_grad_im, &thetas_im, &mags_im);
  
  float mindist = FLT_MAX;
  int minx = 0, miny = 0;
  int minscale = INT_MAX;

#pragma omp parallel for
  for (int scale = 16; scale <= 256; scale += 8) {
    std::vector<float> descriptor;
    int cell_size = scale / settings.cells_per_image_w;
    int block_size = cell_size * settings.cells_per_block;
    for (int y = 0; y + scale < image.rows; y += 2) {
      for (int x = 0; x + scale < image.cols; x += 2) {
        cv::Rect r(x, y, scale, scale);
        computeHOGDescriptor(thetas_im(r), mags_im(r), cell_size, block_size, settings.num_orientations, &descriptor);
        float dist = getL2Distance(descriptor, descriptor_obj);
        if (dist < mindist) {
          mindist = dist;
          minx = x;
          miny = y;
          minscale = scale;
        }
      }
    }
  }
  *rect = cv::Rect(minx, miny, minscale, minscale);
}