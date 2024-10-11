CXX = g++
CXXFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c++17
BUILD_DIR = build/debug
RELEASE_DIR = build/release
SRC_DIR = src

all: server client

server: $(BUILD_DIR)/server_main.o $(BUILD_DIR)/Server.o
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/server $(BUILD_DIR)/server_main.o $(BUILD_DIR)/Server.o

client: $(BUILD_DIR)/client_main.o $(BUILD_DIR)/Client.o
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/client $(BUILD_DIR)/client_main.o $(BUILD_DIR)/Client.o

release: CXXFLAGS += -O2
release: BUILD_DIR = $(RELEASE_DIR)
release: server client

$(BUILD_DIR)/server_main.o: $(SRC_DIR)/server_main.cpp $(SRC_DIR)/Server.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/server_main.cpp -o $(BUILD_DIR)/server_main.o

$(BUILD_DIR)/client_main.o: $(SRC_DIR)/client_main.cpp $(SRC_DIR)/Client.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/client_main.cpp -o $(BUILD_DIR)/client_main.o

$(BUILD_DIR)/Server.o: $(SRC_DIR)/Server.cpp $(SRC_DIR)/Server.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/Server.cpp -o $(BUILD_DIR)/Server.o

$(BUILD_DIR)/Client.o: $(SRC_DIR)/Client.cpp $(SRC_DIR)/Client.h | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/Client.cpp -o $(BUILD_DIR)/Client.o

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/server $(BUILD_DIR)/client $(RELEASE_DIR)/*.o $(RELEASE_DIR)/server $(RELEASE_DIR)/client
