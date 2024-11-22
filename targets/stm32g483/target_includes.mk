FLASH_ADDRESS	=	0x08000000
OPENOCD_TARGET 	= 	stm32g4x

ARCH 			= 	-mcpu=cortex-m4

CORESRC_DIR 	=	$(TARGET_PATH)/Core
HAL_LWIP_DIR	=	$(TARGET_PATH)/lwip

STM32_HAL	=	$(LIBS_DIR)/vendor/STM32G4


C_DEFS += -DSTM32G483xx 
C_DEFS += -DUSE_HAL_DRIVER 
C_DEFS += -DUSE_FULL_LL_DRIVER 
C_DEFS += -DARM_MATH_CM4
C_DEFS += -DPROJECT_HW_V_MAJOR="0"
C_DEFS += -DPROJECT_HW_V_MINOR="1" 
C_DEFS += -DPROJECT_HW_V_PATCH="0"

ASM_SOURCES	+=	$(TARGET_PATH)/linker/startup.s

# link
LDSCRIPT	+=	$(TARGET_PATH)/linker/flash.ld

# project root dir, newlib glue?
C_INCLUDES	+=	-I$(TARGET_PATH)/
C_SOURCES	+=	$(wildcard $(TARGET_PATH)/*.c)

# sys_hal / target specific
C_INCLUDES	+=	-I$(TARGET_BSP_INCLUDES)
C_SOURCES	+=	$(wildcard $(TARGET_BSP)/*.c)

# st hal

C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_adc.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_adc_ex.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_ll_adc.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_crc.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_crc_ex.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_rcc.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_rtc.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_rcc_ex.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_flash.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_flash_ex.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_flash_ramfunc.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_gpio.c
# C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_exti.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_dma.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_dma_ex.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_pwr.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_pwr_ex.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_cortex.c
# C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_comp.c
# C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_cordic.c
# C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_dac.c
# C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_dac_ex.c
# C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_opamp.c
# C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_opamp_ex.c
# C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_tim.c
# C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_tim_ex.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_uart.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_uart_ex.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_spi.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_hal_spi_ex.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_ll_gpio.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_ll_tim.c
# C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_ll_adc.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_ll_exti.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_ll_cordic.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_ll_opamp.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_ll_pwr.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_ll_rcc.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_ll_dac.c
C_SOURCES	+=	$(STM32_HAL)/Src/stm32g4xx_ll_utils.c


# cube drivers
C_INCLUDES 	+=	-I$(STM32_HAL)/Inc
C_INCLUDES 	+=	-I$(STM32_HAL)/Inc/Legacy

# cmsis
C_INCLUDES 	+=	-I$(CMSIS_DIR)/Device/ST/STM32G4xx/Include
C_INCLUDES 	+=	-I$(CMSIS_DIR)/Include
C_INCLUDES 	+=	-I$(CMSIS_DIR)/DSP/Include

# # cube main.c and others
C_SOURCES	+=	$(wildcard $(CORESRC_DIR)/Src/*.c)
C_INCLUDES	+=	-I$(CORESRC_DIR)/Inc

C_SOURCES	+=	$(wildcard $(TARGET_PATH)/Src/*.c)
C_INCLUDES	+=	-I$(TARGET_PATH)/Inc


STATIC_LIBS += $(CMSIS_DIR)/DSP/Lib/GCC/libarm_cortexM4lf_math.a