#include <jni.h>
#include <string>
#include <android/native_window_jni.h>
#include <android/log.h>
#include "FFmpegPlayer.h"
#include "net/NetTimeProvider.h"
#include "net/NetTimeClient.h"

static JavaVM *java_vm;
JavaCallHelper *javaCallHelper = 0;
FFmpegPlayer *ffmpeg = 0;
ANativeWindow *window = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//静态初始化mutex

NetTimeProvider *netTimeProvider = 0;
NetTimeClient *netTimeClient = 0;


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

extern "C" JNIEXPORT void JNICALL
native_stop (JNIEnv *env,
             jobject /* this */) {
    if (ffmpeg) {
        ffmpeg->stop();
    }
}

extern "C" JNIEXPORT void JNICALL
native_release (JNIEnv *env,
             jobject /* this */) {
    pthread_mutex_lock(&mutex);
    if (window) {
        //把老的释放
        ANativeWindow_release(window);
        window = 0;
    }
    pthread_mutex_unlock(&mutex);
    DELETE(ffmpeg);
}

extern "C" JNIEXPORT jint JNICALL
native_getDuration (JNIEnv *env,
                jobject /* this */) {
    jint ret = 0;
    if (ffmpeg) {
        ret = ffmpeg->getDuration();
    }
    return ret;
}

extern "C" JNIEXPORT void JNICALL
native_start_net_time_provider (JNIEnv *env,
                    jobject /* this */,
                    jstring ip,
                    jint port) {
    const char *ip_addr = env->GetStringUTFChars(ip, 0);

    netTimeProvider = new NetTimeProvider(const_cast<char *>(ip_addr), port);
    netTimeProvider->start();

    env->ReleaseStringUTFChars(ip, ip_addr);
}

extern "C" JNIEXPORT void JNICALL
native_stop_net_time_provider (JNIEnv *env,
                                jobject /* this */) {

    if (netTimeProvider) {
        netTimeProvider->stop();
        delete netTimeProvider;
        netTimeProvider = 0;
    }
}

extern "C" JNIEXPORT void JNICALL
native_start_net_time_client (JNIEnv *env,
                                jobject /* this */,
                                jstring ip,
                                jint port) {
    const char *ip_addr = env->GetStringUTFChars(ip, 0);

    netTimeClient = new NetTimeClient(const_cast<char *>(ip_addr), port);
    netTimeClient->start();

    env->ReleaseStringUTFChars(ip, ip_addr);
}

extern "C" JNIEXPORT void JNICALL
native_stop_net_time_client (JNIEnv *env,
                               jobject /* this */) {

    if (netTimeClient) {
        netTimeClient->stop();
        delete netTimeClient;
        netTimeClient = 0;
    }
}

/* List of implemented native methods */
static JNINativeMethod native_methods[] = {
        {"nativePrepare", "(Ljava/lang/String;)V", (void *) native_prepare},
        {"nativeStart", "()V", (void *) native_start},
        {"nativeSetSurface", "(Ljava/lang/Object;)V", (void *) native_set_surface},
        {"nativeStop", "()V", (void *) native_stop},
        {"nativeRelease", "()V", (void *) native_release},
        {"nativeGetDuration", "()I", (void *) native_getDuration},
        {"nativeStartNetTimeProvider", "(Ljava/lang/String;I)V", (void *) native_start_net_time_provider},
        {"nativeStopNetTimeProvider", "()V", (void *) native_stop_net_time_provider},
        {"nativeStartNetTimeClient", "(Ljava/lang/String;I)V", (void *) native_start_net_time_client},
        {"nativeStopNetTimeClient", "()V", (void *) native_stop_net_time_client}
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