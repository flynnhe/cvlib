# OpenCV cmake module.
# include this file from your project directory if you want to add OpenCV support to your project.
# specify -DOpenCV_DIR=.... path to Opencvconfig.cmake
# specify OPENCV_REQUIRED_MODULES to get a set of opencv modules. Otherwise, "core imgproc highgui video" is used.
#
# this file defines the following variables:
# OpenCV_LIBS - the full paths to the dependent libraries
# OpenCV_INCLUDE_DIRS - include directories, automatically added
# opencv_install_dlls(targetname) - a function taking a target name where binary files will be installed
#

message(STATUS "OpenCV: am in ${CMAKE_CURRENT_LIST_DIR}")
if ( NOT OPENCV_REQUIRED_MODULES ) 
  set(OPENCV_REQUIRED_MODULES core imgproc highgui video features2d objdetect)
endif ()
find_package( OpenCV REQUIRED ${OPENCV_REQUIRED_MODULES})
# use release dlls and libs when building RelWithDebInfo, otherwise code crashes as VS links to debug versions:
if ( OpenCV_LIBS ) 
  add_definitions(-DHAVE_OPENCV=1)
  list(GET OpenCV_LIBS 0 HEAD)
  message(STATUS "Head: ${HEAD}")
  if ( NOT ${HEAD} STREQUAL "debug" )  
    set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
  endif () 
endif ( ) 
include_directories(${OpenCV_INCLUDE_DIRS})
message(STATUS "OpenCV in: ${OpenCV_INCLUDE_DIRS} and ${OpenCV_LIBS} in ${OpenCV_LIB_DIR}")
set(OPENCV_VERSION_SHORT ${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH})

function(opencv_install_dlls TARGETNAME )
  message(STATUS "OpenCV DLLs: target: ${TARGETNAME}")

  # copy all dependencies: DLLs
  if ( WIN32 )
    list(GET OpenCV_LIB_DIR 1 OpenCV_FIRST_LIB_DIR)
    message("1")
    message(${OpenCV_LIB_DIR})
    message(${OpenCV_FIRST_LIB_DIR})
    message("2")
  foreach(OCV_MODULE ${OPENCV_REQUIRED_MODULES})
    file(GLOB OpenCV_DLLS       "${OpenCV_FIRST_LIB_DIR}/../bin/opencv_${OCV_MODULE}${OPENCV_VERSION_SHORT}.dll")
    file(GLOB OpenCV_DLLS_DEBUG "${OpenCV_FIRST_LIB_DIR}/../bin/opencv_${OCV_MODULE}${OPENCV_VERSION_SHORT}d.dll")
    message(STATUS "OpenCV DLLs: copying ${OpenCV_DLLS} in Release and ${OpenCV_DLLS_DEBUG} in Debug mode")
      # ${OPENCV_LIB_DIR}/../bin/opencv_highgui24?.dll ${OPENCV_LIB_DIR}/../bin/opencv_imgproc24?.dll")
    install_target_files(${TARGETNAME} "${OpenCV_DLLS}" "${OpenCV_DLLS_DEBUG}")
  endforeach()
  endif()
endfunction()

function(install_target_files TARGETNAME FILES FILES_DBG)
  foreach(CUR_FILE ${FILES})
    message(STATUS "Installing file: ${CUR_FILE}")
    install(FILES ${CUR_FILE} CONFIGURATIONS Release DESTINATION bin)
    add_custom_command(TARGET ${TARGETNAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CUR_FILE} $<TARGET_FILE_DIR:${TARGETNAME}>
    )
  endforeach()
  foreach(CUR_FILE ${FILES_DBG})
    message(STATUS "Installing debug file: ${CUR_FILE}")
    install(FILES ${CUR_FILE} CONFIGURATIONS Debug DESTINATION bin_debug)
    add_custom_command(TARGET ${TARGETNAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CUR_FILE} $<TARGET_FILE_DIR:${TARGETNAME}>
    )
  endforeach()
endfunction()
