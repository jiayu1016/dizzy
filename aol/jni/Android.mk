LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := aol
LOCAL_SRC_FILES := app_context.cpp native_app.cpp
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../include
LOCAL_CPPFLAGS  := -std=c++11
LOCAL_LDLIBS    := -llog -landroid
LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := are
LOCAL_SRC_FILES := are.cpp
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../include
LOCAL_CPPFLAGS  := -std=c++11
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2
LOCAL_SHARED_LIBRARIES := -laol
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

