project(FaceDetection)
cmake_minimum_required(VERSION 2.8)

find_package(OpenCV REQUIRED)

include_directories(include)
file(GLOB_RECURSE SRC include/*.h src/*.cpp)

add_executable(FaceDetection ${SRC})
target_link_libraries(FaceDetection ${OpenCV_LIBS})
