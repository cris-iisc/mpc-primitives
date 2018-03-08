

file(GLOB_RECURSE SRCS *.cpp)

include_directories(${CMAKE_SOURCE_DIR})
link_directories($ENV{HOME}/libscapi/install/lib/)

add_executable(frontend_cryptoTools  ${SRCS})
target_link_libraries(frontend_cryptoTools cryptoTools tests_cryptoTools libboost_system.a libboost_thread.a)
