PYTHON:=python3
VENV:=venv

.PHONY: all help

all: help

help:
	@echo "Targets:"
	@echo "  venv                 - Create Python virtual environment"
	@echo "  mypy                 - Run mypy type checks on tools"
	@echo "  tools                - Build/check Python tools"

# Create a real file target for the venv directory
${VENV}:
	${PYTHON} -m venv ${VENV}

mypy: ${VENV}
	${VENV}/bin/pip install --upgrade pip mypy
	${VENV}/bin/mypy tools

tools: mypy
	@echo "Tools are type-checked."


