CFLAGS = -Wall -Wextra -Wpedantic -Werror -g -O0
CC = gcc
LDFLAGS = -fsanitize=address
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = $(SRC_DIR)/include

include $(SRC_DIR)/sources.mak

# ---- DEFINIZIONE PHONY ----
all: utente lavagna

.PHONY: all clean

clean:
	rm -rf $(BUILD_DIR) utente lavagna



# ---- ESEGUIBILI IN CARTELLA PRINCIPALE ----
utente: build/utente/utente
	ln -sf $< $@

lavagna: build/lavagna/lavagna
	ln -sf $< $@

# ---- ESEGUIBILI IN BUILD ----
build/utente/utente: $(UTENTE_OBJS) $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@ $(LDFLAGS)

build/lavagna/lavagna: $(LAVAGNA_OBJS) $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@ $(LDFLAGS)
