QEMU ?= qemu-system-i386
NASM ?= nasm

all: os.img

kernel.bin: kernel32.asm
	$(NASM) -f bin $< -o $@

kernel_sectors.inc: kernel.bin
	@bytes=$$(stat -c%s kernel.bin); \
	sectors=$$(( (bytes + 511) / 512 )); \
	echo "KERNEL_SECTORS equ $$sectors" > $@

boot.bin: boot.asm kernel_sectors.inc
	$(NASM) -f bin boot.asm -o boot.bin

os.img: boot.bin kernel.bin
	cat boot.bin kernel.bin > os.img

run: os.img
	$(QEMU) -hda os.img -boot c -no-reboot -no-shutdown -monitor stdio

clean:
	rm -f boot.bin kernel.bin os.img kernel_sectors.inc
