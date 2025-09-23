# Gitea Repository Manager Makefile
# This Makefile manages the Gitea repository synchronization setup

.PHONY: all install test run sync push clean help cron-setup

# Default target
all: install

# Virtual environment setup
venv:
	python3 -m venv venv
	venv/bin/pip install --upgrade pip
	venv/bin/pip install -r requirements.txt

# Install dependencies
install: venv

# Test the script (setup only)
test: install
	venv/bin/python gitea_manager.py

# Run the script (setup only)
run: install
	venv/bin/python gitea_manager.py

# Sync and push all repositories
sync: install
	venv/bin/python gitea_manager.py --push --sync-only

# Push all repositories to Gitea
push: install
	venv/bin/python gitea_manager.py --push --sync-only

# Setup cron job for daily sync
cron-setup:
	@echo "Setting up daily cron job..."
	@echo "0 2 * * * /projects/gitea_sync.sh" | crontab -
	@echo "Cron job added: Daily sync at 2:00 AM"

# Clean virtual environment
clean:
	rm -rf venv

# Help target
help:
	@echo "Available targets:"
	@echo "  all        - Install dependencies (default)"
	@echo "  install    - Create virtual environment and install dependencies"
	@echo "  test       - Run the Gitea manager script (setup only)"
	@echo "  run        - Run the Gitea manager script (setup only)"
	@echo "  sync       - Sync and push all repositories to Gitea"
	@echo "  push       - Push all repositories to Gitea"
	@echo "  cron-setup - Setup daily cron job for automatic sync"
	@echo "  clean      - Remove virtual environment"
	@echo "  help       - Show this help message"
