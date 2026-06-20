# TTimer Build Configuration

export CC := clang
export COMPILE_FLAGS := -std=c23 -Wall

# Include HomeTUI's platform detection logic
include src/HomeTUI/config.mk

