#include <android/log.h>
#define TAG "ShieldCompat"
#define LOG(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)

extern "C" {
    int nvsiInit(void* ctx)          { LOG("nvsiInit stubbed"); return 0; }
    int nvsiInitEx(void* ctx)        { return 0; }
    int nvsiVerify(void* ctx)        { LOG("nvsiVerify stubbed"); return 0; }
    int nvsiCheckStatus(void* ctx)   { return 0; }
    int nvsiClose(void* ctx)         { return 0; }
    int nvsiPause(void* ctx)         { return 0; }
    int nvsiResume(void* ctx)        { return 0; }
    int nvsiRunSnippet(void* ctx)    { return 0; }
    int nvsiUpdateSnippet(void* ctx) { return 0; }
    int nvsiForceRecheck(void* ctx)  { return 0; }
    void nvsiGetGlobalData()         {}
    void nvsiSetGlobalData()         {}
}
