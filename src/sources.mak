UTENTE_SRCS = $(wildcard $(SRC_DIR)/utente/*.c)
UTENTE_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(UTENTE_SRCS))


LAVAGNA_SRCS = $(wildcard $(SRC_DIR)/lavagna/*.c)
LAVAGNA_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(LAVAGNA_SRCS))
