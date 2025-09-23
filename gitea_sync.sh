#!/bin/bash
# Gitea Sync Wrapper Script for Cron
# This script runs the Gitea manager with proper environment setup

# Set working directory
cd /projects

# Activate virtual environment and run sync with push
/projects/venv/bin/python /projects/gitea_manager.py --push --sync-only

# Log the execution
echo "$(date): Gitea sync completed" >> /projects/gitea_sync.log
