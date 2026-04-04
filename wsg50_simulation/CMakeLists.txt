cmake_minimum_required(VERSION 3.8)
project(wsg50_simulation)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(wsg50_common REQUIRED)

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

# --- Executables ---
add_executable(wsg_50_sim_keyboard_teleop src/wsg_50_keyboard_teleop.cpp)
ament_target_dependencies(wsg_50_sim_keyboard_teleop rclcpp std_msgs std_srvs wsg50_common)

add_executable(wsg_50_sim_driver src/wsg_50_sim_driver.cpp)
ament_target_dependencies(wsg_50_sim_driver rclcpp std_msgs std_srvs wsg50_common)

# --- Install executables ---
install(TARGETS
  wsg_50_sim_keyboard_teleop
  wsg_50_sim_driver
  DESTINATION lib/${PROJECT_NAME}
)

# --- Install other resources ---
install(
  DIRECTORY meshes urdf launch controllers
  DESTINATION share/${PROJECT_NAME}
)

ament_package()