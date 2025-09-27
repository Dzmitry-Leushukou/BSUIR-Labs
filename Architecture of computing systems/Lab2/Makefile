all: boot_image.bin

boot_image.bin: boot_sector.asm
	nasm -f bin -o boot_image.bin boot_sector.asm

run: boot_image.bin
	qemu-system-x86_64 -drive file=boot_image.bin,format=raw

clean:
	rm -f boot_image.bin

.PHONY: all run clean
