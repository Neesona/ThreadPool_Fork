cmake_minimum_required(VERSION 3.10)

#工程名
project(ThreadPool)

set(CMAKE_CXX_STANDARD 14)  # 可以使用std::make_unique
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定配置，也可以cmake -D CMAKE_CONFIGURATION_TYPES="Debug;Release" ..
if (MSVC)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Build configuration types." FORCE)
endif (MSVC)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug, Release, RelWithDebInfo, MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin) # exe和dll生成目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin) # 对vs无用
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin) # 静态库和导入库生成目录

file(GLOB SRC
    "*.h" 
    "*.cpp"
)

add_executable(ThreadPool ${SRC})