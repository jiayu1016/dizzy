LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := dzy
LOCAL_SRC_FILES :=          \
    engine_context.cpp      \
    native_core.cpp         \
    scene.cpp               \
    render.cpp              \
    assimp_adapter.cpp      \
    program.cpp
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../../include
LOCAL_CPPFLAGS  := -std=c++11
LOCAL_STATIC_LIBRARIES := android_native_app_glue ndk_helper
include $(BUILD_STATIC_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,android/ndk_helper)

