# Hey Emacs, this is a -*- makefile -*-
#
# WinARM template makefile 
# by Martin Thomas, Kaiserslautern, Germany 
# <eversmith@heizung-thomas.de>
#
# based on the WinAVR makefile written by Eric B. Weddington, Jörg Wunsch, et al.
# Released to the Public Domain
# Please read the make user manual!
#
#
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make program = Download the hex file to the device
#
# make filename.s = Just compile filename.c into the assembler code only
#
# make filename.o = Create object filename.o from filename.c (using CFLAGS)
#
# To rebuild project do "make clean" then "make all".
#
# Changelog:
# - 17. Feb. 2005  - added thumb-interwork support (mth)
# - 28. Apr. 2005  - added C++ support (mth)
# - 29. Arp. 2005  - changed handling for lst-Filename (mth)
# -  1. Nov. 2005  - exception-vector placement options (mth)
# - 15. Nov. 2005  - added library-search-path (EXTRA_LIB...) (mth)
# -  2. Dec. 2005  - fixed ihex and binary file extensions (mth)
# - 22. Feb. 2006  - added AT91LIBNOWARN setting (mth)
# - 19. Apr. 2006  - option FLASH_TOOL (default lpc21isp); variable IMGEXT (mth)
# - 23. Jun. 2006  - option USE_THUMB_MODE -> THUMB/THUMB_IW
# -  3. Aug. 2006  - added -ffunction-sections -fdata-sections to CFLAGS
#                    and --gc-sections to LDFLAGS. Only available for gcc 4 (mth)
# -  4. Aug. 2006  - pass SUBMDL-define to frontend (mth)
# - 11. Nov. 2006  - FLASH_TOOL-config, TCHAIN-config (mth)
# - 28. Mar. 2007  - remove .dep-Directory with rm -r -f and force "no error"
# - 24. Apr. 2007  - added "both" option for format (.bin and .hex)
# - 20. Aug. 2007  - extraincdirs in asflags, passing a "board"-define
# - 13. Sep. 2007  - create assembler from c-sources fixed (make foo.s for foo.c)
#                    IMGEXT no longer used and removed, moved some entries

# Toolchain prefix (i.e arm-elf -> arm-elf-gcc.exe)
#TCHAIN = arm-elf
TCHAIN = riscv64-unknown-elf

#FLASH_TOOL = UVISION
#FLASH_TOOL = OPENOCD
#FLASH_TOOL = LPC21ISP 

#USE_THUMB_MODE = YES
#USE_THUMB_MODE = NO

# MCU name, submodel and board
# - MCU used for compiler-option
# - SUBMDL used for linker-script name and passed as define
# - BOARD just passed as define (optional)
MCU      = rv32imfc
SUBMDL   = k1921vg015_flash
BOARD    = 



## not supported in this example:
# RUN_MODE is just passed as define, the user
# has to implement the necessary operations for 
# the used mode(s) (i.e. no copy of .data, remapping)
# Create ROM-Image (final)
RUN_MODE=ROM_RUN

# Create RAM-Image (debugging) - not used in this example
#RUN_MODE=RAM_RUN

## not supported in this example:
# Exception-vector placement is just passed as define,
# the user has to implement the necessary operations
# (i.e. remapping)
# Exception vectors in ROM:
#VECTOR_LOCATION=VECTORS_IN_ROM
# Exception vectors in RAM:
#VECTOR_LOCATION=VECTORS_IN_RAM

# Target file name (without extension).
TARGET = sdr1921

# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files
SRC =  main.c 
SRC += src/plic.c src/printf.c src/sys_init.c src/system_k1921vg015.c
SRC += ./plib015/src/plib015_adcsd.c


#SRC += ./fft/arm_cfft_f32.c 
          
# List C source files here which must be compiled in ARM-Mode.
# use file-extension c for "c-only"-files
#SRCARM = drivers_nxp/mci.c drivers_nxp/dma.c 
SRCARM = 

# List C++ source files here.
# use file-extension cpp for C++-files (use extension .cpp not .C)
CPPSRC = 

# List C++ source files here which must be compiled in ARM-Mode.
# use file-extension cpp for C++-files (use extension .cpp not .C)
#CPPSRCARM = $(TARGET).cpp
CPPSRCARM = 

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = 

# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM = src/startup_k1921vg015.S 

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS = ./inc ./ldscripts ./plib015/inc
#fft fft/dsp

# List any extra directories to look for library files here.
# Also add directories where the linker should search for
# includes to the list
#     Each directory must be seperated by a space.
EXTRA_LIBDIRS = 

# Extra libraries
#    Each library-name must be seperated by a space.
#    i.e. to add libxyz.a, libabc.a and libefsl.a: 
#    EXTRA_LIBS = xyz abc efsl
EXTRA_LIBS = 

# Path to Linker-Scripts
LINKERSCRIPTPATH = ./ldscripts

# Output format. (can be ihex or binary or both)
# (binary i.e. for openocd and SAM-BA, 
# hex i.e. for lpc21isp, newer versions of OpenOCD and uVision)
#FORMAT = ihex
#FORMAT = binary
FORMAT = both

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
#OPT = s
#OPT = 2
OPT = 0

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
#DEBUG = stabs
DEBUG = 3

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options for C here
CDEFS =  -D$(RUN_MODE)
# Project-specific defines:
CDEFS += -DFATLPCMCI_MTH 
#CDEFS += -D_MSC_VER

# Place -I options here
CINCS =

## Using the Atmel AT91_lib produces warning with
## the default warning-levels. 
## yes - disable these warnings; no - keep default settings
#AT91LIBNOWARN = yes
AT91LIBNOWARN = no

#-----

# Place -D or -U options for ASM here
ADEFS =  -D$(RUN_MODE)

ifdef VECTOR_LOCATION
CDEFS += -D$(VECTOR_LOCATION)
ADEFS += -D$(VECTOR_LOCATION)
endif

CDEFS += -D__WinARM__ -D__WINARMSUBMDL_$(SUBMDL)__ -D__WINARMBOARD_$(BOARD)__
ADEFS += -x assembler-with-cpp -I./inc -I./ldscripts -MMD -MP -MF # -D__WinARM__ -D__WINARMSUBMDL_$(SUBMDL)__ -D__WINARMBOARD_$(BOARD)__


# Compiler flags.

ifeq ($(USE_THUMB_MODE),YES)
THUMB    = -mthumb
THUMB_IW = -mthumb-interwork
else 
THUMB    = 
THUMB_IW = 
endif

# Compiler flags to generate dependency files.
### GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS = -g$(DEBUG) -ggdb
CFLAGS += -O$(OPT)
CFLAGS += -march=$(MCU) -mabi=ilp32f
CFLAGS += $(CDEFS) $(CINCS) -I.
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.

#    -O0   
CFLAGS += -msmall-data-limit=8 
CFLAGS += -mstrict-align
CFLAGS += -mno-save-restore
CFLAGS += -fmessage-length=0
CFLAGS += -fsigned-char 
CFLAGS += -ffunction-sections 
CFLAGS += -fdata-sections 
CFLAGS += -fno-common 
CFLAGS += -fno-builtin 
CFLAGS += -flto -ggdb 
CFLAGS += -DSELF_TIMED=1 -Wall -Wextra  -DHSECLK_VAL=16000000 -DSYSCLK_PLL -DCKO_PLL0 -DRETARGET

#CFLAGS += -march=rv32imfc_zba_zbb_zbc_zbs -mabi=ilp32f -O3

#CFLAGS += -Wall -Wcast-align -Wimplicit 

#CFLAGS += -Wpointer-arith -Wswitch

#CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused

#CFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<) 

CFLAGS += $(GENDEPFLAGS)

# flags only for C
CONLYFLAGS += -Wnested-externs 
CONLYFLAGS += $(CSTANDARD)

#ifneq ($(AT91LIBNOWARN),yes)
##AT91-lib warnings with:
#CFLAGS += -Wcast-qual
#CONLYFLAGS += -Wmissing-prototypes 
#CONLYFLAGS += -Wstrict-prototypes
#CONLYFLAGS += -Wmissing-declarations
#endif

# flags only for C++ (arm-elf-g++)
# CPPFLAGS = -fno-rtti -fno-exceptions
CPPFLAGS = 

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
#  -g$(DEBUG): have the assembler create line number information
ASFLAGS = $(ADEFS) -Wa,-adhlns=$(<:.S=.lst),-g$(DEBUG)
ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.


# Additional libraries.

#Support for newlibc-lpc (file: libnewlibc-lpc.a)
#NEWLIBLPC = -lnewlib-lpc

MATH_LIB = -lm

#CPLUSPLUS_LIB = -lstdc++

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -nostartfiles -Wl,-Map=$(TARGET).map,--cref,--gc-sections
LDFLAGS += -lc
LDFLAGS += $(NEWLIBLPC) $(MATH_LIB)
LDFLAGS += -lc -lgcc 
LDFLAGS += $(CPLUSPLUS_LIB)
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))

# Set Linker-Script Depending On Selected Memory and Controller
ifeq ($(RUN_MODE),RAM_RUN)
LDFLAGS +=-T$(LINKERSCRIPTPATH)/$(SUBMDL).ld
else 
LDFLAGS +=-T$(LINKERSCRIPTPATH)/$(SUBMDL).ld
endif

# ---------------------------------------------------------------------------
# Flash-Programming support using lpc21isp by Martin Maurer 
# only for Philips LPC and Analog ADuC ARMs
#
# Settings and variables:
#LPC21ISP = lpc21isp
LPC21ISP = lpc21isp
LPC21ISP_FLASHFILE = $(TARGET).hex
LPC21ISP_PORT = com1
LPC21ISP_BAUD = 57600
LPC21ISP_XTAL = 12000
# other options:
# * verbose output: -debug
# * enter bootloader via RS232 DTR/RTS (only if hardware supports this
#   feature - see Philips AppNote): -control
LPC21ISP_OPTIONS = -control
#LPC21ISP_OPTIONS += -debug
# ---------------------------------------------------------------------------


# Define directories, if needed.
## DIRARM = c:/WinARM/
## DIRARMBIN = $(DIRAVR)/bin/
## DIRAVRUTILS = $(DIRAVR)/utils/bin/

# Define programs and commands.
SHELL   = sh
CC      = $(TCHAIN)-gcc
CPP     = $(TCHAIN)-g++
AR      = $(TCHAIN)-ar
OBJCOPY = $(TCHAIN)-objcopy
OBJDUMP = $(TCHAIN)-objdump
SIZE    = $(TCHAIN)-size
NM      = $(TCHAIN)-nm
COPY    = cp
REMOVE  = rm -f
REMOVEDIR = rm -f -r

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = "-------- begin (mode: $(RUN_MODE)) --------"
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before: 
MSG_SIZE_AFTER = Size after:
MSG_FLASH = Creating load file for Flash:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_COMPILING = Compiling C:
MSG_COMPILING_ARM = "Compiling C (ARM-only):"
MSG_COMPILINGCPP = Compiling C++:
MSG_COMPILINGCPP_ARM = "Compiling C++ (ARM-only):"
MSG_ASSEMBLING = Assembling:
MSG_ASSEMBLING_ARM = "Assembling (ARM-only):"
MSG_CLEANING = Cleaning project:
MSG_FORMATERROR = Can not handle output-format
MSG_LPC21_RESETREMINDER = You may have to bring the target in bootloader-mode now.
MSG_ASMFROMC = "Creating asm-File from C-Source:"
MSG_ASMFROMC_ARM = "Creating asm-File from C-Source (ARM-only):"

# Define all object files.
COLST      = $(SRC:.c=.o.lst) 
COBJ      = $(SRC:.c=.o) 
AOBJ      = $(ASRC:.S=.o)
COBJARM   = $(SRCARM:.c=.o)
AOBJARM   = $(ASRCARM:.S=.o)
CPPOBJ    = $(CPPSRC:.cpp=.o) 
CPPOBJARM = $(CPPSRCARM:.cpp=.o)

# Define all listing files.
LST = $(ASRC:.S=.lst) $(ASRCARM:.S=.lst) $(SRC:.c=.lst) $(SRCARM:.c=.lst)
LST += $(CPPSRC:.cpp=.lst) $(CPPSRCARM:.cpp=.lst)

# Combine all necessary flags and optional flags.
# Add target processor to flags.
#ALL_CFLAGS  = -march=rv32imfc -mabi=ilp32f -msmall-data-limit=8 -mstrict-align -mno-save-restore -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections 
#ALL_CFLAGS += -fno-common -fno-builtin -flto -DSELF_TIMED=1 -Wall -Wextra -g3 -ggdb 

ALL_CFLAGS = -msmall-data-limit=8 
ALL_CFLAGS += -mstrict-align
ALL_CFLAGS += -mno-save-restore
ALL_CFLAGS += -fmessage-length=0
ALL_CFLAGS += -fsigned-char 
ALL_CFLAGS += -ffunction-sections 
ALL_CFLAGS += -fdata-sections 
ALL_CFLAGS += -fno-common 
ALL_CFLAGS += -fno-builtin 
ALL_CFLAGS += -flto -ggdb 
#-x assembler-with-cpp
#ALL_CFLAGS += -DSELF_TIMED=1 -Wall -Wextra  -DHSECLK_VAL=16000000 -DSYSCLK_PLL -DCKO_PLL0 -DRETARGET

ALL_ASFLAGS =  -march=$(MCU) 
ALL_ASFLAGS += -mabi=ilp32f 
ALL_ASFLAGS += -msmall-data-limit=8 
ALL_ASFLAGS += -mstrict-align 
ALL_ASFLAGS += -mno-save-restore 
ALL_ASFLAGS += -O0 
ALL_ASFLAGS += -fmessage-length=0 
ALL_ASFLAGS += -fsigned-char
ALL_ASFLAGS += -ffunction-sections
ALL_ASFLAGS += -fdata-sections
ALL_ASFLAGS += -fno-common 
ALL_ASFLAGS += -fno-builtin
ALL_ASFLAGS += -flto
ALL_ASFLAGS += -DSELF_TIMED=1
ALL_ASFLAGS += -Wall -Wextra
ALL_ASFLAGS += -g3 -ggdb -x assembler-with-cpp
ALL_ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.

# Default target.
#all: begin gccversion sizebefore build sizeafter finished end
all: begin gccversion build sizeafter finished end

ifeq ($(FORMAT),ihex)
build: elf hex lss sym
hex: $(TARGET).hex
IMGEXT=hex
else 
ifeq ($(FORMAT),binary)
build: elf bin lss sym
bin: $(TARGET).bin
IMGEXT=bin
else 
ifeq ($(FORMAT),both)
build: elf hex bin lss sym
hex: $(TARGET).hex
bin: $(TARGET).bin
else 
$(error "$(MSG_FORMATERROR) $(FORMAT)")
endif
endif
endif

elf: $(TARGET).elf
lss: $(TARGET).lss 
sym: $(TARGET).sym

# Eye candy.
begin:
	@echo
	@echo $(MSG_BEGIN)

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	@echo

# Display sizes of sections.
ELFSIZE = $(SIZE) -A $(TARGET).elf
sizebefore:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi

# Display compiler version information.
gccversion : 
	@$(CC) --version

# Program the device.
ifeq ($(FLASH_TOOL),UVISION)
# Program the device with Keil's uVision (needs configured uVision-Workspace). 
program: $(TARGET).hex
	@echo
	@echo "Programming with uVision"
	C:\Keil\uv3\Uv3.exe -f uvisionflash.Uv2 -ouvisionflash.txt
#	$(REMOVE) ../Common_WinARM/$(TARGET).hex
#	$(COPY) $(TARGET).hex ../Common_WinARM/
#	C:\Keil\uv3\Uv3.exe -f ..\Common_WinARM\uvisionflash.Uv2
else
ifeq ($(FLASH_TOOL),OPENOCD)
# Program the device with Dominic Rath's OPENOCD in "batch-mode", needs cfg and "reset-script".
program: $(TARGET).bin
	@echo
	@echo "Programming with OPENOCD"
	openocd_start_flash.cmd
#	@echo "Copying load-file $(TARGET).bin to ../tools/main.bin directory"
#	$(COPY) $(TARGET).bin ../tools/main.bin
#	cd .. && cd tools && openocd_flash_go.cmd
#OOCD_load_image=$(TARGET).bin
#include oocd_flashing.mk
#program : $(TARGET).bin oocd_programprepare oocd_programdoit
#program : $(TARGET).bin oocd_programdoit
else
# Program the device using lpc21isp (for NXP2k and ADuC UART bootloader)
program: $(TARGET).hex
	@echo
	@echo $(MSG_LPC21_RESETREMINDER)
	-$(LPC21ISP) $(LPC21ISP_OPTIONS) $(LPC21ISP_FLASHFILE) $(LPC21ISP_PORT) $(LPC21ISP_BAUD) $(LPC21ISP_XTAL)
endif
endif

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O ihex $< $@
	
# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O binary $< $@

# Create extended listing file from ELF output file.
# testing: option -C
%.lss: %.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S -C $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
%.elf:  $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(THUMB) $(CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@ $(LDFLAGS)
#	$(CPP) $(THUMB) $(CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@ $(LDFLAGS)

# Compile: create object files from C source files. ARM/Thumb
$(COBJ) : %.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(THUMB) $(CFLAGS) $(CONLYFLAGS) $< -o $@ 

# Compile: create object files from C source files. ARM-only
$(COBJARM) : %.o : %.c
	@echo
	@echo $(MSG_COMPILING_ARM) $<
	$(CC) -c $(CFLAGS) $(CONLYFLAGS) $< -o $@ 

# Compile: create object files from C++ source files. ARM/Thumb
$(CPPOBJ) : %.o : %.cpp
	@echo
	@echo $(MSG_COMPILINGCPP) $<
	$(CPP) -c $(THUMB) $(CFLAGS) $(CPPFLAGS) $< -o $@ 

# Compile: create object files from C++ source files. ARM-only
$(CPPOBJARM) : %.o : %.cpp
	@echo
	@echo $(MSG_COMPILINGCPP_ARM) $<
	$(CPP) -c $(CFLAGS) $(CPPFLAGS) $< -o $@ 

# Compile: create assembler files from C source files. ARM/Thumb
$(COBJ:.o=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC) $< to $@
	$(CC) $(THUMB) -S $(CFLAGS) $(CONLYFLAGS) $< -o $@

# Compile: create assembler files from C source files. ARM only
$(COBJARM:.o=.s) : %.s : %.c
	@echo $(MSG_ASMFROMC_ARM) $< to $@
	$(CC) -S $(CFLAGS) $(CONLYFLAGS) $< -o $@

# Assemble: create object files from assembler source files. ARM/Thumb
$(AOBJ) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(THUMB) $(ALL_ASFLAGS) $< -o $@

# Assemble: create object files from assembler source files. ARM-only
$(AOBJARM) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING_ARM) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@


# Target: clean project.
clean: begin clean_list finished end

clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).bin
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).a90
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lnk
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(TARGET).lst
	$(REMOVE) $(COBJ)
	$(REMOVE) $(COLST)
	$(REMOVE) $(CPPOBJ)
	$(REMOVE) $(AOBJ)
	$(REMOVE) $(COBJARM)
	$(REMOVE) $(CPPOBJARM)
	$(REMOVE) $(AOBJARM)
	$(REMOVE) $(LST)
#	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRC:.c=.d)
#	$(REMOVE) $(SRCARM:.c=.s)
	$(REMOVE) $(SRCARM:.c=.d)
#	$(REMOVE) $(CPPSRC:.cpp=.s) 
	$(REMOVE) $(CPPSRC:.cpp=.d)
#	$(REMOVE) $(CPPSRCARM:.cpp=.s) 
	$(REMOVE) $(CPPSRCARM:.cpp=.d)
#        $(REMOVEDIR) /dep | exit 0
# 	$(REMOVE) .dep/*

	@echo $(COBJARM)

# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex bin lss sym clean clean_list program

