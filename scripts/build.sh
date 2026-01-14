#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

# Export user/group ID for correct file permissions
export USER_ID=$(id -u)
export GROUP_ID=$(id -g)

case "${1:-dev}" in
    dev)
        echo "Starting development shell..."
        docker compose run --rm dev
        ;;
    build)
        echo "Building project..."
        docker compose build build
        ;;
    test)
        echo "Running tests..."
        docker compose run --rm test
        ;;
    coverage)
        echo "Running tests with coverage..."
        docker compose run --rm coverage
        echo "Coverage report: coverage/index.html"
        ;;
    lint)
        echo "Running linter..."
        docker compose run --rm lint
        ;;
    clean)
        echo "Cleaning build artifacts..."
        docker compose down -v
        rm -rf build coverage
        ;;
    *)
        echo "Usage: $0 {dev|build|test|coverage|lint|clean}"
        exit 1
        ;;
esac