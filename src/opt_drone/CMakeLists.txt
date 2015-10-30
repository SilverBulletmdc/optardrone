cmake_minimum_required(VERSION 2.8.3)
project(opt_drone C CXX)
aux_source_directory(./src SRC_LIST)


find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  sensor_msgs
  cv_bridge
  image_transport
)


catkin_package()

find_package(OpenCV)
include_directories(
  ${OpenCV_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS}/opencv2 ${OpenCV_INCLUDE_DIRS}/opencv
)

include_directories(include ${catkin_INCLUDE_DIRS})
add_executable(optdrone ${SRC_LIST})
target_link_libraries(optdrone ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})
add_dependencies(optdrone optdrone_generate_messages_cpp)

