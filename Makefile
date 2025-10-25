CC = gcc
CFLAGS = -Wall -Wextra -O2 -Isrc

VERSION = 1.0.0
CFLAGS += -DVERSION=\"$(VERSION)\"

MUSIC_DIR = $(HOME)/music
CFLAGS += -DMUSIC_DIR=\"$(MUSIC_DIR)\"
CFLAGS += -DLYRICS_DIR=\"$(HOME)/.lyrics\"

HAVE_TAGLIB := $(shell pkg-config --exists taglib && echo yes || echo no)

ifeq ($(HAVE_TAGLIB),no)
  $(warning "TagLib not found. Building without metadata support. Filename parsing will be used.")
else
  CFLAGS += $(shell pkg-config --cflags taglib_c)
  CFLAGS += -DWITH_TAGLIB
  LDFLAGS += $(shell pkg-config --libs taglib_c)
endif

TARGET = verse

SRC_DIR = src
OBJ_DIR = obj
PREFIX ?= /usr/local
INSTALL_DIR = $(PREFIX)/bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

install: all
	install -Dm755 $(TARGET) $(INSTALL_DIR)/$(TARGET)
	@echo "Installed $(TARGET) to $(INSTALL_DIR)"

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

clean\ install: clean install

uninstall:
	rm -f $(INSTALL_DIR)/$(TARGET)
	@echo "Uninstalled $(TARGET) from $(INSTALL_DIR)"

.PHONY: all clean install uninstall clean\ install
