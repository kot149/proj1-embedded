# Configuration
CCS_BASE = C:/ti/ccs1280/ccs
DSLITE_PATH = $(CCS_BASE)/ccs_base/DebugServer/bin/DSLite.exe
FTDI_PATH = C:\Windows\System32\DriverStore\FileRepository\ti_ftdi_debug.inf_amd64_63089b49677197a5\i386

# Project configuration
DEFAULT_PROJECT = blinky_smooth
CONFIG = Debug

# Special targets
.PHONY: all clean build flash program

# Set PROJECT based on the target
ifeq ($(MAKECMDGOALS),)
PROJECT = $(DEFAULT_PROJECT)
else
ifneq ($(filter-out all clean build flash program,$(MAKECMDGOALS)),)
PROJECT = $(filter-out all clean build flash program,$(MAKECMDGOALS))
else
PROJECT = $(DEFAULT_PROJECT)
endif
endif

# Paths
DEBUG_DIR = $(PROJECT)/$(CONFIG)
OUT = $(DEBUG_DIR)/$(PROJECT).out
CCXML = $(PROJECT)/target_config.ccxml

# Define MAKE command with proper quoting
MAKE_CMD = "$(CCS_BASE)/utils/bin/gmake"

# Default target
all: build

# Generic target to handle any project name
$(filter-out all clean build flash program,$(MAKECMDGOALS)): build

# Build target
build:
	@if not exist "$(PROJECT)" (echo Project $(PROJECT) not found && exit 1)
	@cd "$(PROJECT)/$(CONFIG)" && $(MAKE_CMD) all

# Clean target
clean:
	@if "$(MAKECMDGOALS)" == "clean" (
		for /d %%d in (*) do @if exist "%%d\$(CONFIG)" (
			echo Cleaning %%d...
			cd "%%d\$(CONFIG)" && $(MAKE_CMD) clean
		)
	) else (
		@echo Cleaning $(PROJECT)...
		@cd "$(PROJECT)/$(CONFIG)" && $(MAKE_CMD) clean
	)

# Flash target (unchanged from original)
flash: build
	@echo Programming flash...
	@set "PATH=$(PATH);$(FTDI_PATH)" && \
	"$(DSLITE_PATH)" flash -c "$(CCXML)" -f "$(OUT)" -r 0 -u -e
	@echo Programming completed

# Program target (alias for flash)
program: flash