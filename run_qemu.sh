#/bin/sh
make clean && make && qemu-system-x86_64 -curses -cdrom rmanhas.iso  -drive id=disk,file=$USER.img,if=none -device ahci,id=ahci -device ide-drive,drive=disk,bus=ahci.0 -gdb tcp::88888 -d cpu_reset -D ./qemu.log
