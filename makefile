SERVER_DIR=vine-street
BIN_DIR=bin
BUILD_DIR=build
SRC_DIR=src

$(BIN_DIR)/ipa-website: $(BUILD_DIR)/ipa-website.o $(BUILD_DIR)/server.o | $(BIN_DIR)
	gcc -o $(BIN_DIR)/ipa-website $(BUILD_DIR)/ipa-website.o $(BUILD_DIR)/server.o
$(BUILD_DIR)/ipa-website.o: $(SRC_DIR)/ipa-website.c $(SRC_DIR)/ipa-website.h $(SERVER_DIR)/server.h | $(BUILD_DIR)
	gcc -o $(BUILD_DIR)/ipa-website.o -c $(SRC_DIR)/ipa-website.c -I $(SERVER_DIR)
$(BUILD_DIR)/server.o: $(SERVER_DIR)/server.c $(SERVER_DIR)/server.h | $(BUILD_DIR)
	gcc -o $(BUILD_DIR)/server.o -c $(SERVER_DIR)/server.c
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
$(BIN_DIR):
	mkdir -p $(BIN_DIR)
