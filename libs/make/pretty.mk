######################################
# prettify output
######################################

RESET_COLOR   = \033[m

BLUE       = \033[1;34m
YELLOW     = \033[1;33m
GREEN      = \033[1;32m
RED        = \033[1;31m
BLACK      = \033[1;30m
MAGENTA    = \033[1;35m
CYAN       = \033[1;36m
WHITE      = \033[1;37m

DBLUE      = \033[0;34m
DYELLOW    = \033[0;33m
DGREEN     = \033[0;32m
DRED       = \033[0;31m
DBLACK     = \033[0;30m
DMAGENTA   = \033[0;35m
DCYAN      = \033[0;36m
DWHITE     = \033[0;37m

BG_WHITE   = \033[47m
BG_RED     = \033[41m
BG_GREEN   = \033[42m
BG_YELLOW  = \033[43m
BG_BLUE    = \033[44m
BG_MAGENTA = \033[45m
BG_CYAN    = \033[46m

# Define the colors to be used in run_and_test
COM_COLOR   = $(DBLUE)
COM_COLOR2   = $(DBLUE)
OBJ_COLOR   = $(DCYAN)
OK_COLOR    = $(DGREEN)
ERROR_COLOR = $(DRED)
WARN_COLOR  = $(DYELLOW)
NO_COLOR    = $(RESET_COLOR)

OK_STRING    = "[OK]"
ERROR_STRING = "[ERROR]"
WARN_STRING  = "[WARNING]"

# A macro that will be used repeatedly.  Performs the command given,
# with colored output.  Uses the colors as defined above.
rwildcard	=	$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

define run_and_test
	mkdir -p $(@D)
	printf "\t\t%b\n" "$(COM_COLOR)$(COM_STRING)$(OBJ_COLOR)$< $(DMAGENTA) -> $(COM_COLOR2)  $(@F) $(NO_COLOR)";
	$(1) 2> $@.log; \
	RESULT=$$?; \
	printf "\t%b" "$(COM_COLOR)$(2) $(OBJ_COLOR)"; \
	if [ $$RESULT -ne 0 ]; then \
		printf "\t%-40b%b" "$@" "$(ERROR_COLOR)$(ERROR_STRING)$(NO_COLOR)\n"; \
	elif [ -s $@.log ]; then \
		printf "\t%-40b%b" "$@" "$(WARN_COLOR)$(WARN_STRING)$(NO_COLOR)\n"; \
	else  \
		printf "\t%-40b%b" "$(@F)" "$(OK_COLOR)$(OK_STRING)$(NO_COLOR)\n"; \
	fi; \
	cat $@.log; \
	rm -f $@.log; \
	exit $$RESULT
endef

define pretty_print
  printf "\t%b\t" "$(COM_COLOR)$(COM_STRING)$(OBJ_COLOR) $< $(DMAGENTA) -> $(COM_COLOR2)  $(@F) $(NO_COLOR) \r\n";
endef

define headers_include_all
	$(addprefix -I, $(dir $(wildcard $1/*/*/)))
endef

define sources_include_all
	$(call rwildcard,$1,*.c)
endef

define rust_include_all
	$(call rwildcard,$1,*.rs)
endef

check_defined = \
    $(strip $(foreach 1,$1, \
        $(call __check_defined,$1,$(strip $(value 2)))))
__check_defined = \
    $(if $(value $1),, \
      $(error Undefined $1$(if $2, ($2))))
