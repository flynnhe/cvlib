project(multimodal)
cmake_minimum_required(VERSION 2.8)

if ( WIN32 )
  # define minimum windows version and disable boost's internal autolinking (use cmake's instead)
  add_definitions(-D_WIN32_WINNT=0x0501 -DBOOST_ALL_NO_LIB -D_CRT_SECURE_NO_WARNINGS /openmp)
else ()
  add_definitions(-std=c++11 -fopenmp -fpermissive) # permissive for taking address of
    #temporary -- needs to be removed
endif ()

include(${CMAKE_CURRENT_LIST_DIR}/../cvbase/cvbase.cmake)

add_executable( hog examples/hog.cpp )
add_executable (gen_training_data examples/gen_training_data )
target_link_libraries( hog cfg_lib cvbase ${OpenCV_LIBS})
target_link_libraries( gen_training_data cfg_lib cvbase ${OpenCV_LIBS})
