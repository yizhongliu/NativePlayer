#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <android/log.h>
#include "FFmpegPlayer.h"

static JavaVM *java_vm;
JavaCallHelper *javaCallHelper = 0;
FFmpegPlayer *ffmpeg = 0;
ANativeWindow *window = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//静态初始化mutex


//1，data;2，linesize；3，width; 4， height
void renderFrame(uint8_t *src_data, int src_lineSize, int width, int height) {
    pthread_mutex_lock(&mutex);

    if (!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);

    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0) != 0) {
        ANativeWindow_release(window);
        window = 0;
        pthread_mutex_unlock(&mutex);
        return;
    }

    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_lineSize = window_buffer.stride * 4;

    for (int i = 0; i < window_buffer.height; i++) {
        memcpy(dst_data + i * dst_lineSize, src_data + i * src_lineSize, dst_lineSize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}

extern "C" JNIEXPORT void JNICALL
native_prepare (
        JNIEnv *env,
        jobject instance, jstring path) {

    const char *dataSource = env->GetStringUTFChars(path, 0);

    javaCallHelper = new JavaCallHelper(java_vm, env, instance);
    ffmpeg = new FFmpegPlayer(javaCallHelper, const_cast<char *>(dataSource));
    ffmpeg->setRenderCallback(renderFrame);
    ffmpeg->prepare();

    env->ReleaseStringUTFChars(path, dataSource);

}

extern "C" JNIEXPORT void JNICALL
native_start (
        JNIEnv *env,
        jobject /* this */) {
    if (ffmpeg) {
        ffmpeg->start();
    }

}

extern "C" JNIEXPORT void JNICALL
native_set_surface (
        JNIEnv *env,
        jobject /* this */, jobject surface) {

    pthread_mutex_lock(&mutex);

    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }

    window = ANativeWindow_fromSurface(env, surface);

    pthread_mutex_unlock(&mutex);

}

/* List of implemented native methods */
static JNINativeMethod native_methods[] = {
        {"nativePrepare", "(Ljava/lang/String;)V", (void *) native_prepare},
        {"nativeStart", "()V", (void *) native_start},
        {"nativeSetSurface", "(Ljava/lang/Object;)V", (void *) native_set_surface}
};


/* Library initializer */
jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;

    java_vm = vm;

    if (vm->GetEnv ((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        __android_log_print (ANDROID_LOG_ERROR, "native-lib",
                             "Could not retrieve JNIEnv");
        return 0;
    }
    jclass klass = env->FindClass ("pri/tool/nativeplayer/NativePlayer");
    env->RegisterNatives ( klass, native_methods,
                           sizeof (native_methods) / sizeof ((native_methods)[0]));


    return JNI_VERSION_1_4;
}