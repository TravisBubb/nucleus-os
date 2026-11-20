# Nuke built-in rules and variables.
MAKEFLAGS += -rR
.SUFFIXES:

# Delete the target of a failed recipe.
.DELETE_ON_ERROR:

# ----------------------------------------------------------------------------
# Project configuration
# ----------------------------------------------------------------------------

# Name of the final kernel executable. Must match the path in limine.conf.
override OUTPUT := nucleus

# All build artifacts live under here, so nothing lands in the repo root.
override BUILD       := build
override OBJDIR      := $(BUILD)/obj
override KERNEL      := $(BUILD)/$(OUTPUT)
override ISODIR      := $(BUILD)/iso_root
override ISO         := $(BUILD)/$(OUTPUT).iso
override LIMINEDIR   := $(BUILD)/limine-binary
override LIMINE_TAR  := $(BUILD)/limine-binary.tar.gz
override LIMINE_URL  := https://github.com/limine-bootloader/limine/releases/latest/download/limine-binary.tar.gz

# QEMU knobs. Override on the command line, e.g.
#   make run QEMUFLAGS="-m 4G"            # no KVM
#   make run OVMF=/path/to/OVMF.fd        # pick firmware for run-uefi
QEMUFLAGS ?= -m 2G -enable-kvm -serial stdio
OVMF      ?= /usr/share/edk2/x64/OVMF.4m.fd

# ----------------------------------------------------------------------------
# Toolchain (user controllable)
# ----------------------------------------------------------------------------

TOOLCHAIN :=
TOOLCHAIN_PREFIX :=
ifneq ($(TOOLCHAIN),)
    ifeq ($(TOOLCHAIN_PREFIX),)
        TOOLCHAIN_PREFIX := $(TOOLCHAIN)-
    endif
endif

ifneq ($(TOOLCHAIN_PREFIX),)
    CC := $(TOOLCHAIN_PREFIX)gcc
else
    CC := cc
endif

LD := $(TOOLCHAIN_PREFIX)ld

# Defaults overrides for variables if using "llvm" as toolchain.
ifeq ($(TOOLCHAIN),llvm)
    CC := clang
    LD := ld.lld
endif

# User controllable flags.
CFLAGS := -g -O2 -pipe
CPPFLAGS :=
NASMFLAGS := -g
LDFLAGS :=

# Check if CC is Clang.
override CC_IS_CLANG := $(shell ! $(CC) --version 2>/dev/null | grep -q '^Target: '; echo $$?)

# If the C compiler is Clang, set the target as needed.
ifeq ($(CC_IS_CLANG),1)
    override CC += \
        -target x86_64-unknown-none-elf
endif

# Internal C flags that should not be changed by the user.
override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIC \
    -ffunction-sections \
    -fdata-sections \
    -m64 \
    -march=x86-64 \
    -mabi=sysv \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
    -mcmodel=kernel

# Internal C preprocessor flags that should not be changed by the user.
override CPPFLAGS := \
    -I src \
    $(CPPFLAGS) \
    -MMD \
    -MP

# Internal nasm flags that should not be changed by the user.
override NASMFLAGS += \
    -f elf64 \
    -i src/ \
    -Wall

# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
    -m elf_x86_64 \
    -nostdlib \
    -static \
    -z max-page-size=0x1000 \
    -z noexecstack \
    --gc-sections \
    -T linker.lds

# Check if the linker supports -no-pie and enable it if it does.
override LD_HAS_NO_PIE := $(shell ! $(LD) --help 2>/dev/null | grep -qE '(^|[[:space:]])--?no-pie([[:space:]]|$$)'; echo $$?)

ifeq ($(LD_HAS_NO_PIE),1)
    override LDFLAGS += \
        -no-pie
endif

# ----------------------------------------------------------------------------
# Source discovery
# ----------------------------------------------------------------------------

override SRCFILES := $(shell find -L src -type f 2>/dev/null | LC_ALL=C sort)
override CFILES := $(filter %.c,$(SRCFILES))
override ASFILES := $(filter %.S,$(SRCFILES))
override NASMFILES := $(filter %.asm,$(SRCFILES))
override OBJ := $(addprefix $(OBJDIR)/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix $(OBJDIR)/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d) $(NASMFILES:.asm=.asm.d))

# ----------------------------------------------------------------------------
# Targets
# ----------------------------------------------------------------------------

# Show the help menu by default.
.DEFAULT_GOAL := help

.PHONY: help
help: ## Show this help menu
	@echo "nucleus-os build targets:"
	@echo
	@grep -hE '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) \
		| sort \
		| awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-12s\033[0m %s\n", $$1, $$2}'
	@echo
	@echo "Useful overrides: QEMUFLAGS, OVMF, TOOLCHAIN=llvm"

.PHONY: all
all: $(KERNEL) ## Build the kernel ELF only

.PHONY: iso
iso: $(ISO) ## Build the bootable ISO image

.PHONY: run
run: $(ISO) ## Build (if needed) and boot the ISO in QEMU (BIOS)
	qemu-system-x86_64 $(QEMUFLAGS) -cdrom $(ISO) -boot d

.PHONY: run-uefi
run-uefi: $(ISO) ## Build (if needed) and boot the ISO in QEMU (UEFI, needs OVMF)
	@test -f "$(OVMF)" || { echo "OVMF firmware not found at $(OVMF); set OVMF=/path/to/OVMF.fd"; exit 1; }
	qemu-system-x86_64 $(QEMUFLAGS) -bios "$(OVMF)" -cdrom $(ISO) -boot d

# Include header dependencies.
-include $(HEADER_DEPS)

# Link the final kernel executable.
$(KERNEL): GNUmakefile linker.lds $(OBJ)
	mkdir -p "$(dir $@)"
	$(LD) $(LDFLAGS) $(OBJ) -o $@

# Compilation rules for *.c files.
$(OBJDIR)/%.c.o: %.c GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.S files.
$(OBJDIR)/%.S.o: %.S GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.asm (nasm) files.
$(OBJDIR)/%.asm.o: %.asm GNUmakefile
	mkdir -p "$(dir $@)"
	nasm $(NASMFLAGS) -MD $(@:.o=.d) -MP $< -o $@

# Download and build the Limine bootloader binary release.
$(LIMINEDIR)/limine:
	mkdir -p "$(BUILD)"
	curl -fL -o "$(LIMINE_TAR)" "$(LIMINE_URL)"
	tar -xzf "$(LIMINE_TAR)" -C "$(BUILD)"
	$(MAKE) -C "$(LIMINEDIR)"

# Assemble the ISO root and build a bootable hybrid BIOS/UEFI ISO.
$(ISO): $(KERNEL) $(LIMINEDIR)/limine limine.conf
	rm -rf "$(ISODIR)"
	mkdir -p "$(ISODIR)/boot/limine" "$(ISODIR)/EFI/BOOT"
	cp "$(KERNEL)" "$(ISODIR)/boot/"
	cp limine.conf \
	   "$(LIMINEDIR)/limine-bios.sys" \
	   "$(LIMINEDIR)/limine-bios-cd.bin" \
	   "$(LIMINEDIR)/limine-uefi-cd.bin" \
	   "$(ISODIR)/boot/limine/"
	cp "$(LIMINEDIR)/BOOTX64.EFI" "$(LIMINEDIR)/BOOTIA32.EFI" "$(ISODIR)/EFI/BOOT/"
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		"$(ISODIR)" -o "$(ISO)"
	"$(LIMINEDIR)/limine" bios-install "$(ISO)"

.PHONY: clean
clean: ## Remove compiled kernel and object files
	rm -rf "$(OBJDIR)" "$(KERNEL)" "$(ISODIR)" "$(ISO)"

.PHONY: distclean
distclean: ## Remove everything, including the downloaded Limine toolchain
	rm -rf "$(BUILD)"
