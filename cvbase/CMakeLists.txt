# HF computer vision library
project(cvbase)
cmake_minimum_required(VERSION 2.8)

set(OPENCV_REQUIRED_MODULES core imgproc highgui video flann calib3d features2d objdetect ml)

include(${CMAKE_CURRENT_LIST_DIR}/cvbase.cmake)

include_directories(${CVLIB_INCLUDE_DIRS})