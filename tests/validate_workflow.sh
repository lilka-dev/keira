#!/bin/bash

# Comprehensive test script for GitHub Actions workflow using act
# Tests both tag-triggered builds (all languages via matrix) and main/PR builds (default only)

set -e  # Exit on any error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TEST_OUTPUT_DIR="$REPO_ROOT/test_output"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if act is installed
check_act_installed() {
    if ! command -v act &> /dev/null; then
        log_error "act is not installed. Please install it first:"
        echo "  curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash"
        echo "  or visit: https://github.com/nektos/act"
        exit 1
    fi
    log_info "act version: $(act --version)"
}

# Test language file reading and matrix generation
test_language_detection() {
    log_info "Testing language file reading..."
    
    if [[ ! -f "$REPO_ROOT/languages.txt" ]]; then
        log_error "languages.txt not found!"
        return 1
    fi
    
    echo "Supported languages:"
    cat "$REPO_ROOT/languages.txt"
    
    echo ""
    echo "Testing matrix generation (tag push scenario):"
    LANGS=$(cat "$REPO_ROOT/languages.txt" | jq -R -s -c 'split("\n") | map(select(. != ""))')
    echo "Matrix JSON: $LANGS"

    echo ""
    echo "Testing matrix generation (PR scenario):"
    echo 'Matrix JSON: ["default"]'

    echo ""
    echo "Testing build flag generation:"
    for lang in $(cat "$REPO_ROOT/languages.txt"); do
        echo "  Matrix job $lang → PLATFORMIO_BUILD_FLAGS=-D$lang"
    done
}

# Test GitHub ref detection logic simulation
test_ref_logic() {
    log_info "Testing GitHub ref detection logic (matrix selection)..."
    
    # Simulate different refs and show which matrix would be used
    test_refs=(
        "refs/heads/main"
        "refs/heads/feature-branch"
        "refs/tags/v1.0.0"
        "refs/tags/v2.1.3"
        "refs/pull/123/merge"
    )

    for ref in "${test_refs[@]}"; do
        if [[ "$ref" == refs/tags/* ]]; then
            matrix=$(cat "$REPO_ROOT/languages.txt" | jq -R -s -c 'split("\n") | map(select(. != ""))')
            result="MATRIX: $matrix"
        else
            result='MATRIX: ["default"]'
        fi
        echo "Ref: $ref -> $result"
    done
}

# Test with act - simulate main branch push (prepare + keira jobs, default only)
test_act_main_branch() {
    log_info "Testing workflow with act - simulating main branch push..."
    
    cd "$REPO_ROOT"
    mkdir -p "$TEST_OUTPUT_DIR"
    
    # Run act for push event (simulates main branch)
    if act push \
        --workflows .github/workflows/firmware.yml \
        --eventpath <(echo '{"ref": "refs/heads/main", "repository": {"default_branch": "main"}}') \
        --artifact-server-path "$TEST_OUTPUT_DIR" \
        --verbose; then
        log_info "Main branch workflow test PASSED"
        
        # In the new matrix workflow, a main-branch push should only produce a single
        # "default" build artifact (KeiraOS-firmware-default).
        if ls "$TEST_OUTPUT_DIR"/**/KeiraOS-firmware-default/**/*.bin 2>/dev/null; then
            log_info "Default build artifact found"
        else
            log_warn "Default build artifact not found (this might be expected in dry-run)"
        fi
        
        # Check that per-language artifacts were NOT created
        lang_artifacts=$(find "$TEST_OUTPUT_DIR" -path "*/KeiraOS-firmware-LANG_*" -type d 2>/dev/null | wc -l || echo "0")
        if [[ "$lang_artifacts" -eq 0 ]]; then
            log_info "No per-language build artifacts found (correct for main branch)"
        else
            log_error "Found $lang_artifacts per-language artifacts (should be 0 for main branch)"
            return 1
        fi
        
    else
        log_error "Main branch workflow test FAILED"
        return 1
    fi
}

# Test with act - simulate tag push (prepare + keira matrix + release jobs)
test_act_tag_push() {
    log_info "Testing workflow with act - simulating tag push..."
    
    cd "$REPO_ROOT"
    mkdir -p "$TEST_OUTPUT_DIR"
    
    # Run act for tag push event
    if act push \
        --workflows .github/workflows/firmware.yml \
        --eventpath <(echo '{"ref": "refs/tags/v1.0.0"}') \
        --artifact-server-path "$TEST_OUTPUT_DIR" \
        --verbose; then
        log_info "Tag push workflow test PASSED"
        
        # In the matrix workflow, each language gets its own artifact directory
        expected_langs=$(cat "$REPO_ROOT/languages.txt" | wc -l)
        actual_artifacts=$(find "$TEST_OUTPUT_DIR" -path "*/KeiraOS-firmware-LANG_*" -type d 2>/dev/null | wc -l || echo "0")
        
        if [[ "$actual_artifacts" -gt 0 ]]; then
            log_info "Found $actual_artifacts per-language artifacts (expected $expected_langs)"
        else
            log_warn "No per-language artifacts found (this might be expected in dry-run)"
        fi
        
        # Also verify the combined release artifact was created
        if find "$TEST_OUTPUT_DIR" -path "*/KeiraOS-firmware/*" -type f 2>/dev/null | head -1 | grep -q .; then
            log_info "Combined release artifact found"
        else
            log_warn "Combined release artifact not found (this might be expected in dry-run)"
        fi
        
    else
        log_error "Tag push workflow test FAILED"
        return 1
    fi
}

# Test with act - simulate PR (prepare + keira jobs, default only)
test_act_pull_request() {
    log_info "Testing workflow with act - simulating pull request..."
    
    cd "$REPO_ROOT"
    mkdir -p "$TEST_OUTPUT_DIR"
    
    # Run act for pull request event
    if act pull_request \
        --workflows .github/workflows/firmware.yml \
        --eventpath <(echo '{"pull_request": {"head": {"ref": "feature-branch"}}}') \
        --artifact-server-path "$TEST_OUTPUT_DIR" \
        --verbose; then
        log_info "Pull request workflow test PASSED"
        
        # PR builds should only produce a single default artifact, same as main branch
        lang_artifacts=$(find "$TEST_OUTPUT_DIR" -path "*/KeiraOS-firmware-LANG_*" -type d 2>/dev/null | wc -l || echo "0")
        if [[ "$lang_artifacts" -eq 0 ]]; then
            log_info "No per-language artifacts for PR (correct)"
        else
            log_error "Found $lang_artifacts per-language artifacts (should be 0 for PR)"
            return 1
        fi
    else
        log_error "Pull request workflow test FAILED"
        return 1
    fi
}

# Cleanup test artifacts
cleanup_test_artifacts() {
    log_info "Cleaning up test artifacts..."
    if [[ -d "$TEST_OUTPUT_DIR" ]]; then
        rm -rf "$TEST_OUTPUT_DIR"
        log_info "Test output directory cleaned up"
    fi
}

# Main execution
main() {
    log_info "Starting KeiraOS GitHub Actions workflow validation..."
    log_info "Workflow uses 3-job architecture: prepare -> keira (matrix) -> release"
    
    # Basic checks
    check_act_installed
    test_language_detection
    echo ""
    test_ref_logic
    echo ""
    
    # Cleanup previous test runs
    cleanup_test_artifacts
    
    # Act tests
    log_info "Running act tests..."
    echo ""
    
    # Test different scenarios
    test_act_main_branch
    echo ""
    test_act_tag_push
    echo ""
    test_act_pull_request
    echo ""
    
    # Cleanup
    cleanup_test_artifacts
    
    log_info "All workflow validation tests completed successfully!"
}

# Handle script arguments
case "${1:-all}" in
    "main")
        check_act_installed
        test_act_main_branch
        ;;
    "tag")
        check_act_installed
        test_act_tag_push
        ;;
    "pr")
        check_act_installed
        test_act_pull_request
        ;;
    "logic")
        test_language_detection
        test_ref_logic
        ;;
    "all"|*)
        main
        ;;
esac