set -e

ISO=modality.iso

mkdir -p iso_root/boot/

# Copy the kernel to the ISO root
cp sys/hive/hive.sys iso_root/boot

# Copy boot files
cp admin/conf/limine.conf sys/boot/limine/limine-bios.sys \
    sys/boot/limine/limine-bios-cd.bin \
    sys/boot/limine/limine-uefi-cd.bin \
    iso_root/

# Generate the ISO
xorriso -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 \
        -boot-info-table --efi-boot limine-uefi-cd.bin -efi-boot-part \
		--efi-boot-image --protective-msdos-label iso_root/ -o $ISO 1>/dev/null

sys/boot/limine/limine bios-install $ISO 1>/dev/null
rm -rf iso_root
