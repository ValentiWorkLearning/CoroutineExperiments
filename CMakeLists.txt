cmake_minimum_required(VERSION 3.14)

project(coroutine_experiments C CXX)

if( UNIX )
    find_package(Threads REQUIRED)
endif()

add_executable(coroutine_experiments_app ${PROJECT_SOURCES})
#add_subdirectory(3rdparty)

if( UNIX )
    target_link_libraries( coroutine_experiments_app PRIVATE Threads::Threads)
    target_compile_options( coroutine_experiments_app PRIVATE_HEADER -fcoroutines)
endif()

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