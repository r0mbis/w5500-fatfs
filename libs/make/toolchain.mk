# Set things up to use the arm-none-eabi-gcc that is on
# our path by default but allow things to be easily overridden, i.e:
#
# COMPILER=<PATH_TO_OTHER_GCC_VERSION>/arm-none-eabi-gcc make
# COMPILER=clang make

## target arch
ARCH 		?= -mcpu=cortex-m4
CPU			=  $(ARCH)
FPU			= -mfpu=fpv4-sp-d16
FLOAT-ABI 	= -mfloat-abi=hard
MCU 		= $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

PREFIX = arm-none-eabi-
COMPILER ?= $(PREFIX)gcc

CC = $(COMPILER) -std=gnu2x
LD = $(PREFIX)gcc -std=gnu2x
AS = $(PREFIX)gcc -std=gnu2x -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
BIN = $(CP) -O binary -S

UF2CONV_DIR = $(UTILS_DIR)/uf2conv
UF2CONV     = $(UF2CONV_DIR)/uf2conv
ELF_ANALYZE = python3 $(UTILS_DIR)/elf_size_analyze.py


ifneq '' '$(findstring clang,$(COMPILER))'

COMPILER_SPECIFIC_CFLAGS	+= --target=thumbv7em-none-none-eabihf -fshort-enums
# COMPILER_SPECIFIC_CFLAGS 	+= -ffast-math -freciprocal-math -fassociative-math -funsafe-math-optimizations
COMPILER_SPECIFIC_LDFLAGS	+= -B$(LLVM_BIN_PATH) -Wl,-fuse-ld=lld
COMPILER_SPECIFIC_CFLAGS 	+= -DCMAKE_EXPORT_COMPILE_COMMANDS
# Default assumes a GNU Arm toolchain is on your path
# but an explicit path can also be provided by doing
# ARM_GNU_CC=<PATH_TO_GCC> make
ARM_CORTEXM_SYSROOT = \
  $(shell arm-none-eabi-gcc $(ARCH)  -print-sysroot 2>&1)

# The directory where Newlib's libc.a & libm.a reside
# for the specific target architecture
ARM_CORTEXM_MULTI_DIR = \
  $(shell arm-none-eabi-gcc $(ARCH)  -print-multi-directory 2>&1)

# Pick up builtins needed for compilation
ARM_CORTEXM_BUILTINS ?= \
$(shell arm-none-eabi-gcc $(ARCH)  -print-libgcc-file-name 2>&1)

COMPILER_SPECIFIC_CFLAGS += \
  --sysroot=$(ARM_CORTEXM_SYSROOT) 
  --target=arm-none-eabi \
COMPILER_SPECIFIC_LDFLAGS += \
  -L$(ARM_CORTEXM_SYSROOT)/lib/$(ARM_CORTEXM_MULTI_DIR) \
  $(ARM_CORTEXM_BUILTINS)

endif

ifneq '' '$(findstring gcc,$(COMPILER))'

# COMPILER_SPECIFIC_CFLAGS	+=  -fsingle-precision-constant
# COMPILER_SPECIFIC_CFLAGS	+=	-fshort-enums -ffast-math -freciprocal-math -fassociative-math
COMPILER_SPECIFIC_CFLAGS	+=  -Wno-error=format -Wno-error=strict-aliasing

endif


CFLAGS	+= $(COMPILER_SPECIFIC_CFLAGS)
LDFLAGS	+= $(COMPILER_SPECIFIC_LDFLAGS) 
LDFLAGS += $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections



#######################################
# CFLAGS
#######################################
# compile gcc flags
LIBS = -lc -lm -lnosys -nostdlib -fno-builtin-printf
ASFLAGS	+= $(ARCH) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections 
CFLAGS	+= $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections 
# -fstack-protector
LDFLAGS = $(MCU) -specs=nosys.specs -specs=nano.specs

ifeq ($(DEBUG), 1)
CFLAGS	+=	-g3 -gdwarf-2 
LDFLAGS	+=	-g

endif

CFLAGS	+=	-Wall -Wextra -Wshadow -Wdouble-promotion -fno-common -Werror -fno-builtin-printf -Wno-unused-parameter -Wno-error=misleading-indentation -Wno-error=double-promotion
# CFLAGS	+= -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-statement-expression-from-macro-expansion -Wno-newline-eof -Wno-unused-parameter  -Wno-gnu-binary-literal

# Generate dependency information
CFLAGS 	+=	-MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
####################################### 
# libraries


LDFLAGS += -T$(LDSCRIPT) $(STATIC_LIBS) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map,--cref 
LDFLAGS += -Wl,--gc-sections -static -Wl,--undefined=uxTopUsedPriority  -Wl,--print-memory-usage 
LDFLAGS	+= -Wl,--wrap,_malloc_r -Wl,--wrap,_free_r -Wl,--wrap,_calloc_r -Wl,--wrap,_realloc_r


OPENOCD_OPTS	+= 	-f interface/stlink.cfg \
					-c "reset_config srst_only" \
					-c "transport select hla_swd" \
					-f $(TARGETS_LIB)/dbg/openocd.cfg \
					-c "${OPENOCD_TARGET}.cpu0 configure -rtos auto" -c "reset init" \


OPENOCD_FLASH_OPTS 	+=	-f interface/stlink.cfg \
						-c "transport select hla_swd" -f $(TARGETS_LIB)/dbg/openocd.cfg \
						-c "reset_config none separate" -c "init"	\

SVD_FILE += $(UTILS_DIR)/STM32H743x.svd