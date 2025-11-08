#!/bin/bash

# Script to setup pre-commit hooks for the project
set -e

echo "Setting up pre-commit hooks..."

# Check if Python is installed
if ! command -v python3 &> /dev/null; then
    echo "Error: Python 3 is required but not installed."
    echo "Please install Python 3 and try again."
    exit 1
fi

# Check if pip is installed
if ! command -v pip3 &> /dev/null; then
    echo "Error: pip3 is required but not installed."
    echo "Please install pip3 and try again."
    exit 1
fi

# Install pre-commit
echo "Installing pre-commit..."
pip3 install --user pre-commit

# Install pre-commit hooks
echo "Installing pre-commit hooks..."
pre-commit install

# Install commit-msg hook
pre-commit install --hook-type commit-msg

# Generate secrets baseline if it doesn't exist
if [ ! -f .secrets.baseline ]; then
    echo "Generating secrets baseline..."
    detect-secrets scan > .secrets.baseline || echo "{}" > .secrets.baseline
fi

# Run pre-commit on all files to check setup
echo "Running pre-commit on all files to verify setup..."
pre-commit run --all-files || echo "Some checks failed. This is normal for first setup."

echo ""
echo "✓ Pre-commit hooks installed successfully!"
echo ""
echo "Usage:"
echo "  - Hooks will run automatically on 'git commit'"
echo "  - To run manually: pre-commit run --all-files"
echo "  - To update hooks: pre-commit autoupdate"
echo "  - To skip hooks (not recommended): git commit --no-verify"
echo ""

