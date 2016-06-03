project(opencvjs)
cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_LIST_DIR}/../cvbase/cvbase.cmake)

if ( WIN32 )
  # define minimum windows version and disable boost's internal autolinking (use cmake's instead)
  add_definitions(-D_WIN32_WINNT=0x0501 -DBOOST_ALL_NO_LIB -D_CRT_SECURE_NO_WARNINGS /openmp)
else ()
  add_definitions(-std=c++11 -fopenmp -fpermissive) # permissive for taking address of
  if ( EMSCRIPTEN )
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "--bind -O3 -s --preload-file ../data")
    message("${CMAKE_EXE_LINKER_FLAGS}")
  endif()
endif ()

if ( EMSCRIPTEN )
  add_executable( opencvjs_facedetector examples/opencvjs_facedetector.cpp )
  target_link_libraries( opencvjs_facedetector cfg_lib cvbase ${OpenCV_LIBS})
endif()
