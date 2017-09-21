include(FindPackageHandleStandardArgs)

set(FIND_XYXT_PATHS
    ${XYXT_ROOT})

# Search for the header file
find_path(XYXT_INCLUDE_DIR xyginext/Config.hpp PATH_SUFFIXES include HINTS ${FIND_XYXT_PATHS})

# Search for the appropriate library
IF(CMAKE_BUILD_TYPE MATCHES Debug)
    find_library(XYXT_LIBRARIES NAMES xyginext-d PATH_SUFFIXES lib HINTS ${FIND_XYXT_PATHS})
else()
    find_library(XYXT_LIBRARIES NAMES xyginext PATH_SUFFIXES lib HINTS ${FIND_XYXT_PATHS})
endif()

# Did we find everything we need?
FIND_PACKAGE_HANDLE_STANDARD_ARGS(xyginext DEFAULT_MSG XYXT_LIBRARIES XYXT_INCLUDE_DIR) 
