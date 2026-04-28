include EMU2000/config.mk

# $(shell rm -rf $SCRIPT_PATH/output/mynand)
# # 创建mynand 指向release
# $(shell ln -sf $(ROOT_OUTPUT_DIR)/$(PLATFORM)/release $(ROOT_OUTPUT_DIR)/mynand)

# $(shell cp -p $(LIB_61850) $(LIBRARY_PATH)/)

# ifeq ($(PLATFORM), desk)
# $(shell cp -p $(ROOT_DIR)/lib/desk/libiec* $(LIBRARY_PATH)/)
# endif

all:
	@mkdir -p $(ROOT_OUTPUT_DIR)/
	@rm -rf $(ROOT_OUTPUT_DIR)/mynand
	@ln -sf $(ROOT_OUTPUT_DIR)/$(PLATFORM)/release $(ROOT_OUTPUT_DIR)/mynand
	@cp -p $(LIB_61850) $(LIBRARY_PATH)/
	@cp -p $(ROOT_DIR)/lib/desk/libiec* $(LIBRARY_PATH)/

	@echo "Makefile absolute directory: $(SRC_ROOT_DIR)"
	@echo "Platform: $(PLATFORM)"
	@echo "Binary path: $(BINARY_PATH)"
	@echo "Library path: $(LIBRARY_PATH)"
	@echo "Object path: $(OBJECT_PATH)"
