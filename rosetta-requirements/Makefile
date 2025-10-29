# Makefile for Requirements Tracking Tools Evaluation Environment
# 
# This Makefile follows strict conventions:
# - All variables use full escape syntax: ${VAR}
# - Targets are actual files/folders where possible
# - All Python operations use venv/bin/ paths explicitly
# - No system pip or Python package installation

PYTHON := python3
VENV_DIR := venv
VENV_BIN := ${VENV_DIR}/bin
PIP := ${VENV_BIN}/pip
PYTHON_VENV := ${VENV_BIN}/python
REQUIREMENTS := requirements.txt

.PHONY: help install clean

# Default target
help:
	@echo "Requirements Tracking Tools Evaluation Environment"
	@echo ""
	@echo "Available targets:"
	@echo "  install    - Create venv and install all dependencies"
	@echo "  venv       - Create Python virtual environment only"
	@echo "  clean      - Remove venv and generated files"
	@echo "  help       - Show this help message"

# Install target depends on venv existing
install: ${VENV_DIR}
	${PIP} install --upgrade pip
	${PIP} install -r ${REQUIREMENTS}
	@echo ""
	@echo "Installation complete!"
	@echo "Activate the virtual environment with: source ${VENV_BIN}/activate"

# Create virtual environment (actual directory target)
${VENV_DIR}:
	${PYTHON} -m venv ${VENV_DIR}
	@echo "Virtual environment created at ${VENV_DIR}"

# Alias target for convenience
venv: ${VENV_DIR}

# Clean up generated files and venv
clean:
	rm -rf ${VENV_DIR}
	find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete 2>/dev/null || true
	find . -type d -name "*.egg-info" -exec rm -rf {} + 2>/dev/null || true
	@echo "Cleanup complete"

