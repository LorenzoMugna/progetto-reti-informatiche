GLOBAL_HEADERS = $(wildcard $(SRC_DIR)/include/*.h)
LIB_SRCS = $(wildcard $(SRC_DIR)/lib/*.c)
LIB_HEADERS = $(wildcard $(SRC_DIR)/lib/*.h)
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(LIB_SRCS))

# ---- REGOLA DI COMPILAZIONE OGGETTI (Modulo Libreria) ----
$(BUILD_DIR)/lib/%.o : $(SRC_DIR)/lib/%.c $(LIB_HEADERS) $(GLOBAL_HEADERS)
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR) $(CFLAGS)

# ---- REGOLA DI COMPILAZIONE OGGETTI (Modulo Utente) ----
UTENTE_SRCS = $(wildcard $(SRC_DIR)/utente/*.c)
UTENTE_HEADERS = $(wildcard $(SRC_DIR)/utente/*.h)
UTENTE_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(UTENTE_SRCS))

$(BUILD_DIR)/utente/%.o : $(SRC_DIR)/utente/%.c $(UTENTE_HEADERS) $(GLOBAL_HEADERS)
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR) $(CFLAGS)

# ---- REGOLA DI COMPILAZIONE OGGETTI (Modulo Lavagna) ----
LAVAGNA_SRCS = $(wildcard $(SRC_DIR)/lavagna/*.c)
LAVAGNA_HEADERS = $(wildcard $(SRC_DIR)/lavagna/*.h)	
LAVAGNA_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(LAVAGNA_SRCS))


$(BUILD_DIR)/lavagna/%.o : $(SRC_DIR)/lavagna/%.c $(LAVAGNA_HEADERS) $(GLOBAL_HEADERS)
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ -I$(INCLUDE_DIR) $(CFLAGS)