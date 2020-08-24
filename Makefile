LIBS := libvncserver libavcodec libavformat libswscale libavutil

CFLAGS  := $(CFLAGS) $(shell pkg-config --cflags $(LIBS))
OBJ_DIR := .obj
BIN_DIR := bin

.PHONY: all
all: bin/hello

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(dir $@) && $(CC) $(CPPFLAGS) $(CFLAGS) $(TARGET_ARCH) -c -o $@ $<

$(BIN_DIR)/hello: $(addprefix $(OBJ_DIR)/,hello.o av/frame_iterator.o frame_iterator.o)
	mkdir -p $(dir $@) && $(CC) $(LDFLAGS) $(TARGET_ARCH) $^ -lstdc++ $(shell pkg-config --libs $(LIBS)) $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean
clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)
