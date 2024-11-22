PROTOC = $(LIBS_DIR)/thirdparty/nanopb/nanopb_generator.py
PROTOC_OPT = --strip-path -L quote

PROTO_DIR ?= $(APP_DIR)/protobuf
CONFIGURATOR_DIR ?= $(LIBS_DIR)/utils/configurator/

PROTOS := $(wildcard $(PROTO_DIR)/*.proto)
PROTOBUF_TARGETS := $(patsubst $(PROTO_DIR)/%.proto,$(PROTO_DIR)/%.pb.c,$(PROTOS))

.PHONY: proto force

# Add a dummy target to always force the rebuild
force: 

proto: $(PROTOBUF_TARGETS)

# Always run the compilation by depending on the force target
$(PROTO_DIR)/%.pb.c: $(PROTO_DIR)/%.proto force
	@printf "\tPROTOBUF\t%b\n" "$(COM_COLOR)$(COM_STRING)$(OBJ_COLOR)$<$(DMAGENTA) -> $(COM_COLOR2)$@$(NO_COLOR)";
	python3 $(PROTOC) $(PROTOC_OPT) $<
	protoc --python_out=$(CONFIGURATOR_DIR) $<
