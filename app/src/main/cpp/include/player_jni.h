//
// Created by llm on 19-7-19.
//

#ifndef VIDEOEDIT_PLAYER_JNI_H
#define VIDEOEDIT_PLAYER_JNI_H

#include <stdint.h>
#include <jni.h>


JNIEXPORT JavaVM* get_jni_jvm(void);
JNIEXPORT JNIEnv* get_jni_env(void);
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*);

#ifdef __cplusplus
extern "C" {
#endif

void  JniDetachCurrentThread(void);
void *JniRequestWinObj(void *data);
void  JniReleaseWinObj(void *data);
void  JniPostMessage  (void *extra, int32_t msg, int64_t param);

#ifdef __cplusplus
}
#endif


#endif //VIDEOEDIT_PLAYER_JNI_H
