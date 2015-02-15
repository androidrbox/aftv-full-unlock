Amazon FireTV Bootloader Unlock
==========

This tool when run as root on the Amazon FireTV will unlock the bootloader and then reboot.  It requires the bootloader from 51.1.0.2 or earlier.

## Download
    git clone --recursive https://github.com/androidrbox/aftv-full-unlock.git

## Compile
Ensure you have the android-ndk in your PATH.

    make

## Run
    adb push aftv-full-unlock /data/local/tmp
    adb shell
    $ su
    # chmod 755 /data/local/tmp/aftv-full-unlock
    # /data/local/tmp/aftv-full-unlock

After which is will unlock and reboot.  After it restarts, running `adb shell` should result in a root prompt.

## Conclusion
Thanks to everyone mentioned at [http://forum.xda-developers.com/kindle-fire-hdx/orig-development/dev-bootloader-unlock-procedure-software-t3030281](http://forum.xda-developers.com/kindle-fire-hdx/orig-development/dev-bootloader-unlock-procedure-software-t3030281).  I'm no quite sure who is responsible for what.  I wrote most of this code months ago, before Cuber was released, but thanks to dpeddi and whoever else for figuring out the mmc ID stuff, I have decided to publish it.
