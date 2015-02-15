#
# Copyright (C) 2015 rbox
#

all:
	cd android-external-openssl-ndk-static && ndk-build
	ndk-build
	cp libs/armeabi/aftv-full-unlock .
