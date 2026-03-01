cmake_minimum_required(VERSION 3.16)
project(tracker_solver)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Find catkin macros and libraries
# [ROS 1] -----------------------------------------------------------
# find_package(catkin REQUIRED COMPONENTS
#   auto_aim_common
#   roscpp
#   std_msgs
# )
# -------------------------------------------------------------------

# [ROS 2] -----------------------------------------------------------
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(auto_aim_common REQUIRED)
# -------------------------------------------------------------------

## System dependencies
find_package(fmt 6.1.2 CONFIG REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

## Uncomment this if the package has a setup.py
# catkin_python_setup()

################################################
## Declare ROS messages, services and actions ##
################################################

# [ROS 1 Message generation commented out...]
# add_message_files(...)
# generate_messages(...)

###################################
## catkin specific configuration ##
###################################
# [ROS 1]
# catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES tracker_solver
#    CATKIN_DEPENDS roscpp std_msgs auto_aim_common
# )

###########
## Build ##
###########

## Declare a C++ library
# Find all source files
file(GLOB_RECURSE CAR_TRACKER_SRC_FILES src/*.cpp)

add_library(${PROJECT_NAME} SHARED
  ${CAR_TRACKER_SRC_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${OpenCV_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS}
)

# Link system libraries
target_link_libraries(${PROJECT_NAME}
  ${OpenCV_LIBS}
  Eigen3::Eigen
  fmt::fmt
)

# Link ROS 2 dependencies
ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  std_msgs
  auto_aim_common
)

## Declare a C++ executable
add_executable(${PROJECT_NAME}_node 
  car_tracker_solver_node.cpp
)

# Link to the library we just built
target_link_libraries(${PROJECT_NAME}_node
  ${PROJECT_NAME}
)

# Link ROS 2 dependencies for the node
ament_target_dependencies(${PROJECT_NAME}_node
  rclcpp
  std_msgs
  auto_aim_common
)

#############
## Install ##
#############

# ROS 2 Install Rules
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

#############
## Testing ##
#############

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# [CRITICAL FIX HERE] Export dependencies
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(
  rclcpp
  std_msgs
  auto_aim_common
  OpenCV   # <--- FIXED: Changed from 'opencv' to 'OpenCV' (Must match find_package name)
  Eigen3   # <--- Changed to 'Eigen3' for consistency
)

ament_package()