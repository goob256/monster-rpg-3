
# Uncomment this if you're using STL in your project
# See CPLUSPLUS-SUPPORT.html in the NDK documentation for more information
APP_STL := c++_shared

APP_ABI := arm64-v8a x86_64

# Min SDK level
APP_PLATFORM=android-21

# goobliata
APP_CPPFLAGS += -fexceptions -frtti

APP_OPTIM := release
