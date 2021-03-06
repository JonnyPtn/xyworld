cmake_minimum_required(VERSION 3.1)

# Rename this variable to change the project name
SET(PROJECT_NAME xyworld)

# Set up the project
project(${PROJECT_NAME})

# Set the path to our find modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Some default variables which the user may change
SET(CMAKE_BUILD_TYPE        Debug CACHE STRING  "Choose the type of build (Debug or Release)")

# We're using c++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Only works with SFML version 2.4 and above
SET(SFML_MIN_VERSION 2.4)
find_package(SFML ${SFML_MIN_VERSION} REQUIRED graphics window audio system network)

# Find xyginext
find_package(XYGINEXT REQUIRED)

# Additional include directories
include_directories(
  ${XYXT_INCLUDE_DIR}
  ${SFML_INCLUDE_DIR} 
  ${CMAKE_SOURCE_DIR}/include)

# Project source files
add_subdirectory(include)
add_subdirectory(src)

# Create the actual executable (PROJECT_SRC variable is set inside previous steps)
add_executable(${PROJECT_NAME} ${PROJECT_SRC})

# Linker settings
target_link_libraries(${PROJECT_NAME}
  ${SFML_LIBRARIES}
  ${SFML_DEPENDENCIES}
  ${XYXT_LIBRARIES})

# Install executable
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION .)

# Install assets folder
install(DIRECTORY assets
  DESTINATION .)
