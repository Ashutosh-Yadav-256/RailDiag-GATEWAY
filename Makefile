BUILD_DIR ?= build

.PHONY: build test unit-test integration-test clean run

build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(BUILD_DIR) --parallel

unit-test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

integration-test: build
	PYTHONPATH=python RAILDIAG_BINARY="$(CURDIR)/$(BUILD_DIR)/raildiag-gateway" python3 -m unittest discover -s tests -v

test: unit-test integration-test

run: build
	./$(BUILD_DIR)/raildiag-gateway --bind 127.0.0.1 --port 9100 --log diagnostics.csv

clean:
	rm -rf $(BUILD_DIR)
