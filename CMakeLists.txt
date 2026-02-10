cmake_minimum_required(VERSION 3.8)


project(mc_common)


add_library(mc_common STATIC
  src/mc_common.cpp
)


target_include_directories(mc_common PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

install(
  TARGETS  mc_common
  DESTINATION lib/${PROJECT_NAME} 
)

install(DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)