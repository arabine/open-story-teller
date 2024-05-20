# Story player project

The Story Player is a Flutter application.

# Packages

sudo apt-get install clang cmake ninja-build pkg-config libgtk-3-dev liblzma-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev

# Environment variables

```
export PATH=$PATH:/home/anthony/development/flutter

export ANDROID_HOME=/mnt/work/android-sdk
export PATH=$PATH:$ANDROID_HOME/tools/bin:$ANDROID_HOME/platform-tools
```

# Build for Linux

flutter build linux

# Build and run for Android


flutter build apk


adb devices
adb -s emulator-5554  install -r  build/app/outputs/flutter-apk/app-release.apk
adb push stories /storage/emulated/0/Download/
