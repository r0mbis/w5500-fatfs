
USE_PRINTF_LIB	= 1
USE_MLIB 		= 0
USE_SYSLIB 		= 1
USE_LWIP 		= 0
USE_PROTOBUF 	= 0
USE_FREERTOS 	= 1
USE_FATFS		= 1

OPT 		+= 	-O3 
#-fno-signed-zeros

C_SOURCES	+= 	$(wildcard $(APP_DIR)/*.c) 
C_INCLUDES	+= 	-I$(APP_DIR)/
C_SOURCES	+= 	$(wildcard $(APP_DIR)/W5500/Ethernet/*.c) 
C_SOURCES	+= 	$(wildcard $(APP_DIR)/W5500/Internet/DHCP/*.c)
C_SOURCES	+= 	$(wildcard $(APP_DIR)/W5500/Internet/ICMP/*.c)
C_SOURCES	+= 	$(wildcard $(APP_DIR)/W5500/Internet/FTPServer/*.c)
C_SOURCES	+= 	$(wildcard $(APP_DIR)/SPI_SDcard/*.c)
C_INCLUDES	+= 	-I$(APP_DIR)/W5500/Ethernet/
C_INCLUDES	+= 	-I$(APP_DIR)/W5500/Internet/DHCP/
C_INCLUDES	+= 	-I$(APP_DIR)/W5500/Internet/ICMP/
C_INCLUDES	+= 	-I$(APP_DIR)/W5500/Internet/FTPServer/
C_INCLUDES	+= 	-I$(APP_DIR)/SPI_SDcard/

# protobuf 
ifeq ($(USE_PROTOBUF), 1)
C_SOURCES	+= 	$(wildcard $(LIBS_DIR)/thirdparty/nanopb/*.c)
C_INCLUDES	+= 	-I$(LIBS_DIR)/thirdparty/nanopb
C_SOURCES	+= 	$(wildcard $(APP_DIR)/protobuf/*.c)
C_INCLUDES	+= 	-I$(APP_DIR)/protobuf
include $(LIBS_DIR)/thirdparty/nanopb/proto.mk
endif 


FORMAT_SOURCES += $(shell find ./app -iname "*.h" -o -iname "*.c" -o -iname "*.cpp")