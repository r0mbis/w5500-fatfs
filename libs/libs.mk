CMSIS_DIR		=	$(LIBS_DIR)/vendor/CMSIS
FREERTOS_DIR	=	$(LIBS_DIR)/thirdparty/FreeRTOS
UTILS_DIR		=	$(LIBS_DIR)/utils
DRIVERS_DIR		=	$(LIBS_DIR)/drivers

ifeq ($(USE_PRINTF_LIB), 1)
PRINTF_DIR		=	$(LIBS_DIR)/thirdparty/printf
C_SOURCES		+=	$(wildcard $(PRINTF_DIR)/*.c)
C_INCLUDES 		+=	-I$(PRINTF_DIR)/
endif
 
ifeq ($(USE_MLIB), 1)
MLIB_DIR		=	$(LIBS_DIR)/thirdparty/mlib
C_SOURCES		+=	$(wildcard $(MLIBS_DIR)/*.c)
C_INCLUDES 		+=	-I$(MLIBS_DIR)/
endif
 
ifeq ($(USE_SYSLIB), 1)
SYS_LIB			=	$(LIBS_DIR)/sys
C_SOURCES		+=	$(wildcard $(SYS_LIB)/*.c)
C_INCLUDES		+=	-I$(SYS_LIB)
endif

ifeq ($(USE_LWIP), 1)
LWIP_DIR		=	$(LIBS_DIR)/thirdparty/LwIP
C_SOURCES		+= 	$(wildcard $(LWIP_DIR)/src/netif/*.c) 
C_SOURCES		+= 	$(wildcard $(LWIP_DIR)/src/api/*.c) 
C_SOURCES		+= 	$(wildcard $(LWIP_DIR)/src/core/*.c) 
C_SOURCES		+= 	$(wildcard $(LWIP_DIR)/src/core/ipv4/*.c) 
C_SOURCES		+= 	$(wildcard $(LWIP_DIR)/system/OS/*.c) 
C_SOURCES		+= 	$(wildcard $(LWIP_DIR)/src/apps/mqtt/*.c) 

C_INCLUDES 		+= -I$(LWIP_DIR)/src/include/
C_INCLUDES 		+= -I$(LWIP_DIR)/src/include/lwip/
C_INCLUDES 		+= -I$(LWIP_DIR)/src/include/lwip/apps/
C_INCLUDES 		+= -I$(LWIP_DIR)/src/include/lwip/priv/
C_INCLUDES 		+= -I$(LWIP_DIR)/src/include/lwip/prot/
C_INCLUDES 		+= -I$(LWIP_DIR)/src/include/netif/
C_INCLUDES 		+= -I$(LWIP_DIR)/system/
C_INCLUDES 		+= -I$(LWIP_DIR)/system/arch/
endif

ifeq ($(USE_FATFS), 1)
FATFS_DIR		=	$(LIBS_DIR)/thirdparty/FatFs
C_SOURCES		+= 	$(wildcard $(FATFS_DIR)/*.c)
C_INCLUDES 		+= -I$(FATFS_DIR)/ 
endif


ifeq ($(USE_FREERTOS), 1)
# freertos
C_SOURCES	+=	$(FREERTOS_DIR)/croutine.c
C_SOURCES	+=	$(FREERTOS_DIR)/event_groups.c
C_SOURCES	+=	$(FREERTOS_DIR)/list.c
C_SOURCES	+=	$(FREERTOS_DIR)/queue.c
C_SOURCES	+=	$(FREERTOS_DIR)/stream_buffer.c
C_SOURCES	+=	$(FREERTOS_DIR)/tasks.c
C_SOURCES	+=	$(FREERTOS_DIR)/timers.c
# C_SOURCES	+=	$(FREERTOS_DIR)/CMSIS_RTOS_V2/cmsis_os2.c
C_SOURCES	+=	$(FREERTOS_DIR)/portable/GCC/ARM_CM4F/port.c
C_SOURCES	+=	$(FREERTOS_DIR)/portable/MemMang/heap_4.c

C_INCLUDES 	+=	-I$(FREERTOS_DIR)/include
# C_INCLUDES 	+=	-I$(FREERTOS_DIR)/CMSIS_RTOS_V2
C_INCLUDES 	+=	-I$(FREERTOS_DIR)/portable/GCC/ARM_CM4F

endif
# FORMAT_SOURCES += $(shell find ./libs -type d \( -name LWIP -o -name STM32H7 -o -name FreeRTOS -o -name CMSIS \) -prune -iname "*.h" -o -iname "*.c" -o -iname "*.cpp")
