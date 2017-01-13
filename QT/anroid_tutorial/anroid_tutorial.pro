TEMPLATE = app

QT += qml quick sensors svg xml

CONFIG += c++11

SOURCES += main.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

DISTFILES += \
    Bluebubble.svg \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat


android {
    message("* Using settings for Android.")

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

    # Change this paths to your own OpenCV for Android installation
    INCLUDEPATH += /home/dima/Downloads/OpenCV-android-sdk/sdk/native/jni/include

    # Change the last part (armeabi-v7a) according to your build
    OPENCV3RDPARTYLIBS = /home/dima/Downloads/OpenCV-android-sdk/sdk/native/3rdparty/libs/armeabi-v7a
    OPENCVNATIVELIBS = /home/dima/Downloads/OpenCV-android-sdk/sdk/native/libs/armeabi-v7a


    LIBS += $$OPENCV3RDPARTYLIBS/liblibtiff.a \
        $$OPENCV3RDPARTYLIBS/liblibjpeg.a \
        $$OPENCV3RDPARTYLIBS/liblibjasper.a \
        $$OPENCV3RDPARTYLIBS/liblibpng.a \
        $$OPENCV3RDPARTYLIBS/libtbb.a \
        $$OPENCV3RDPARTYLIBS/libIlmImf.a

    LIBS += $$OPENCVNATIVELIBS/libopencv_core.a \
    #    $$OPENCVNATIVELIBS/libopencv_androidcamera.a \
        $$OPENCVNATIVELIBS/libopencv_flann.a \
        $$OPENCVNATIVELIBS/libopencv_imgproc.a \
        $$OPENCVNATIVELIBS/libopencv_highgui.a \
        $$OPENCVNATIVELIBS/libopencv_features2d.a \
        $$OPENCVNATIVELIBS/libopencv_calib3d.a \
        $$OPENCVNATIVELIBS/libopencv_ml.a \
        $$OPENCVNATIVELIBS/libopencv_objdetect.a \
        $$OPENCVNATIVELIBS/libopencv_photo.a \
        $$OPENCVNATIVELIBS/libopencv_stitching.a \
        $$OPENCVNATIVELIBS/libopencv_superres.a \
        $$OPENCVNATIVELIBS/libopencv_ts.a \
        $$OPENCVNATIVELIBS/libopencv_video.a \
        $$OPENCVNATIVELIBS/libopencv_videostab.a \
        $$OPENCVNATIVELIBS/libopencv_java3.so \
    #    $$OPENCVNATIVELIBS/libopencv_info.so

    ANDROID_EXTRA_LIBS += $$OPENCVNATIVELIBS/libopencv_java3.so \
    #    $$OPENCVNATIVELIBS/libopencv_info.so
}



linux:!android {
    message("* Using settings for Unix/Linux.")
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib \
#    -lopencv_core \
#    -lopencv_imgcodecs \
#    -lopencv_highgui \
    -lopencv_calib3d \
    -lopencv_core \
    -lopencv_features2d \
    -lopencv_flann \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_imgproc \
    -lopencv_ml \
    -lopencv_objdetect \
    -lopencv_photo \
    -lopencv_shape \
    -lopencv_stitching \
    -lopencv_superres \
    -lopencv_videoio \
    -lopencv_video \
    -lopencv_videostab

}
