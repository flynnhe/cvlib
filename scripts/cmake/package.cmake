# this cmake file provides a means to package a given target into a zip file
# input: TARGETNAME  -- the executable and target of the package
#        PACKAGENAME -- name of the resulting zip file (excluding extension)
#        DESCRIPTION -- a small description of the package

function(package_target TARGETNAME PACKAGENAME DESCRIPTION)
message(STATUS "packaging target: ${TARGETNAME}")

get_property(TGT_LOCATION_RLS TARGET ${TARGETNAME} PROPERTY LOCATION_RELEASE)
get_property(TGT_LOCATION_DBG TARGET ${TARGETNAME} PROPERTY LOCATION_DEBUG)
get_filename_component(RLS_EXT ${TGT_LOCATION_RLS} EXT)
get_filename_component(DBG_EXT ${TGT_LOCATION_DBG} EXT)
message(STATUS "release extension of target: ${RLS_EXT}")


if ( RLS_EXT STREQUAL ".pyd" ) 
  set(LIBDIR bin)
else () 
  set(LIBDIR lib)
endif () 

install(TARGETS ${TARGETNAME} CONFIGURATIONS Release
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION ${LIBDIR}
  LIBRARY DESTINATION ${LIBDIR})

if ( DBG_EXT STREQUAL ".pyd" ) 
  set(LIBDIR_D bin_debug)
else () 
  set(LIBDIR_D lib_debug)
endif () 
install(TARGETS ${TARGETNAME} CONFIGURATIONS Debug
  RUNTIME DESTINATION bin_debug
  ARCHIVE DESTINATION ${LIBDIR_D}
  LIBRARY DESTINATION ${LIBDIR_D})

#this is necessary for packaging only
find_package(Subversion)
if ( Subversion_FOUND ) 
	# get revision number into HT_WC_REVISION
	Subversion_WC_INFO(.. HT)
else ()
	set(HT_WC_REVISION "svn-unknown")
endif ()

# add MS runtime dlls to package
include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "RealD Inc") 
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH ${HT_WC_REVISION})

set(CPACK_PACKAGE_NAME ${PACKAGENAME})
set(CPACK_PACKAGE_EXECUTABLES ${TARGETNAME})
include(CPack)
endfunction()
