cmake_minimum_required(VERSION 3.22.1)
project(mpe_controller LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(mpe_controller SHARED
    src/main.cpp
    src/mpe_engine.cpp
    src/midi_output.cpp
    src/touch_input.cpp
    src/vulkan_renderer.cpp)

find_library(log-lib log)
find_library(android-lib android)
find_library(vulkan-lib vulkan)
find_library(midi-lib amidi)

set(NATIVE_APP_GLUE_DIR ${ANDROID_NDK}/sources/android/native_app_glue)
add_library(native_app_glue STATIC ${NATIVE_APP_GLUE_DIR}/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC ${NATIVE_APP_GLUE_DIR})

target_include_directories(mpe_controller PRIVATE include)

target_link_libraries(mpe_controller
    native_app_glue
    ${android-lib}
    ${log-lib}
    ${vulkan-lib}
    ${midi-lib})
