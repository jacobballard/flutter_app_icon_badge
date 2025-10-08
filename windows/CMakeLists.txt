cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "flutter_app_icon_badge")
project(${PROJECT_NAME} LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "flutter_app_icon_badge_plugin")

add_library(${PLUGIN_NAME} SHARED
  "flutter_app_icon_badge_plugin.cpp"
)
apply_standard_settings(${PLUGIN_NAME})
set_target_properties(${PLUGIN_NAME} PROPERTIES
  CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL WINRT_LEAN_AND_MEAN)
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin windowsapp)

# List of absolute paths to libraries that should be bundled with the plugin
set(flutter_app_icon_badge_bundled_libraries
  ""
  PARENT_SCOPE
)
