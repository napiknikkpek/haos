cmake_minimum_required(VERSION 3.16)

project(haos)

set(CMAKE_CXX_STANDARD 20)
set(Boost_USE_STATIC_LIBS On)

find_package(Boost 1.79 REQUIRED)
find_package(range-v3 REQUIRED)
find_package(SFML REQUIRED)
find_package(benchmark REQUIRED)

add_compile_options(
    -Wall
    -Wextra
    -Wno-unknown-pragmas
    -fno-omit-frame-pointer
    -g
)

add_executable(haos
    src/main.cpp
    src/collision_schedule.cpp
)

target_link_libraries(haos PUBLIC
    boost::boost
    range-v3::range-v3
    sfml::sfml
)

add_executable(collision_bench
  src/collision_schedule.cpp
  src/collision_bench.cpp
)

target_link_libraries(collision_bench PUBLIC
  boost::boost
  range-v3::range-v3
  benchmark::benchmark
)
