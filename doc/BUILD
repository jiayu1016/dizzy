*** Build assimp 3.1.1 ***
0. environment
android-ndk-r9d x86_64 on ubuntu 14.04 64bit, assimp 3.1.1
.bashrc settings:
export ANDROID_NDK=$HOME/android/android-ndk-r9d
export ANDROID_SDK=$HOME/android/adt-bundle-linux-x86_64-20140702/sdk
export PATH=$ANDROID_NDK:$ANDROID_SDK/platform-tools:$ANDROID_SDK/tools:$PATH
export ANDROID_NDK_TOOLCHAIN=$HOME/android/mytoolchain
export ANDROID_CMAKE_TOOLCHAIN=$HOME/android/android.toolchain.cmake

1.  make toolchain
$ cd $ANDROID_NDK/build/tools
$ ./make-standalone-toolchain.sh --platform=android-19 --ndk-dir=$ANDROID_NDK --install-dir=$HOME/android/mytoolchain --toolchain=arm-linux-androideabi-4.8 --system=linux-x86_64

2. download android.toolchain.cmake to $HOME/android

3. generate makefile
$ mkdir buildandroid
$ cd buildandroid
$ cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_CMAKE_TOOLCHAIN -DENABLE_BOOST_WORKAROUND=ON -DCMAKE_INSTALL_PREFIX=$PWD -DANDROID_STL=gnustl_shared -DANDROID_ABI=armeabi-v7a _DANDROID_NATIVE_API_LEVEL=android-19 -DANDROID_FORCE_ARM_BUILD=TRUE ..

4. modify soname, otherwise System.loadLibrary will fail
modify file buildandroid => code => cMakeFiles => assimp.dir => link.txt
search for the string -soname, libassimp.so.3 and remove the .3 from the extension of the library

5. build
$ make
$ make install

6. rename and copy
$ cp libassimp.so.3.1.1 $(PROJECT_TOP_DIR)/libs/armeabi-v7a/libassimp.so
$ cp -a include/assimp $(PROJECT_TOP_DIR)/include
