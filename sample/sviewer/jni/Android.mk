LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := assimp
LOCAL_SRC_FILES := ../../../libs/$(TARGET_ARCH_ABI)/libassimp.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := dzy
LOCAL_SRC_FILES := ../../../src/obj/local/$(TARGET_ARCH_ABI)/libdzy.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := sviewer
LOCAL_SRC_FILES := main.cpp
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../../include
LOCAL_CPPFLAGS  := -std=c++11
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv3
LOCAL_WHOLE_STATIC_LIBRARIES := android_native_app_glue ndk_helper dzy
LOCAL_SHARED_LIBRARIES := assimp
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,android/ndk_helper)

