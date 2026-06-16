# TTimer Build Configuration

# Include HomeTUI's platform detection logic
include src/HomeTUI/config.mk

# TTimer specific overrides
export CC := clang
# COMPILE_FLAGS is already being appended to in HomeTUI/config.mk
