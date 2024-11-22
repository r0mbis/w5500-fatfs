######################################
# PROJECT
######################################
PROJECT 			= 	stm32_test_driver
PROJECT_NAME 		= 	"stm32_test_driver"
PROJECT_ROOT		=	.
TARGET 				=	discovery_f407vg


APP_DIR				=	$(PROJECT_ROOT)/app
LIBS_DIR			=	$(PROJECT_ROOT)/libs
MAKE_DIR			=	$(PROJECT_ROOT)/libs/make
TARGETS_DIR			=	$(PROJECT_ROOT)/targets
BUILD_DIR			=	$(PROJECT_ROOT)/.build/$(TARGET)

######################################
# makefile utils
######################################
include			$(MAKE_DIR)/meta.mk
include			$(MAKE_DIR)/pretty.mk
include			$(MAKE_DIR)/version.mk

#######################################
# app and libs includes, order matters
#######################################
include			$(APP_DIR)/app.mk
include			$(LIBS_DIR)/libs.mk
include			$(TARGETS_DIR)/target.mk

#######################################
# make rules, toolchain options, versioning
#######################################
include			$(MAKE_DIR)/toolchain.mk
include			$(MAKE_DIR)/rules.mk
