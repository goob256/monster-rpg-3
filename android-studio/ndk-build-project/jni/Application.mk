
# Uncomment this if you're using STL in your project
# See CPLUSPLUS-SUPPORT.html in the NDK documentation for more information
APP_STL := c++_shared

APP_ABI := armeabi-v7a x86

# Min SDK level
APP_PLATFORM=android-19

# goobliata
APP_CPPFLAGS += -fexceptions -frtti

APP_OPTIM := release
