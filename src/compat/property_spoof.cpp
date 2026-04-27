#include <cstring>
#include <cstdlib>
#include <sys/system_properties.h>
#include <android/log.h>

#define TAG "ShieldCompat"
#define LOG(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

static const struct {
    const char* name;
    const char* value;
} spoof_props[] = {
    { "ro.product.model",        "SHIELD Android TV"  },
    { "ro.product.brand",        "NVIDIA"             },
    { "ro.product.name",         "foster_e"           },
    { "ro.product.device",       "foster"             },
    { "ro.product.board",        "foster"             },
    { "ro.product.manufacturer", "NVIDIA"             },
    { "ro.board.platform",       "tegra"              },
    { "ro.hardware",             "tegra"              },
    { "ro.build.product",        "foster"             },
    { "ro.build.characteristics","tv"                 },
    { "ro.build.flavor",         "foster_e-user"      },
    { "ro.build.fingerprint",    "NVIDIA/foster_e/foster:5.1/LMY47D/35739_609.6420:user/release-keys" },
    { "ro.opengles.version",     "196609"             },
    { nullptr, nullptr }
};

extern "C" int __system_property_get(const char* name, char* value) {
    for (int i = 0; spoof_props[i].name != nullptr; i++) {
        if (strcmp(name, spoof_props[i].name) == 0) {
            strcpy(value, spoof_props[i].value);
            LOG("spoofed: %s = %s", name, value);
            return strlen(value);
        }
    }

    LOG("passthrough: %s", name);
    const prop_info* pi = __system_property_find(name);
    if (!pi) {
        value[0] = '\0';
        return 0;
    }

    char prop_name[PROP_NAME_MAX];
    char prop_value[PROP_VALUE_MAX];
    __system_property_read(pi, prop_name, prop_value);
    strcpy(value, prop_value);
    return strlen(value);
}
