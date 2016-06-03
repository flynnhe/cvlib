# HF computer vision library

# Requires the following variables to be set:
#   CFG_DIR - path to cfg_parser of Clemens Wacha (http://www.wacha.ch/wiki/cfg_parser)

cmake_minimum_required(VERSION 2.8)


if ( NOT CVBASE_INCLUDED ) 

set(CVBASE_INCLUDED "CVBASE_INCLUDED")

set(OPENCV_REQUIRED_MODULES core imgproc highgui video flann calib3d features2d objdetect ml)
include_directories(${CMAKE_CURRENT_LIST_DIR} ${CFG_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/../scripts/cmake/opencv.cmake)

set(__CVBASE_P ${CMAKE_CURRENT_LIST_DIR})

# Core files
set(CVBASE_SOURCES
  ${__CVBASE_P}/cv/base/io.cpp
	${__CVBASE_P}/cv/base/utils.cpp
	)
set(CVBASE_HEADERS
  ${__CVBASE_P}/cv/base/io.h
	${__CVBASE_P}/cv/base/utils.h
	)

add_library(cvbase ${CVBASE_LIBTYPE} ${CVBASE_SOURCES} ${CVBASE_HEADERS})

# cfg parsing files
set(CFG_SOURCES
	${CMAKE_CURRENT_LIST_DIR}/cv/base/cfg_parser/cfg_parser.h
	${CMAKE_CURRENT_LIST_DIR}/cv/base/cfg_parser/cfg_parser.cpp
	)
  
add_library(cfg_lib ${CFG_SOURCES})

set_source_files_properties( ${CVBASE_SOURCES} PROPERTIES COMPILE_DEFINITIONS "${CVBASE_SYMBOLTYPE}")

set(CVBASE_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR})
if ( WIN32 )
  set(CVBASE_LIBS cvbase shell32)
else ()
  set(CVBASE_LIBS cvbase)
endif ()
  
set(CVBASE_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR})
  
endif ( NOT CVBASE_INCLUDED ) # end of include guard
