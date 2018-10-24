# CMake module to search for the C library libyaml
#(library for parsing YAML files)
#
# If it's found it sets LIBYAML_FOUND to TRUE
# and following variables are set:
#    LIBYAML_INCLUDE_DIR
#    LIBYAML_LIBRARY

find_path(LIBYAML_INCLUDE_DIR NAMES yaml.h)
find_library(LIBYAML_LIBRARY NAMES yaml libyaml)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibYaml DEFAULT_MSG LIBYAML_LIBRARY LIBYAML_INCLUDE_DIR)
mark_as_advanced(LIBYAML_INCLUDE_DIR LIBYAML_LIBRARY)
