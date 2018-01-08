LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=  \
     mtk_nfc_android_main.c  \
     mtk_nfc_sys.c  \
     mtk_nfc_hal_aosp_main.c  \
     mtk_nfc_log.c

LOCAL_C_INCLUDES:= \
     $(LOCAL_PATH)/inc 

LOCAL_MODULE := nfc_nci.mt6605.default

LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_CFLAGS := $(D_CFLAGS) -DNFC_HAL_TARGET=TRUE -DNFC_RW_ONLY=TRUE
LOCAL_CFLAGS += -DUSE_GCC -DSUPPORT_I2C  -DSUPPORT_SHARED_LIBRARY -DHALIMPL -DMTK_LIB_NFC
LOCAL_CPPFLAGS := $(LOCAL_CFLAGS)

LOCAL_SHARED_LIBRARIES := libc libm liblog libcutils libhardware_legacy libmtknfc

include $(BUILD_SHARED_LIBRARY)
