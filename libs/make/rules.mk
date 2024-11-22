all: $(BUILD_DIR)/$(PROJECT).bin $(BUILD_DIR)/$(PROJECT).uf2 $(PROTOBUF_TARGETS) $(FLATBUFFS_TARGETS)
bin: $(BUILD_DIR)/$(PROJECT).bin 
uf2: $(BUILD_DIR)/$(PROJECT).uf2 


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# compile c sources
ifneq '' '$(findstring gcc,$(COMPILER))'
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@printf "\tCC\t%b\n" "$(COM_COLOR)$(COM_STRING)$(OBJ_COLOR)$<$(DMAGENTA) -> $(COM_COLOR2)$@$(NO_COLOR)";
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@
else
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@printf "\tCL\t%b\n" "$(COM_COLOR)$(COM_STRING)$(OBJ_COLOR)$<$(DMAGENTA) -> $(COM_COLOR2)$@$(NO_COLOR)";
	$(CC) -c $(CFLAGS) $< -o $@ 
endif 

$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS) Makefile 
	@eval $(LD) $(OBJECTS) $(LDFLAGS) -o $@
	@printf "\tLD\t%b\n" "$(COM_COLOR)$(COM_STRING)$(OBJ_COLOR) *.o $(DMAGENTA) -> $(COM_COLOR2)$@$(NO_COLOR)";


# compile asm sources
$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR) 
	@printf "\tASM\t%b\n" "$(COM_COLOR)$(COM_STRING)$(OBJ_COLOR)$<$(DMAGENTA) -> $(COM_COLOR2)$@$(NO_COLOR)";
	$(AS) -c $(ASFLAGS) $< -o $@ 

# make bin image
$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf | $(BUILD_DIR)
	@printf "\tBIN\t%b\n" "$(COM_COLOR)$(COM_STRING)$(OBJ_COLOR)$<$(DMAGENTA) -> $(COM_COLOR2)$@$(NO_COLOR)";
	$(BIN) $< $@

# make uf2 firmware image
$(BUILD_DIR)/$(PROJECT).uf2: $(BUILD_DIR)/$(PROJECT).bin
#	@cd $(FWBUNDLER_DIR) && git submodule update --init --recursive && make all
	@cd $(FWBUNDLER_DIR) && make all
	$(FWBUNDLER) -v -e -c $(FWBUNDLER_ARGS) -f $(FAMILY_ID) -a $(FLASH_ADDRESS) -i $< -o $@	

# flash via openocd
$(BUILD_DIR)/flash: $(BUILD_DIR)/$(PROJECT).bin
	openocd $(OPENOCD_FLASH_OPTS) -c "reset init" -c "program $(BUILD_DIR)/$(PROJECT).bin verify reset exit $(FLASH_ADDRESS)" 

# flash with stlink utility
$(BUILD_DIR)/flash_st: $(BUILD_DIR)/$(PROJECT).bin
	@echo "\tFLASH\t"
	st-flash --freq=4000k --reset --connect-under-reset write $(BUILD_DIR)/$(PROJECT).bin $(FLASH_ADDRESS) 

# make build dir
$(BUILD_DIR):
	mkdir -p $@

#######################################
# clean up
#######################################
clean: 
	@echo "\tCLEAN\t"
	$(RM) -rf $(PROJECT_ROOT)/.build/
	$(RM) -rf $(FLATBUFF_APP_DIR)/*.h

update:
	git submodule update --remote

init:
	git submodule update --init --recursive
#######################################
# flash
#######################################
rst:
	@echo "t\RESET\t"
	openocd $(OPENOCD_FLASH_OPTS) -c "reset" -c "exit"

rst_st:
	@echo "t\RESET\t"
	st-flash reset

flash: $(BUILD_DIR)/flash

flash_st: $(BUILD_DIR)/flash_st

shell: 
	@python3.10 -m serial.tools.miniterm --filter colorize --eol=lf /dev/tty.usbmodem11** 115200
#######################################
# flash
#######################################

format:
	@printf "%b" "\t$(BG_YELLOW) Formatting sources with clang-format $(NO_COLOR) \t\n"
	@clang-format -style=file:${UTILS_DIR}/.clang-format -i $(FORMAT_SOURCES)

ramstats:
	@printf "%b" "\t$(BG_YELLOW) RAM usage statistics $(NO_COLOR) \t\n"
	@$(ELF_ANALYZE) $(BUILD_DIR)/$(PROJECT).elf -t arm-none-eabi- -R | more -r

romstats:
	@printf "%b" "\t$(BG_YELLOW) ROM usage statistics $(NO_COLOR) \t\n"
	@$(ELF_ANALYZE) $(BUILD_DIR)/$(PROJECT).elf -t arm-none-eabi- -F | more -r

magic:
	@printf "\t%b0x%04x%04x%b\t\n" "$(COM_COLOR)" $$RANDOM $$RANDOM "$(NO_COLOR)"

db:
	make clean
	compiledb make
#######################################
# debug
#######################################
configurator:
	@printf "%b" "$(BG_YELLOW) Starting configurator utility... $(NO_COLOR) \t\n"
	@python3 $(CONFIGURATOR_DIR)/configurator.py

scan:
	@make clean
	scan-build --analyzer-target=arm-none-eabi -analyze-headers --force-analyze-debug-code \
	-enable-checker nullability.NullableDereferenced -enable-checker  nullability.NullableReturnedFromNonnull \
	-enable-checker optin.performance.Padding -enable-checker  optin.performance.GCDAntipattern -enable-checker valist.CopyToSelf \
	-enable-checker  nullability.NullablePassedToNonnull -enable-checker nullability.NullableReturnedFromNonnull  \
	-o $(BUILD_DIR) make

debug: flash
	arm-none-eabi-gdb \
		-ex 'target extended-remote | openocd -c "gdb_port pipe" $(OPENOCD_OPTS)' \
		-ex "set confirm off" \
		-ex "source $(UTILS_DIR)/FreeRTOS/FreeRTOS.py" \
		-ex "source $(UTILS_DIR)/PyCortexMDebug/PyCortexMDebug.py" \
		-ex "svd_load $(TARGETS_LIB)/dbg/cmsis.svd" \
		-ex "compare-sections" \
		$(BUILD_DIR)/$(PROJECT).elf; \

debug_other:
	arm-none-eabi-gdb \
		-ex 'target extended-remote | openocd -c "gdb_port pipe" $(OPENOCD_OPTS)' \
		-ex "set confirm off" \
		-ex "source $(UTILS_DIR)/PyCortexMDebug/PyCortexMDebug.py" \
		-ex "svd_load $(TARGETS_LIB)/dbg/cmsis.svd" \

openocd:
	openocd $(OPENOCD_FLASH_OPTS)



#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***
