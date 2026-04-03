cmake_minimum_required(VERSION 3.8)
project(wsg50_common)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Cmd.msg"
  "msg/Status.msg"
  "srv/Conf.srv"
  "srv/Incr.srv"
  "srv/Move.srv"
)

ament_package()