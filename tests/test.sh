#!/bin/bash

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

total=0
partial=0
failed=0

test_file() {
    local file="$1"
    local expected="$2"

    ((total++))
    printf "checking %-45s... " "$file"

    local output
    output=$(../vmasm "$file" -run 2>&1)

    IFS=',' read -r -a words < <(echo "$expected")
    local count=0
    local total_words=${#words[@]}

    for word in "${words[@]}"; do
        if [[ "$output" =~ "$word" ]]; then
            ((count++))
        fi
    done
    
    if [ "$count" -eq "$total_words" ]; then
        printf "[ ${GREEN}OK${NC} ]\n"
    elif [ "$count" -gt 0 ]; then
        printf "[ ${YELLOW}PARTIAL${NC} ]\n"
        ((partial++))
    else
        printf "[ ${RED}FAIL${NC} ]\n"
        ((failed++))
    fi
}

echo "========================================"
echo "Starting VMASM Test Suite"
echo "========================================"

test_file "./errors/EXCEPTION_ILLEGAL_WRITE_str.asm" "EXCEPTION_ILLEGAL_WRITE,in str()"
test_file "./errors/EXCEPTION_ILLEGAL_WRITE_mov.asm" "EXCEPTION_ILLEGAL_WRITE,in mov()"
test_file "./errors/EXCEPTION_ILLEGAL_WRITE_ld.asm"  "EXCEPTION_ILLEGAL_WRITE,in ld()"

echo "========================================"
echo "Test Summary:"
echo -e "Total: $total | Passed: $((total - partial - failed)) | ${YELLOW}Partial: $partial${NC} | ${RED}Failed: $failed${NC}"
echo "========================================"

