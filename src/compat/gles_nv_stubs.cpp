#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <android/log.h>


typedef unsigned int   GLenum;
typedef int            GLint;
typedef unsigned int   GLuint;
typedef int            GLsizei;
typedef unsigned char  GLubyte;
typedef unsigned int   GLbitfield;
typedef float          GLfloat;
typedef long           GLintptr;
typedef long           GLsizeiptr;
typedef void (*__eglMustCastToProperFunctionPointerType)(void);

#define GL_EXTENSIONS       0x1F03
#define GL_NUM_EXTENSIONS   0x821D
#define GL_BUFFER_SIZE      0x8764
#define GL_WRITE_ONLY       0x88B9
#define GL_MAP_READ_BIT     0x0001
#define GL_MAP_WRITE_BIT    0x0002
#define GL_MAP_PERSISTENT_BIT  0x0040
#define GL_MAP_COHERENT_BIT    0x0080
#define GL_CLIENT_STORAGE_BIT  0x0200

// Logging

#define TAG "NVStubs"
// it's hardcoded for super Mega baseball ik it's ugly 
static const char* log_paths[] = {
    "/sdcard/Android/data/com.metalheadsoftware.smb/tsec.log",
    "/data/data/com.metalheadsoftware.smb/tsec.log",
    nullptr
};

static void log_file(const char* msg) {
    for (int i = 0; log_paths[i]; i++) {
        FILE* f = fopen(log_paths[i], "a");
        if (f) { fprintf(f, "%s\n", msg); fclose(f); return; }
    }
}

#define LOGI(fmt, ...) do { \
    char _b[512]; snprintf(_b, sizeof(_b), "[I] " fmt, ##__VA_ARGS__); \
    __android_log_print(ANDROID_LOG_INFO,  TAG, "%s", _b); \
    log_file(_b); \
} while(0)

#define LOGE(fmt, ...) do { \
    char _b[512]; snprintf(_b, sizeof(_b), "[E] " fmt, ##__VA_ARGS__); \
    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s", _b); \
    log_file(_b); \
} while(0)


static void* s_gles = nullptr;
static void* s_egl  = nullptr;

static void ensure_libs() {
    if (!s_gles) s_gles = dlopen("libGLESv3.so", RTLD_NOW | RTLD_GLOBAL);
    if (!s_gles) s_gles = dlopen("libGLESv2.so", RTLD_NOW | RTLD_GLOBAL);
    if (!s_egl)  s_egl  = dlopen("libEGL.so",    RTLD_NOW | RTLD_GLOBAL);
}

static void* resolve(const char* sym) {
    ensure_libs();
    void* fn = nullptr;
    if (s_gles) fn = dlsym(s_gles, sym);
    if (!fn && s_egl) {
        auto real_egl = (__eglMustCastToProperFunctionPointerType(*)(const char*))
            dlsym(s_egl, "eglGetProcAddress");
        if (real_egl) fn = (void*)real_egl(sym);
    }
    if (!fn) LOGE("resolve: MISSING [%s]", sym);
    return fn;
}


static __eglMustCastToProperFunctionPointerType
    (*s_real_eglGetProcAddress)(const char*)               = nullptr;
static const GLubyte* (*s_real_glGetString)(GLenum)        = nullptr;
static const GLubyte* (*s_real_glGetStringi)(GLenum,GLuint)= nullptr;
static void (*s_real_glGetIntegerv)(GLenum, GLint*)        = nullptr;
static void* (*s_real_eglGetCurrentContext)()              = nullptr;

static void ensure_real_fns() {
    ensure_libs();
    if (!s_real_eglGetProcAddress && s_egl)
        s_real_eglGetProcAddress = (decltype(s_real_eglGetProcAddress))
            dlsym(s_egl, "eglGetProcAddress");
    if (!s_real_glGetString && s_gles)
        s_real_glGetString = (decltype(s_real_glGetString))
            dlsym(s_gles, "glGetString");
    if (!s_real_glGetStringi && s_gles)
        s_real_glGetStringi = (decltype(s_real_glGetStringi))
            dlsym(s_gles, "glGetStringi");
    if (!s_real_glGetIntegerv && s_gles)
        s_real_glGetIntegerv = (decltype(s_real_glGetIntegerv))
            dlsym(s_gles, "glGetIntegerv");
    if (!s_real_eglGetCurrentContext && s_egl)
        s_real_eglGetCurrentContext = (decltype(s_real_eglGetCurrentContext))
            dlsym(s_egl, "eglGetCurrentContext");
}

// Returns true if a valid EGL context is current
// Prevents GL calls before context creation or after destruction

static bool has_context() {
    ensure_real_fns();
    if (!s_real_eglGetCurrentContext) return false;
    void* ctx = s_real_eglGetCurrentContext();
    return ctx != nullptr && ctx != (void*)-1; // EGL_NO_CONTEXT = 0
}

#define GET_FN(type, var, name) \
    static type var = nullptr;  \
    if (!var) var = (type)resolve(name);

typedef void  (*FN_DrawElements)(GLenum,GLsizei,GLenum,const void*,GLsizei);
typedef void  (*FN_DrawArrays)(GLenum,GLint,GLsizei,GLsizei);
typedef void  (*FN_AttribDivisor)(GLuint,GLuint);
typedef void  (*FN_TexImage3D)(GLenum,GLint,GLint,GLsizei,GLsizei,GLsizei,
                                GLint,GLenum,GLenum,const void*);
typedef void  (*FN_TexSubImage3D)(GLenum,GLint,GLint,GLint,GLint,
                                   GLsizei,GLsizei,GLsizei,GLenum,GLenum,const void*);
typedef void  (*FN_CompTexImage3D)(GLenum,GLint,GLenum,GLsizei,GLsizei,
                                    GLsizei,GLint,GLsizei,const void*);
typedef void  (*FN_CompTexSubImage3D)(GLenum,GLint,GLint,GLint,GLint,
                                       GLsizei,GLsizei,GLsizei,GLenum,GLsizei,const void*);
typedef void  (*FN_RenderbufferMultisample)(GLenum,GLsizei,GLenum,GLsizei,GLsizei);
typedef void  (*FN_BlitFramebuffer)(GLint,GLint,GLint,GLint,
                                     GLint,GLint,GLint,GLint,GLbitfield,GLenum);
typedef void* (*FN_MapBufferRange)(GLenum,GLintptr,GLsizeiptr,GLbitfield);

// NV Stubs 

extern "C" {

void glDrawElementsInstancedNV(GLenum mode, GLsizei count, GLenum type,
                                const void* indices, GLsizei instancecount) {
    if (!has_context()) { LOGE("glDrawElementsInstancedNV: no context!"); return; }
    GET_FN(FN_DrawElements, fn, "glDrawElementsInstanced");
    if (fn) fn(mode, count, type, indices, instancecount);
    else LOGE("glDrawElementsInstancedNV: no backend!");
}

void glDrawArraysInstancedNV(GLenum mode, GLint first,
                              GLsizei count, GLsizei instancecount) {
    if (!has_context()) { LOGE("glDrawArraysInstancedNV: no context!"); return; }
    GET_FN(FN_DrawArrays, fn, "glDrawArraysInstanced");
    if (fn) fn(mode, first, count, instancecount);
    else LOGE("glDrawArraysInstancedNV: no backend!");
}

void glVertexAttribDivisorNV(GLuint index, GLuint divisor) {
    if (!has_context()) { LOGE("glVertexAttribDivisorNV: no context!"); return; }
    GET_FN(FN_AttribDivisor, fn, "glVertexAttribDivisor");
    if (fn) fn(index, divisor);
    else LOGE("glVertexAttribDivisorNV: no backend!");
}

void glTexImage3DNV(GLenum target, GLint level, GLint internalformat,
                    GLsizei width, GLsizei height, GLsizei depth,
                    GLint border, GLenum format, GLenum type, const void* pixels) {
    if (!has_context()) { LOGE("glTexImage3DNV: no context!"); return; }
    GET_FN(FN_TexImage3D, fn, "glTexImage3D");
    if (fn) fn(target, level, internalformat, width, height,
               depth, border, format, type, pixels);
    else LOGE("glTexImage3DNV: no backend!");
}

void glTexSubImage3DNV(GLenum target, GLint level,
                       GLint xoffset, GLint yoffset, GLint zoffset,
                       GLsizei width, GLsizei height, GLsizei depth,
                       GLenum format, GLenum type, const void* pixels) {
    if (!has_context()) { LOGE("glTexSubImage3DNV: no context!"); return; }
    GET_FN(FN_TexSubImage3D, fn, "glTexSubImage3D");
    if (fn) fn(target, level, xoffset, yoffset, zoffset,
               width, height, depth, format, type, pixels);
    else LOGE("glTexSubImage3DNV: no backend!");
}

void glCompressedTexImage3DNV(GLenum target, GLint level, GLenum internalformat,
                               GLsizei width, GLsizei height, GLsizei depth,
                               GLint border, GLsizei imageSize, const void* data) {
    if (!has_context()) { LOGE("glCompressedTexImage3DNV: no context!"); return; }
    GET_FN(FN_CompTexImage3D, fn, "glCompressedTexImage3D");
    if (!fn) {
        static FN_CompTexImage3D fn_oes = nullptr;
        if (!fn_oes) fn_oes = (FN_CompTexImage3D)resolve("glCompressedTexImage3DOES");
        fn = fn_oes;
    }
    if (fn) fn(target, level, internalformat, width, height,
               depth, border, imageSize, data);
    else LOGE("glCompressedTexImage3DNV: no backend!");
}

void glCompressedTexSubImage3DNV(GLenum target, GLint level,
                                  GLint xoffset, GLint yoffset, GLint zoffset,
                                  GLsizei width, GLsizei height, GLsizei depth,
                                  GLenum format, GLsizei imageSize, const void* data) {
    if (!has_context()) { LOGE("glCompressedTexSubImage3DNV: no context!"); return; }
    GET_FN(FN_CompTexSubImage3D, fn, "glCompressedTexSubImage3D");
    if (!fn) {
        static FN_CompTexSubImage3D fn_oes = nullptr;
        if (!fn_oes) fn_oes = (FN_CompTexSubImage3D)
            resolve("glCompressedTexSubImage3DOES");
        fn = fn_oes;
    }
    if (fn) fn(target, level, xoffset, yoffset, zoffset,
               width, height, depth, format, imageSize, data);
    else LOGE("glCompressedTexSubImage3DNV: no backend!");
}

void glRenderbufferStorageMultisampleNV(GLenum target, GLsizei samples,
                                         GLenum internalformat,
                                         GLsizei width, GLsizei height) {
    if (!has_context()) { LOGE("glRenderbufferStorageMultisampleNV: no context!"); return; }
    GET_FN(FN_RenderbufferMultisample, fn, "glRenderbufferStorageMultisample");
    if (!fn) {
        static FN_RenderbufferMultisample fn_ext = nullptr;
        if (!fn_ext) fn_ext = (FN_RenderbufferMultisample)
            resolve("glRenderbufferStorageMultisampleEXT");
        fn = fn_ext;
    }
    if (fn) fn(target, samples, internalformat, width, height);
    else LOGE("glRenderbufferStorageMultisampleNV: no backend!");
}

void glBlitFramebufferNV(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                          GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                          GLbitfield mask, GLenum filter) {
    if (!has_context()) { LOGE("glBlitFramebufferNV: no context!"); return; }
    GET_FN(FN_BlitFramebuffer, fn, "glBlitFramebuffer");
    if (!fn) {
        static FN_BlitFramebuffer fn_ext = nullptr;
        if (!fn_ext) fn_ext = (FN_BlitFramebuffer)resolve("glBlitFramebufferEXT");
        fn = fn_ext;
    }
    if (fn) fn(srcX0, srcY0, srcX1, srcY1,
               dstX0, dstY0, dstX1, dstY1, mask, filter);
    else LOGE("glBlitFramebufferNV: no backend!");
}


void* glMapBufferRangeEXT(GLenum target, GLintptr offset,
                           GLsizeiptr length, GLbitfield access) {
    if (!has_context()) {
        LOGE("glMapBufferRangeEXT: no context!");
        return nullptr;
    }

    LOGI("glMapBufferRangeEXT: target=0x%x offset=%ld length=%ld access=0x%x",
         target, (long)offset, (long)length, access);

    // Log which flags are set
    if (access & GL_MAP_PERSISTENT_BIT)
        LOGI("glMapBufferRangeEXT: has GL_MAP_PERSISTENT_BIT");
    if (access & GL_MAP_COHERENT_BIT)
        LOGI("glMapBufferRangeEXT: has GL_MAP_COHERENT_BIT");
    if (access & GL_MAP_READ_BIT)
        LOGI("glMapBufferRangeEXT: has GL_MAP_READ_BIT");
    if (access & GL_MAP_WRITE_BIT)
        LOGI("glMapBufferRangeEXT: has GL_MAP_WRITE_BIT");

    GET_FN(FN_MapBufferRange, fn, "glMapBufferRange");
    void* ptr = fn ? fn(target, offset, length, access) : nullptr;

    if (!ptr)
        LOGE("glMapBufferRangeEXT: returned NULL! target=0x%x length=%ld access=0x%x",
             target, (long)length, access);
    else
        LOGI("glMapBufferRangeEXT: OK ptr=%p", ptr);

    return ptr;
}


struct NVStub { const char* name; void* ptr; };

static const NVStub nv_stubs[] = {
    {"glDrawElementsInstancedNV",         (void*)glDrawElementsInstancedNV},
    {"glDrawArraysInstancedNV",           (void*)glDrawArraysInstancedNV},
    {"glVertexAttribDivisorNV",           (void*)glVertexAttribDivisorNV},
    {"glTexImage3DNV",                    (void*)glTexImage3DNV},
    {"glTexSubImage3DNV",                 (void*)glTexSubImage3DNV},
    {"glCompressedTexImage3DNV",          (void*)glCompressedTexImage3DNV},
    {"glCompressedTexSubImage3DNV",       (void*)glCompressedTexSubImage3DNV},
    {"glRenderbufferStorageMultisampleNV",(void*)glRenderbufferStorageMultisampleNV},
    {"glBlitFramebufferNV",               (void*)glBlitFramebufferNV},
    {"glMapBufferRangeEXT",               (void*)glMapBufferRangeEXT},
    {nullptr, nullptr}
};

__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char* procname) {
    for (int i = 0; nv_stubs[i].name; i++) {
        if (strcmp(procname, nv_stubs[i].name) == 0) {
            LOGI("eglGetProcAddress: stub [%s]", procname);
            return (__eglMustCastToProperFunctionPointerType)nv_stubs[i].ptr;
        }
    }
    ensure_real_fns();
    auto result = s_real_eglGetProcAddress
                ? s_real_eglGetProcAddress(procname)
                : nullptr;
    if (result)
        LOGI("eglGetProcAddress: OK [%s] = %p", procname, (void*)result);
    else
        LOGE("eglGetProcAddress: NULL [%s]", procname);
    return result;
}

static const char* nv_ext_list[] = {
    "GL_NV_draw_instanced",
    "GL_NV_instanced_arrays",
    "GL_NV_texture_array",
    "GL_NV_framebuffer_multisample",
    "GL_NV_framebuffer_blit",
    "GL_EXT_map_buffer_range",
    nullptr
};

static const char** s_ext_list  = nullptr;
static int          s_ext_count = 0;

static void build_ext_list() {
    if (s_ext_list) return;
    ensure_real_fns();

    // Must have valid context before querying extensions
    if (!has_context()) {
        LOGE("build_ext_list: no EGL context, skipping");
        return;
    }

    GLint real_count = 0;
    if (s_real_glGetIntegerv)
        s_real_glGetIntegerv(GL_NUM_EXTENSIONS, &real_count);

    LOGI("build_ext_list: device has %d extensions", real_count);

    int nv_count = 0;
    for (int i = 0; nv_ext_list[i]; i++) nv_count++;

    s_ext_list = new const char*[real_count + nv_count + 1];
    s_ext_count = 0;

    for (int i = 0; i < real_count; i++) {
        const GLubyte* e = s_real_glGetStringi
                         ? s_real_glGetStringi(GL_EXTENSIONS, i)
                         : nullptr;
        if (e) s_ext_list[s_ext_count++] = (const char*)e;
    }

    for (int i = 0; nv_ext_list[i]; i++) {
        bool found = false;
        for (int j = 0; j < s_ext_count; j++) {
            if (strcmp(s_ext_list[j], nv_ext_list[i]) == 0) {
                found = true; break;
            }
        }
        if (!found) {
            LOGI("build_ext_list: injecting %s", nv_ext_list[i]);
            s_ext_list[s_ext_count++] = nv_ext_list[i];
        }
    }
    s_ext_list[s_ext_count] = nullptr;
    LOGI("build_ext_list: total %d extensions", s_ext_count);
}

static char ext_string_buf[8192];
static bool ext_string_ready = false;

const GLubyte* glGetString(GLenum name) {
    if (!has_context()) {
        ensure_real_fns();
        return s_real_glGetString ? s_real_glGetString(name) : nullptr;
    }
    ensure_real_fns();
    const GLubyte* result = s_real_glGetString
                          ? s_real_glGetString(name)
                          : nullptr;
    if (name == GL_EXTENSIONS && result && !ext_string_ready) {
        build_ext_list();
        if (s_ext_list && s_ext_count > 0) {
            ext_string_buf[0] = 0;
            for (int i = 0; i < s_ext_count; i++) {
                if (i > 0) strncat(ext_string_buf, " ",
                                   sizeof(ext_string_buf) - strlen(ext_string_buf) - 1);
                strncat(ext_string_buf, s_ext_list[i],
                        sizeof(ext_string_buf) - strlen(ext_string_buf) - 1);
            }
            ext_string_ready = true;
            LOGI("glGetString: extension string spoofed");
        }
    }
    return (name == GL_EXTENSIONS && ext_string_ready)
           ? (const GLubyte*)ext_string_buf
           : result;
}

// glGetStringi hook 

const GLubyte* glGetStringi(GLenum name, GLuint index) {
    if (name == GL_EXTENSIONS) {
        if (!has_context()) {
            LOGE("glGetStringi: no context!");
            return nullptr;
        }
        build_ext_list();
        if (s_ext_list && (int)index < s_ext_count)
            return (const GLubyte*)s_ext_list[index];
        return nullptr;
    }
    ensure_real_fns();
    return s_real_glGetStringi
         ? s_real_glGetStringi(name, index)
         : nullptr;
}

// glGetIntegerv hook 

void glGetIntegerv(GLenum pname, GLint* params) {
    if (pname == GL_NUM_EXTENSIONS) {
        if (!has_context()) {
            LOGE("glGetIntegerv(GL_NUM_EXTENSIONS): no context!");
            if (params) *params = 0;
            return;
        }
        build_ext_list();
        if (params) *params = s_ext_count;
        return;
    }
    ensure_real_fns();
    if (s_real_glGetIntegerv) s_real_glGetIntegerv(pname, params);
}

} // extern "C"
