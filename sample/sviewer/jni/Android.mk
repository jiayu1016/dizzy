LOCAL_PATH := $(call my-dir)
MY_SRC_DIR := ../../../src/jni

include $(CLEAR_VARS)
LOCAL_MODULE    := assimp
LOCAL_SRC_FILES := ../../../libs/$(TARGET_ARCH_ABI)/libassimp.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := sviewer
LOCAL_SRC_FILES := main.cpp               \
    $(MY_SRC_DIR)/app_context.cpp         \
    $(MY_SRC_DIR)/native_app.cpp          \
    $(MY_SRC_DIR)/are.cpp                 \
    $(MY_SRC_DIR)/scene.cpp               \
    $(MY_SRC_DIR)/assimp_adapter.cpp
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../../include
LOCAL_CPPFLAGS  := -std=c++11
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_SHARED_LIBRARIES := assimp
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
