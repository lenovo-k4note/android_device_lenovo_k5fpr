LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  mtk.c \
  radiomgr.c \
  radiomod.c \
  bt_drv.c

LOCAL_C_INCLUDES := \
  system/bt/hci/include

# To support MTK vendor opcode
LOCAL_CFLAGS += -DMTK_MT6630
LOCAL_CFLAGS += -D__MTK_MERGE_INTERFACE_SUPPORT_
LOCAL_CFLAGS += -DMTK_BT_COMMON

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libbt-vendor
LOCAL_MULTILIB := both
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SHARED_LIBRARIES := liblog libcutils libnvram
include $(BUILD_SHARED_LIBRARY)
