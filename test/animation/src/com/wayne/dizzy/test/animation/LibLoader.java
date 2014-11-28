package com.wayne.dizzy.test.animation;

import android.app.NativeActivity;

public class LibLoader extends NativeActivity {
    static {
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("assimp");
        System.loadLibrary("animation");
     }
}
