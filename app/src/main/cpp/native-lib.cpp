#include <jni.h>
#include <string>

extern "C"
jstring
Java_com_ghnor_seetaface_1as_HomeActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
