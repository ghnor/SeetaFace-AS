APP_STL := gnustl_static
# 允许使用c++11的函数功能
NDK_TOOLCHAIN_VERSION=4.8
APP_CPPFLAGS := -std=c++11 
#APP_CPPFLAGS += -pthread
#允许使用异常功能和运行时类型识别
APP_CPPFLAGS += -frtti -fexceptions 
#APP_STL  := stlport_static

APP_ABI := armeabi-v7a 
#APP_ABI := armeabi armeabi-v7a x86 #（或者APP_ABI := all）

APP_PLATFORM := android-19