TARGETS_LIB			=	$(LIBS_DIR)/targets/$(TARGET)
TARGET_PATH 		= 	$(TARGETS_DIR)/$(TARGET)

TARGET_BSP_INCLUDES	=	$(LIBS_DIR)/targets/bsp
TARGET_BSP 			=	$(TARGETS_LIB)/bsp

FAMILY_ID		= 0x$(shell echo "$(TARGET)boot" | sha1sum | cut -c 1-8 )
FAMILY_ID_APP	= 0x$(shell echo "$(TARGET)fw" | sha1sum | cut -c 1-8)

# uf2 target specific config
C_INCLUDES	+=	-I$(TARGETS_LIB)/

# defs
C_DEFS +=  \
-DUSE_FULL_ASSERT \
-DPROJECT_NAME_STR=\"$(PROJECT_NAME)\" \
-DPROJECT_MODULE_ID="30001" \
-DBOOT_FAMILY_ID=$(FAMILY_ID) \
-DAPP_FAMILY_ID=$(FAMILY_ID_APP)

ifeq ($(DEBUG), 1)
C_DEFS += -DDEBUG_BUILD
$(warning TARGET $(TARGET) DEBUG BUILD)
else
$(warning TARGET $(TARGET) RELEASE BUILD )
endif

# hardware specific includes
include 	$(UTILS_DIR)/fw_bundler/bundler.mk
include		$(TARGET_PATH)/target_includes.mk

# bootloader flag for fwbundler, need for correct descriptor placement
FWBUNDLER_ARGS += -b