LOCAL_PATH := $(call my-dir)
CUSTOM_LIB_DIR 	:= ../../../aol/libs/$(TARGET_ARCH_ABI)

include $(CLEAR_VARS)

LOCAL_MODULE    := libdzy
LOCAL_SRC_FILES := $(CUSTOM_LIB_DIR)/libdzy.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := sviewer
LOCAL_SRC_FILES := main.cpp
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../../include
LOCAL_CPPFLAGS  := -std=c++11
LOCAL_LDLIBS    := -llog -landroid
LOCAL_SHARED_LIBRARIES := -ldzy
#CUSTOM_LIB_DIR 	:= $(LOCAL_PATH)/../../../aol/libs/$(TARGET_ARCH_ABI)
#PRODUCT_COPY_FILES += $(CUSTOM_LIB_DIR)/libaol.so
#PRODUCT_COPY_FILES += $(CUSTOM_LIB_DIR)/libare.so
include $(BUILD_SHARED_LIBRARY)

