cmake_minimum_required(VERSION 3.8)
project(robot_move)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(xacro REQUIRED)
find_package(robot_gripper REQUIRED)
find_package(moveit_robot_setup REQUIRED)

find_package(rclcpp REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)

add_executable(robot_move_execute src/robot_motion_source.cpp)

ament_target_dependencies(
  robot_move_execute
  rclcpp
  moveit_ros_planning_interface
  geometry_msgs
  tf2
  moveit_robot_setup
)

install(TARGETS robot_move_execute DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY worlds launch DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
