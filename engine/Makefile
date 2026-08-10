GO ?= go
PYTHON ?= python3
BUILD_DIR ?= build
PREFIX ?= $(HOME)/.local
BINARY := $(BUILD_DIR)/bin/fileman-engine
ATLAS_INSTALL_DIR := $(DESTDIR)$(PREFIX)/share/fileman-engine/service-atlas

.PHONY: build test race vet bench cross-build docs docs-test docs-check check install install-m4-dogfood uninstall-m4-dogfood

build:
	mkdir -p $(dir $(BINARY))
	$(GO) build -trimpath -o $(BINARY) ./cmd/fileman-engine

test:
	$(GO) test ./...

race:
	$(GO) test -race ./...

vet:
	$(GO) vet ./...

bench:
	$(GO) test -run '^$$' -bench . -benchmem ./...

cross-build:
	GOOS=linux GOARCH=amd64 $(GO) build ./...
	GOOS=windows GOARCH=amd64 $(GO) build ./...

docs:
	$(PYTHON) tools/generate_library_docs.py

docs-test:
	$(PYTHON) -m unittest discover -s tools -p 'test_*.py'

docs-check: docs-test
	$(PYTHON) tools/generate_library_docs.py --check

check: test race vet cross-build docs-check

install: build docs-check
	install -d "$(DESTDIR)$(PREFIX)/bin" "$(ATLAS_INSTALL_DIR)"
	install -m 0755 "$(BINARY)" "$(DESTDIR)$(PREFIX)/bin/fileman-engine"
	cp -R docs/library/. "$(ATLAS_INSTALL_DIR)/"
	@echo "Installed development binary: $(DESTDIR)$(PREFIX)/bin/fileman-engine"
	@echo "Installed service atlas: $(ATLAS_INSTALL_DIR)/index.html"

install-m4-dogfood:
	test -n "$(ROOT)"
	/bin/sh tools/install_macos_m4_dogfood.sh "$(ROOT)"

uninstall-m4-dogfood:
	/bin/sh tools/uninstall_macos_m4_dogfood.sh
