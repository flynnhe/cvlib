#include <cv/base/io.h>
#include <cv/base/utils.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <iostream>

#include <emscripten/bind.h>

using namespace cv::base;

class FaceDetector {
  cv::CascadeClassifier m_detector;

public:
  FaceDetector()
  {}

  bool configure(const std::string& path) {
    std::string cascade_file(path);
    if (!m_detector.load(cascade_file)) {
      std::cerr << "Error loading face classifier" << std::endl;
      return false;
    }
    std::cerr << "Successfully opened classifier" << std::endl;
    return true;
  }

  bool detect(uintptr_t dst, int w, int h) {
    char* ptr = reinterpret_cast<char*>(dst);
    cv::Mat image(h, w, CV_8UC4);
    image.data = (unsigned char*)ptr;

    std::vector<cv::Rect> faces;
    cv::Mat frame_gray;

    cv::cvtColor( image, frame_gray, CV_BGR2GRAY );
    cv::equalizeHist( frame_gray, frame_gray );

    m_detector.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, cv::Size(30, 30) );

    for( size_t i = 0; i < faces.size(); i++ )
    {
      cv::Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
      cv::ellipse( image, center, cv::Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, cv::Scalar( 255, 0, 255 ), 4, 8, 0 );
    }

    return true;
  }

};

// Binding code
EMSCRIPTEN_BINDINGS(jstracker_example) {
  emscripten::class_<FaceDetector>("FaceDetector")
    .constructor()
    .function("configure", &FaceDetector::configure)
    .function("detect", &FaceDetector::detect, emscripten::allow_raw_pointers())
    ;
}
