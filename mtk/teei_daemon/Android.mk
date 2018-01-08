# Copyright 2013 The Android Open Source Project
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := main.c md5_utils.c klog.c

LOCAL_SHARED_LIBRARIES := libcutils liblog

LOCAL_MODULE := teei_daemon
LOCAL_PROPRIETARY_MODULE := true

LOCAL_CFLAGS := -Werror

include $(BUILD_EXECUTABLE)
