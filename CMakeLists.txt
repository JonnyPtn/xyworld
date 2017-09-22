#rename your project here
project(xy_world)
SET(PROJECT_NAME xy_world)
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")


SET(SFML_MIN_VERSION "2.4")

find_package(SFML 2 REQUIRED graphics window audio system network)

find_package(XYGINEXT REQUIRED)

include_directories(
  ${XYXT_INCLUDE_DIR}
  ${SFML_INCLUDE_DIR} 
  ${CMAKE_SOURCE_DIR}/include)


SET(PROJECT_DIR ${CMAKE_SOURCE_DIR}/src)
include(${PROJECT_DIR}/CMakeLists.txt)
include(${CMAKE_SOURCE_DIR}/include/CMakeLists.txt)

add_executable(${PROJECT_NAME} ${PROJECT_SRC})

target_link_libraries(${PROJECT_NAME}
  ${SFML_LIBRARIES}
  ${SFML_DEPENDENCIES}
  ${XYXT_LIBRARIES})

#install executable
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION .)

#install game data
install(DIRECTORY assets
  DESTINATION .)

#install the OpenAL DLL
if(WIN32)
  install(FILES ${SFML_INCLUDE_DIR}/../bin/openal32.dll
    DESTINATION .)
endif()
