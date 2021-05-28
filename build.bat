@echo off

echo select vdisk file="%cd%\boot.vhd" > wrvhd
echo attach vdisk >> wrvhd
diskpart /s "%cd%\wrvhd"

cd build
copy CARBLOAD.EXE T:\SYSTEM\CARBLOAD.SYS
cd ..

cd examples
cd multi_basic
cd release
copy KERNEL.SYS T:\SYSTEM\KERNEL.SYS
copy HAL.SYS T:\SYSTEM\BOOT\HAL.SYS
cd ..
cd ..
cd ..

echo select vdisk file="%cd%\boot.vhd" > wrvhd
echo detach vdisk >> wrvhd
diskpart /s "%cd%\wrvhd"
del wrvhd

qemu-system-x86_64 -m 1024 -monitor stdio -d int -no-shutdown -no-reboot -accel hax -hda "%cd%\boot.vhd"