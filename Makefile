CFLAGS = -Wall -Wextra -Wpedantic -std=c11
CC = gcc
LDFLAGS =
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

include $(SRC_DIR)/sources.mak



# ---- REGOLA DI COMPILAZIONE OGGETTI ----
$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR) $(CFLAGS)


# ---- ESEGUIBILI IN CARTELLA PRINCIPALE ----
utente: build/utente/utente
	ln -sf $< $@

lavagna: build/lavagna/lavagna
	ln -sf $< $@

# ---- ESEGUIBILI IN BUILD ----
build/utente/utente: $(UTENTE_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@ $(LDFLAGS)

build/lavagna/lavagna: $(LAVAGNA_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@ $(LDFLAGS)

# ---- DEFINIZIONE PHONY ----
.PHONY: all clean

clean:
	rm -rf $(BUILD_DIR) utente lavagna

all: utente lavagna