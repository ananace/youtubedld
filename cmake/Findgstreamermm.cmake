# Find gstreamermm
#
# GSTREAMERMM_INCLUDE_DIR
# GSTREAMERMM_LIBRARIES
# GSTREAMERMM_FOUND

include(FindPkgConfig)
pkg_search_module(GSTREAMERMM gstreamermm-1.0 gstreamermm)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gstreamermm DEFAULT_MSG GSTREAMERMM_LIBRARIES GSTREAMERMM_INCLUDE_DIRS)

mark_as_advanced(GSTREAMERMM_INCLUDE_DIRS GSTREAMERMM_LIBRARIES)

