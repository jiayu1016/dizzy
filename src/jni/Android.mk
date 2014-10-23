LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := assimp
LOCAL_SRC_FILES := ../../libs/$(TARGET_ARCH_ABI)/libassimp.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := dzy
LOCAL_SRC_FILES :=                        \
    app_context.cpp         \
    native_app.cpp          \
    scene.cpp               \
    render.cpp              \
    assimp_adapter.cpp
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../include
LOCAL_CPPFLAGS  := -std=c++11
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv3
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_STATIC_LIBRARIES := android_native_app_glue ndk_helper
LOCAL_SHARED_LIBRARIES := assimp
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,android/ndk_helper)

