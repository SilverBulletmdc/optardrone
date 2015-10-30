cmake_minimum_required(VERSION 2.8.3)
project(droneTest)

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
  ${OpenCV_INCLUDE_DIRS}
)

include_directories(include ${catkin_INCLUDE_DIRS})
add_executable(droneTest src/droneTest.cpp src/droneTest.h src/drone.h)
target_link_libraries(droneTest ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})
add_dependencies(droneTest droneTest_generate_messages_cpp)
