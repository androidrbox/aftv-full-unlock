#
# Copyright (C) 2015 rbox
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE           := aftv-full-unlock
LOCAL_SRC_FILES        := aftv-full-unlock.c
LOCAL_CFLAGS           := -Iandroid-external-openssl-ndk-static/include
LOCAL_LDLIBS := android-external-openssl-ndk-static/obj/local/armeabi/libcrypto.a
include $(BUILD_EXECUTABLE)
