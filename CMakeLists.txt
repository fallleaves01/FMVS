cmake_minimum_required(VERSION 3.20)
project(my_project CXX)

# ---------------- 基本配置 ----------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 默认构建类型：Release（如果你没手动指定的话）
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # 给 Release 配置更激进的优化
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native")
  set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   -O3 -march=native -mtune=native")

  # 开启 LTO（CMake 会自动加 -flto 等）
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

include(FetchContent)

# ------------ FetchContent 第三方库（header-only 化）------------

# -------- spdlog：header-only 模式 --------
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.14.1
)

FetchContent_GetProperties(spdlog)
if(NOT spdlog_POPULATED)
  FetchContent_Populate(spdlog)
endif()

# 只用它的头文件，不跑它自己的 CMake
add_library(spdlog_header_only INTERFACE)
target_include_directories(spdlog_header_only
  INTERFACE
    ${spdlog_SOURCE_DIR}/include
)
target_compile_definitions(spdlog_header_only
  INTERFACE SPDLOG_HEADER_ONLY=1
)
add_library(spdlog::spdlog ALIAS spdlog_header_only)

# -------- Eigen：原生 header-only --------
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.4.0
)

FetchContent_GetProperties(eigen)
if(NOT eigen_POPULATED)
  FetchContent_Populate(eigen)
endif()

add_library(eigen3 INTERFACE)
target_include_directories(eigen3
  INTERFACE
    ${eigen_SOURCE_DIR}      # 里面有 Eigen/Dense 等
)
add_library(Eigen3::Eigen ALIAS eigen3)

# -------- nlohmann_json：header-only --------
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)

FetchContent_GetProperties(nlohmann_json)
if(NOT nlohmann_json_POPULATED)
  FetchContent_Populate(nlohmann_json)
endif()

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json
  INTERFACE
    ${nlohmann_json_SOURCE_DIR}/include
)
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)

# -------- parallel-hashmap：header-only --------
FetchContent_Declare(
  parallel_hashmap
  GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git
  GIT_TAG v1.3.12
)

FetchContent_GetProperties(parallel_hashmap)
if(NOT parallel_hashmap_POPULATED)
  FetchContent_Populate(parallel_hashmap)
endif()

add_library(phmap INTERFACE)
target_include_directories(phmap
  INTERFACE
    ${parallel_hashmap_SOURCE_DIR}/parallel_hashmap
)
add_library(phmap::phmap ALIAS phmap)

# -------- mimalloc：必须编译成真正的库 --------
FetchContent_Declare(
  mimalloc
  GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
  GIT_TAG v2.1.7
)

set(MI_BUILD_TESTS  OFF CACHE BOOL "" FORCE)
set(MI_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(MI_BUILD_STATIC ON  CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(mimalloc)

# ------------ OpenMP & OpenBLAS 走系统 ------------

find_package(OpenMP REQUIRED)
# 如果需要 BLAS/OpenBLAS：
# find_package(BLAS REQUIRED)

# 1. 搜集 include 目录下的所有头文件
file(GLOB_RECURSE HEADER_FILES "include/*.hpp" "include/*.h")

add_executable(fmvs
  src/main.cpp
  ${HEADER_FILES}  # 2. 将头文件加入源列表，专门为了让 IDE 识别上下文
)

target_include_directories(fmvs
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

set_property(TARGET fmvs PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

target_link_libraries(fmvs
  PRIVATE
    spdlog::spdlog
    Eigen3::Eigen
    nlohmann_json::nlohmann_json
    phmap::phmap
    OpenMP::OpenMP_CXX
    mimalloc-static
    # BLAS::BLAS   # 如果上面 find_package(BLAS REQUIRED) 了可以加上
)
