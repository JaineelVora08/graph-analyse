.PHONY: all clean test

BUILD_DIR := build

all:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR) --config Release

clean:
	cmake -E rm -rf $(BUILD_DIR)

test: all
	ctest --test-dir $(BUILD_DIR) --output-on-failure
