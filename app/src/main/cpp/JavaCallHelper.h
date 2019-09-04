//
// Created by llm on 19-8-29.
//

#ifndef NATIVEPLAYER_JAVACALLHELPER_H
#define NATIVEPLAYER_JAVACALLHELPER_H

#include <jni.h>

class JavaCallHelper {
public:
    JavaCallHelper(JavaVM *javaVM_, JNIEnv *env_, jobject instance_);

    ~JavaCallHelper();

    void onPrepared(int threadMode);

    void onError(int threadMode, int errorCode);

    void onProgress(int threadMode, int progress);

private:
    JavaVM *javaVM;
    JNIEnv *env;
    jobject instance;
    jmethodID jmd_prepared;
    jmethodID jmd_onError;
    jmethodID jmd_onProgress;

};


#endif //NATIVEPLAYER_JAVACALLHELPER_H
