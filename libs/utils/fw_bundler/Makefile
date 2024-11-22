UNAME_S = $(shell uname -s)

SRC_DIR			=	src
TINYAES_DIR		=	$(SRC_DIR)/tinyaes
ECDSAUTIL_DIR	=	$(SRC_DIR)/ecdsautil
UF2_DIR			=	$(SRC_DIR)/uf2

RESET_COLOR   	=	\033[m
MAGENTA    		=	\033[1;35m
CYAN       		=	\033[1;36m

AR				=	ar

ARGP	?= /opt/homebrew/Cellar/argp-standalone/1.3
CFLAGS 	= -Wno-unused-command-line-argument

all: src/tinyaes/aes.o fw_bundler

fw_bundler: src/fw_bundler.c src/tinyaes/aes.o src/ecdsautil/sha256.o src/ecdsautil/ecdsa.o src/ecdsautil/libuecc/src/ec25519.o src/ecdsautil/libuecc/src/ec25519_gf.o
	@printf "\tCC\t%b\n" "$(MAGENTA)$^$(CYAN) -> $(MAGENTA)$(@F)$(RESET_COLOR)";
ifeq ($(UNAME_S),Darwin)
	@gcc -w -I$(UF2_DIR) -I$(ECDSAUTIL_DIR)/ -I$(ECDSAUTIL_DIR)/libuecc/include/ -I./src -I/usr/include -I$(ARGP)/include $(ARGP)/lib/libargp.a $(CFLAGS) -pthread -Wall -Werror $^ -o $@ 
else 
	@gcc -w -I$(UF2_DIR) -I$(ECDSAUTIL_DIR)/ -I$(ECDSAUTIL_DIR)/libuecc/include/ -I./src -I/usr/include  -pthread -Wall -Werror $^ -o $@ 
endif

src/tinyaes/aes.o: src/tinyaes/aes.c
	@printf "\tCC\t%b\n" "$(MAGENTA)$< $(CYAN) -> $(MAGENTA)$@ $(RESET_COLOR)";
	@gcc -I$(ECDSAUTIL_DIR)/ -c $< -DCBC=1 -o $@

src/ecdsautil/sha256.o: src/ecdsautil/sha256.c
	@printf "\tCC\t%b\n" "$(MAGENTA)$< $(CYAN) -> $(MAGENTA)$@ $(RESET_COLOR)";
	@gcc -I$(ECDSAUTIL_DIR)/ -I./$(ECDSAUTIL_DIR)/libuecc/include/  -c $< -DCBC=1 -o $@


src/ecdsautil/ecdsa.o: src/ecdsautil/ecdsa.c src/ecdsautil/sha256.c
	@printf "\tCC\t%b\n" "$(MAGENTA)$< $(CYAN) -> $(MAGENTA)$@ $(RESET_COLOR)";
	@gcc -I$(ECDSAUTIL_DIR)/ -I./$(ECDSAUTIL_DIR)/libuecc/include/  -c $< -DCBC=1 -o $@

src/ecdsautil/libuecc/src/ec25519_gf.o: src/ecdsautil/libuecc/src/ec25519_gf.c
	@printf "\tCC\t%b\n" "$(MAGENTA)$< $(CYAN) -> $(MAGENTA)$@ $(RESET_COLOR)";
	@gcc -I$(ECDSAUTIL_DIR)/ -I./$(ECDSAUTIL_DIR)/libuecc/include/ -c $< -DCBC=1 -o $@ 


src/ecdsautil/libuecc/src/ec25519.o: src/ecdsautil/libuecc/src/ec25519.c
	@printf "\tCC\t%b\n" "$(MAGENTA)$< $(CYAN) -> $(MAGENTA)$@ $(RESET_COLOR)";
	@gcc -I$(ECDSAUTIL_DIR)/ -I./$(ECDSAUTIL_DIR)/libuecc/include/ -c $< -DCBC=1 -o $@ 

clean:
	@echo "CLEAN"
	rm -f $(SRC_DIR)/*.o
	rm -f $(SRC_DIR)/*.a
	rm -f fw_bundler
	rm -f $(TINYAES_DIR)/*.o
	rm -f $(ECDSAUTIL_DIR)/*.o
	rm -f $(ECDSAUTIL_DIR)/ecdsautil/libuecc/*.o


