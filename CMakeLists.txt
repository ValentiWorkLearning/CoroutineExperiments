cmake_minimum_required(VERSION 3.14)

project(coroutine_experiments C CXX)

add_executable(coroutine_experiments_app ${PROJECT_SOURCES})
#add_subdirectory(3rdparty)

target_sources(
    coroutine_experiments_app
    PRIVATE
    main.cpp
)
target_compile_features(
    coroutine_experiments_app
    PRIVATE
    cxx_std_20
)

#target_link_libraries(
#    coroutine_experiments_app
#    PRIVATE
#    cppcoro
#)