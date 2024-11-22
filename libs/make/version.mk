#######################################
# GIT BUILD INFO
#######################################

GIT_COMMIT				:= $(shell git rev-parse --short HEAD || echo 'unknown')
GIT_BRANCH				:= $(shell echo $${WORKFLOW_BRANCH_OR_TAG-$$(git rev-parse --abbrev-ref HEAD || echo 'unknown')})
GIT_VERSION				:= $(shell git describe --tags --abbrev=0  || echo 'unknown')
GIT_VERSION_MAJOR		:= $(shell git describe --tags --abbrev=0 | cut -d'.' -f1 || echo '0')
GIT_VERSION_MINOR		:= $(shell git describe --tags --abbrev=0 | cut -d'.' -f2 || echo '0')
GIT_VERSION_PATCH		:= $(shell git describe --tags --abbrev=0 | cut -d'.' -f3 || echo '0')

GIT_DIRTY_BUILD := $(shell git diff --quiet ; echo $$?)
 
GIT_DIRTY_SUFFIX :=
ifeq ($(GIT_DIRTY_BUILD), 1)
	GIT_DIRTY_SUFFIX := _dirty
	CFLAGS += \
		-DGIT_DIRTY=1
endif

CFLAGS += -DGIT_COMMIT=\"$(GIT_COMMIT)\" 
CFLAGS += -DGIT_BRANCH=\"$(GIT_BRANCH)\" 
CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\" 
CFLAGS += -DGIT_VERSION_MAJOR=\$(GIT_VERSION_MAJOR)
CFLAGS += -DGIT_VERSION_MINOR=\$(GIT_VERSION_MINOR)
CFLAGS += -DGIT_VERSION_PATCH=\$(GIT_VERSION_PATCH)


DIST_SUFFIX	:= $(GIT_VERSION)$(GIT_DIRTY_SUFFIX)_$(GIT_COMMIT)
