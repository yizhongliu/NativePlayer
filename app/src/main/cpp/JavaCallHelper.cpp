//
// Created by llm on 19-8-29.
//

#include "JavaCallHelper.h"
#include "macro.h"

JavaCallHelper::JavaCallHelper(JavaVM *javaVM_, JNIEnv *env_, jobject instance_) {
    this->javaVM = javaVM_;
    this->env = env_;

    //this->instance = instance_;//不能直接赋值！
    //一旦涉及到 jobject 跨方法、跨线程，需要创建全局引用
    this->instance = env->NewGlobalRef(instance_);
    jclass clazz = env->GetObjectClass(instance);

    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V");
    jmd_onError = env->GetMethodID(clazz, "onError", "(I)V");
    jmd_onProgress = env->GetMethodID(clazz, "onProgress", "(I)V");
}

JavaCallHelper::~JavaCallHelper() {
    javaVM = 0;
    env->DeleteGlobalRef(instance);
    instance = 0;
}

void JavaCallHelper::onPrepared(int threadMode) {
    if (threadMode == THREAD_MAIN) {
        //主线程
        env->CallVoidMethod(instance, jmd_prepared);
    } else {
        //子线程
        //当前子线程的 JNIEnv
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_prepared);
        javaVM->DetachCurrentThread();
    }
}

void JavaCallHelper::onError(int threadMode, int errorCode) {
    if (threadMode == THREAD_MAIN) {
        //主线程
        env->CallVoidMethod(instance, jmd_onError, errorCode);
    } else {
        //子线程
        //当前子线程的 JNIEnv
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_onError, errorCode);
        javaVM->DetachCurrentThread();
    }
}

void JavaCallHelper::onProgress(int threadMode, int progress) {
    if (threadMode == THREAD_MAIN) {
        //主线程
        env->CallVoidMethod(instance, jmd_onProgress, progress);
    } else {
        //子线程
        //当前子线程的 JNIEnv
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmd_onProgress, progress);
        javaVM->DetachCurrentThread();
    }
}