# HF computer vision library
cmake_minimum_required(VERSION 2.8)

# include guard
if ( NOT CVBASE_INCLUDED ) 

set(CVBASE_INCLUDED "CVBASE_INCLUDED")

include_directories(${CMAKE_CURRENT_LIST_DIR} ${CFG_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/../scripts/cmake/opencv.cmake)

set(__CVBASE_P ${CMAKE_CURRENT_LIST_DIR})

# Core files
set(CVBASE_SOURCES
	${__CVBASE_P}/cv/base/utils.cpp
	)
set(CVBASE_HEADERS
	${__CVBASE_P}/cv/base/utils.h
	)
  
add_library(cvbase ${CVBASE_LIBTYPE} ${CVBASE_SOURCES} ${CVBASE_HEADERS})
set_source_files_properties( ${CVBASE_SOURCES} PROPERTIES COMPILE_DEFINITIONS "${CVBASE_SYMBOLTYPE}")

set(CVBASE_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR})
if ( WIN32 )
  set(CVBASE_LIBS cvbase shell32)
else ()
  set(CVBASE_LIBS cvbase)
endif ()

# build cfg parser
set(CFG_SOURCES
	${CFG_DIR}/cfg_parser.h
	${CFG_DIR}/cfg_parser.cpp
	)

add_library(cfg_lib ${CFG_SOURCES})
set(CV_BASE_LIBS ${CV_BASE_LIBS} cfg_lib)
  
set(CVBASE_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR})
  
endif ( NOT CVBASE_INCLUDED ) # end of include guard
