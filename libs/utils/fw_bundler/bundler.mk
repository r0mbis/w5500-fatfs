FWBUNDLER_DIR 	?= 	$(UTILS_DIR)/fw_bundler
FWBUNDLER     	?= 	$(FWBUNDLER_DIR)/fw_bundler
FWBUNDLER_ARGS  ?= 
# tinyaes lib
C_SOURCES		+=	$(FWBUNDLER_DIR)/src/tinyaes/aes.c
C_INCLUDES		+=	-I$(FWBUNDLER_DIR)/src/tinyaes/ 
C_INCLUDES		+=	-I$(FWBUNDLER_DIR)/src/ 

# uf2 block
C_INCLUDES		+= 	-I$(FWBUNDLER_DIR)/src/uf2/