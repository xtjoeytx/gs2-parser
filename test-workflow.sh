#!/bin/bash
# Test GitHub Actions workflow locally using 'act'

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if act is installed
if ! command -v act &> /dev/null; then
    echo -e "${RED}Error: 'act' is not installed${NC}"
    echo ""
    echo "Install act with:"
    echo "  macOS:   brew install act"
    echo "  Linux:   curl -s https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash"
    echo "  Windows: choco install act-cli"
    echo ""
    echo "Visit: https://github.com/nektos/act"
    exit 1
fi

# Function to print usage
usage() {
    echo -e "${BLUE}GitHub Actions Local Testing Tool${NC}"
    echo ""
    echo "Usage: $0 [command] [options]"
    echo ""
    echo "Commands:"
    echo "  list            - List all available workflows and jobs"
    echo "  quick           - Quick test (Ubuntu Release build only)"
    echo "  full            - Full matrix test (all OS and build types)"
    echo "  ubuntu          - Test Ubuntu builds (Debug + Release)"
    echo "  macos           - Test macOS builds (Debug + Release)"
    echo "  windows         - Test Windows builds (Debug + Release)"
    echo "  debug           - Test Debug builds (all platforms)"
    echo "  release         - Test Release builds (all platforms)"
    echo "  dry-run         - Show what would run without executing"
    echo "  custom          - Run custom act command (pass additional args)"
    echo ""
    echo "Options:"
    echo "  -v, --verbose   - Verbose output"
    echo "  -h, --help      - Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 quick                    # Fast test on Ubuntu Release"
    echo "  $0 ubuntu -v                # Test Ubuntu with verbose output"
    echo "  $0 custom -j build-and-test --matrix os:macos-latest"
    exit 0
}

# Parse global options
VERBOSE=""
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            ;;
        -v|--verbose)
            VERBOSE="-v"
            shift
            ;;
        *)
            break
            ;;
    esac
done

COMMAND=${1:-quick}

echo -e "${GREEN}Testing GitHub Actions workflow locally...${NC}"
echo ""

case $COMMAND in
    list)
        echo -e "${YELLOW}Available workflows and jobs:${NC}"
        act -l
        ;;

    quick)
        echo -e "${YELLOW}Running quick test (Ubuntu Release)...${NC}"
        act --rm push -j build-and-test \
            --matrix os:ubuntu-latest \
            --matrix build_type:Release \
            -W .github/workflows/ci.yml \
            $VERBOSE
        ;;

    full)
        echo -e "${YELLOW}Running full matrix test (this will take a while)...${NC}"
        echo -e "${RED}Warning: This will test all OS/build_type combinations${NC}"
        read -p "Continue? (y/N) " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            act --rm --bind push -j build-and-test \
                -W .github/workflows/ci.yml \
                $VERBOSE
        else
            echo "Cancelled."
            exit 0
        fi
        ;;

    ubuntu)
        echo -e "${YELLOW}Testing Ubuntu builds (Debug + Release)...${NC}"
        echo "Running Release build..."
        act --rm --bind push -j build-and-test \
            --matrix os:ubuntu-latest \
            --matrix build_type:Release \
            -W .github/workflows/ci.yml \
            $VERBOSE

        echo ""
        echo "Running Debug build..."
        act --rm --bind push -j build-and-test \
            --matrix os:ubuntu-latest \
            --matrix build_type:Debug \
            -W .github/workflows/ci.yml \
            $VERBOSE
        ;;

    macos)
        echo -e "${YELLOW}Testing macOS builds (Debug + Release)...${NC}"
        echo "Running Release build..."
        act --rm --bind push -j build-and-test \
            --matrix os:macos-latest \
            --matrix build_type:Release \
            -W .github/workflows/ci.yml \
            $VERBOSE

        echo ""
        echo "Running Debug build..."
        act --rm --bind push -j build-and-test \
            --matrix os:macos-latest \
            --matrix build_type:Debug \
            -W .github/workflows/ci.yml \
            $VERBOSE
        ;;

    windows)
        echo -e "${YELLOW}Testing Windows builds (Debug + Release)...${NC}"
        echo "Running Release build..."
        act --rm --bind push -j build-and-test \
            --matrix os:windows-latest \
            --matrix build_type:Release \
            -W .github/workflows/ci.yml \
            $VERBOSE

        echo ""
        echo "Running Debug build..."
        act --rm --bind push -j build-and-test \
            --matrix os:windows-latest \
            --matrix build_type:Debug \
            -W .github/workflows/ci.yml \
            $VERBOSE
        ;;

    debug)
        echo -e "${YELLOW}Testing Debug builds (all platforms)...${NC}"
        act --rm --bind push -j build-and-test \
            --matrix build_type:Debug \
            -W .github/workflows/ci.yml \
            $VERBOSE
        ;;

    release)
        echo -e "${YELLOW}Testing Release builds (all platforms)...${NC}"
        act --rm --bind push -j build-and-test \
            --matrix build_type:Release \
            -W .github/workflows/ci.yml \
            $VERBOSE
        ;;

    dry-run)
        echo -e "${YELLOW}Dry run - showing what would execute...${NC}"
        act -n -W .github/workflows/ci.yml
        ;;

    custom)
        shift  # Remove 'custom' from args
        echo -e "${YELLOW}Running custom act command...${NC}"
        echo "Command: act --rm --bind push -W .github/workflows/ci.yml $@"
        act --rm --bind push -W .github/workflows/ci.yml "$@"
        ;;

    *)
        echo -e "${RED}Error: Unknown command '$COMMAND'${NC}"
        echo ""
        usage
        ;;
esac

echo ""
echo -e "${GREEN}Done!${NC}"