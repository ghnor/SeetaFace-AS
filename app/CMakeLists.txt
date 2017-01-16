# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             src/main/cpp/native-lib.cpp )

add_library( # Sets the name of the library.
             SeetafaceSo

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             src/main/cpp/FaceAlignment/cfan.cpp
             src/main/cpp/FaceAlignment/face_alignment.cpp
             src/main/cpp/FaceAlignment/sift.cpp
             src/main/cpp/FaceDetection/src/face_detection.cpp
             src/main/cpp/FaceDetection/src/fust.cpp
             src/main/cpp/FaceDetection/src/util/image_pyramid.cpp
             src/main/cpp/FaceDetection/src/util/nms.cpp
             src/main/cpp/FaceDetection/src/feat/lab_feature_map.cpp
             src/main/cpp/FaceDetection/src/feat/surf_feature_map.cpp
             src/main/cpp/FaceDetection/src/io/lab_boost_model_reader.cpp
             src/main/cpp/FaceDetection/src/io/surf_mlp_model_reader.cpp
             src/main/cpp/FaceDetection/src/classifier/lab_boosted_classifier.cpp
             src/main/cpp/FaceDetection/src/classifier/mlp.cpp
             src/main/cpp/FaceDetection/src/classifier/surf_mlp.cpp
             src/main/cpp/FaceIdentification/src/bias_adder_net.cpp
             src/main/cpp/FaceIdentification/src/blob.cpp
             src/main/cpp/FaceIdentification/src/bn_net.cpp
             src/main/cpp/FaceIdentification/src/common_net.cpp
             src/main/cpp/FaceIdentification/src/conv_net.cpp
             src/main/cpp/FaceIdentification/src/eltwise_net.cpp
             src/main/cpp/FaceIdentification/src/inner_product_net.cpp
             src/main/cpp/FaceIdentification/src/log.cpp
             src/main/cpp/FaceIdentification/src/math_functions.cpp
             src/main/cpp/FaceIdentification/src/max_pooling_net.cpp
             src/main/cpp/FaceIdentification/src/net.cpp
             src/main/cpp/FaceIdentification/src/pad_net.cpp
             src/main/cpp/FaceIdentification/src/spatial_transform_net.cpp
             src/main/cpp/FaceIdentification/src/tform_maker_net.cpp
             src/main/cpp/FaceIdentification/tools/aligner.cpp
             src/main/cpp/FaceIdentification/tools/face_identification.cpp
             src/main/cpp/Seetaface_JniClient.cpp
             src/main/cpp/CMImgProc.cpp
             )

include_directories( src/main/cpp/FaceAlignment/include/
                     src/main/cpp/FaceDetection/include/classifier/
                     src/main/cpp/FaceDetection/include/feat/
                     src/main/cpp/FaceDetection/include/io/
                     src/main/cpp/FaceDetection/include/util/
                     src/main/cpp/FaceDetection/include/
                     src/main/cpp/FaceIdentification/include/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

find_library( # Sets the name of the path variable.
              jnigraphics-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              jnigraphics )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )

target_link_libraries( # Specifies the target library.
                       SeetafaceSo

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       ${jnigraphics-lib} )
