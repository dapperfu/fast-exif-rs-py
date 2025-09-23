# Gitea Repository Manager Makefile
# This Makefile manages the Gitea repository synchronization setup

.PHONY: all install test run clean help

# Default target
all: install

# Virtual environment setup
venv:
	python3 -m venv venv
	venv/bin/pip install --upgrade pip
	venv/bin/pip install -r requirements.txt

# Install dependencies
install: venv

# Test the script
test: install
	venv/bin/python gitea_manager.py

# Run the script
run: install
	venv/bin/python gitea_manager.py

# Clean virtual environment
clean:
	rm -rf venv

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Install dependencies (default)"
	@echo "  install - Create virtual environment and install dependencies"
	@echo "  test    - Run the Gitea manager script"
	@echo "  run     - Run the Gitea manager script"
	@echo "  clean   - Remove virtual environment"
	@echo "  help    - Show this help message"
