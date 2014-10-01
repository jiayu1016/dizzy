LOCAL_PATH := $(call my-dir)
MY_SRC_DIR := ../../../src/jni

include $(CLEAR_VARS)

include $(CLEAR_VARS)
LOCAL_MODULE    := sviewer
LOCAL_SRC_FILES := main.cpp               \
	$(MY_SRC_DIR)/app_context.cpp         \
	$(MY_SRC_DIR)/native_app.cpp          \
	$(MY_SRC_DIR)/are.cpp
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../../include
LOCAL_CPPFLAGS  := -std=c++11
LOCAL_LDLIBS    := -llog -landroid
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
