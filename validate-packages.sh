#!/bin/bash
# Package validation script for OpenAuto

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🔍 OpenAuto Package Validator${NC}"
echo "=================================="

# Function to validate package contents
validate_package() {
    local package_path=$1
    local package_type=$2
    
    echo -e "${BLUE}📦 Validating $package_type package: $(basename "$package_path")${NC}"
    
    if [ ! -f "$package_path" ]; then
        echo -e "${RED}❌ Package not found: $package_path${NC}"
        return 1
    fi
    
    # Check package info
    echo -e "${YELLOW}ℹ️  Package Information:${NC}"
    dpkg-deb --info "$package_path" | grep -E "Package|Version|Architecture|Depends"
    
    echo -e "${YELLOW}📂 Package Contents:${NC}"
    dpkg-deb --contents "$package_path" | head -20
    
    # Validate expected files for different package types
    local expected_files=()
    local missing_files=()
    
    if [[ "$package_type" == "debug" ]]; then
        expected_files=(
            "./opt/openauto-debug/autoapp"
            "./opt/openauto-debug/btservice"
            "./etc/openauto-debug/openauto.conf"
            "./etc/openauto-debug/logger.conf"
            "./etc/openauto-debug/services.conf"
            "./etc/systemd/system/openauto-debug.service"
            "./etc/systemd/system/openauto-btservice-debug.service"
            "./etc/logrotate.d/openauto-debug"
        )
    else
        expected_files=(
            "./opt/openauto/autoapp"
            "./opt/openauto/btservice"
            "./etc/openauto/openauto.conf"
            "./etc/openauto/logger.conf"
            "./etc/openauto/services.conf"
            "./etc/systemd/system/openauto.service"
            "./etc/systemd/system/openauto-btservice.service"
            "./etc/logrotate.d/openauto"
        )
    fi
    
    echo -e "${YELLOW}🔍 Checking required files...${NC}"
    for file in "${expected_files[@]}"; do
        if dpkg-deb --contents "$package_path" | grep -q "$(echo $file | sed 's|^\./||')"; then
            echo -e "  ${GREEN}✅ $file${NC}"
        else
            echo -e "  ${RED}❌ $file${NC}"
            missing_files+=("$file")
        fi
    done
    
    # Check package scripts
    echo -e "${YELLOW}📜 Checking package scripts...${NC}"
    local script_dir=$(mktemp -d)
    dpkg-deb --control "$package_path" "$script_dir"
    
    for script in preinst postinst prerm postrm; do
        if [ -f "$script_dir/$script" ]; then
            echo -e "  ${GREEN}✅ $script script present${NC}"
            # Check if script is executable
            if [ -x "$script_dir/$script" ]; then
                echo -e "    ${GREEN}✅ Executable${NC}"
            else
                echo -e "    ${RED}❌ Not executable${NC}"
            fi
        else
            echo -e "  ${YELLOW}⚠️  $script script missing (optional)${NC}"
        fi
    done
    
    rm -rf "$script_dir"
    
    # Summary
    if [ ${#missing_files[@]} -eq 0 ]; then
        echo -e "${GREEN}✅ $package_type package validation passed${NC}"
        return 0
    else
        echo -e "${RED}❌ $package_type package validation failed${NC}"
        echo -e "${RED}Missing files:${NC}"
        for file in "${missing_files[@]}"; do
            echo -e "  ${RED}• $file${NC}"
        done
        return 1
    fi
}

# Function to test package installation (dry run)
test_package_install() {
    local package_path=$1
    local package_type=$2
    
    echo -e "${BLUE}🧪 Testing $package_type package installation (dry run)${NC}"
    
    # Simulate installation
    echo -e "${YELLOW}🔍 Checking dependencies...${NC}"
    if dpkg-deb --info "$package_path" | grep -q "Depends:"; then
        local deps=$(dpkg-deb --info "$package_path" | grep "Depends:" | cut -d: -f2)
        echo -e "  Dependencies: $deps"
        
        # Check if we can satisfy dependencies
        echo -e "${YELLOW}📋 Dependency check:${NC}"
        apt-cache policy $(echo "$deps" | tr ',' '\n' | grep -o '[a-zA-Z0-9.-]*' | head -10) 2>/dev/null | grep -E "Candidate|Installed" || true
    fi
}

# Main validation
PACKAGE_DIR="packages"
VALIDATION_PASSED=true

if [ ! -d "$PACKAGE_DIR" ]; then
    echo -e "${RED}❌ Package directory not found: $PACKAGE_DIR${NC}"
    echo "Run ./build-packages.sh first to create packages"
    exit 1
fi

# Find and validate packages
RELEASE_PACKAGE=$(find "$PACKAGE_DIR" -name "*openauto-modern_*.deb" ! -name "*debug*" | head -1)
DEBUG_PACKAGE=$(find "$PACKAGE_DIR" -name "*openauto-modern-debug*.deb" | head -1)

if [ -n "$RELEASE_PACKAGE" ]; then
    validate_package "$RELEASE_PACKAGE" "release" || VALIDATION_PASSED=false
    test_package_install "$RELEASE_PACKAGE" "release"
    echo ""
else
    echo -e "${YELLOW}⚠️  No release package found${NC}"
fi

if [ -n "$DEBUG_PACKAGE" ]; then
    validate_package "$DEBUG_PACKAGE" "debug" || VALIDATION_PASSED=false
    test_package_install "$DEBUG_PACKAGE" "debug"
    echo ""
else
    echo -e "${YELLOW}⚠️  No debug package found${NC}"
fi

# Final result
echo -e "${BLUE}📊 Validation Summary${NC}"
echo "=================================="

if [ "$VALIDATION_PASSED" = true ]; then
    echo -e "${GREEN}🎉 All packages passed validation!${NC}"
    echo ""
    echo -e "${YELLOW}📋 Next Steps:${NC}"
    echo "1. Install release package: sudo apt install $RELEASE_PACKAGE"
    echo "2. Or install debug package: sudo apt install $DEBUG_PACKAGE"
    echo "3. Check service status: systemctl status openauto"
    echo "4. View logs: journalctl -u openauto -f"
    exit 0
else
    echo -e "${RED}❌ Some packages failed validation${NC}"
    echo "Please check the errors above and rebuild packages"
    exit 1
fi
