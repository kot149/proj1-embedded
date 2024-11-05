# Configuration
CCS_BASE = C:/ti/ccs1280/ccs
COMPILER_PATH = $(CCS_BASE)/tools/compiler/ti-cgt-arm_20.2.7.LTS
TIOBJ2BIN = $(CCS_BASE)/utils/tiobj2bin/tiobj2bin
TIVAWARE_PATH = C:/ti/TivaWare_C_Series-1.0

# Flash tool configuration
DSLITE_PATH = $(CCS_BASE)/ccs_base/DebugServer/bin/DSLite.exe
# FTDI_PATH = $(CCS_BASE)/ccs_base/emulation/windows/ftdi_drivers/i386
FTDI_PATH = C:\Windows\System32\DriverStore\FileRepository\ti_ftdi_debug.inf_amd64_63089b49677197a5\i386

# Tools
CC = "$(COMPILER_PATH)/bin/armcl"
LINK = "$(COMPILER_PATH)/bin/armcl"

# Project configuration
DEFAULT_PROJECT = blinky_smooth
CONFIG ?= Debug

# Special targets that should not be interpreted as projects
.PHONY: all clean build debug_vars FORCE flash program

# Set PROJECT based on the target
ifeq ($(MAKECMDGOALS),)
PROJECT = $(DEFAULT_PROJECT)
else
ifneq ($(filter-out all clean build debug_vars FORCE flash program,$(MAKECMDGOALS)),)
PROJECT = $(filter-out all clean build debug_vars FORCE flash program,$(MAKECMDGOALS))
else
PROJECT = $(DEFAULT_PROJECT)
endif
endif

# Common compiler flags
COMMON_FLAGS = -mv7M4 \
               --code_state=16 \
               --float_support=FPv4SPD16 \
               -me \
               -O2 \
               --include_path="$(COMPILER_PATH)/include" \
               --include_path="$(TIVAWARE_PATH)/examples/boards/ek-tm4c123gxl" \
               --include_path="$(TIVAWARE_PATH)" \
               --define=ccs="ccs" \
               --define=PART_TM4C123GH6PM \
               --define=TARGET_IS_BLIZZARD_RB1 \
               --define=PART_TM4C123GE6PM \
               -g \
               --gcc \
               --diag_warning=225 \
               --diag_wrap=off \
               --display_error_number \
               --gen_func_subsections=on \
               --abi=eabi \
               --ual

# Compiler flags
CFLAGS = $(COMMON_FLAGS) \
         --preproc_with_compile

# Linker flags
LDFLAGS = $(COMMON_FLAGS) \
          -z \
          --heap_size=0 \
          --stack_size=512 \
          -i"$(COMPILER_PATH)/lib" \
          -i"$(COMPILER_PATH)/include" \
          --reread_libs \
          --warn_sections \
          --rom_model

# Libraries
LIBS = -llibc.a \
       -l"$(TIVAWARE_PATH)/driverlib/ccs/Debug/driverlib.lib"

# Default source files (can be overridden by project-specific settings)
SRCS ?= $(wildcard $(PROJECT)/*.c)
ifeq ($(strip $(SRCS)),)
    # If no .c files found in project directory, use these defaults
    SRCS = $(PROJECT)_main.c \
           my_util.c \
           periphConf.c \
           startup_ccs.c
endif

# Project settings with defaults
MAP_FILE ?= app_TM4C123GE6PM_ccs.map
LINKER_CMD ?= tm4c123ge6pm.cmd

# Derived settings
OBJ_DIR = $(PROJECT)/$(CONFIG)
OBJS = $(addprefix $(OBJ_DIR)/,$(notdir $(SRCS:.c=.obj)))
BIN = $(OBJ_DIR)/$(PROJECT).bin
OUT = $(OBJ_DIR)/$(PROJECT).out
MAP = $(OBJ_DIR)/$(MAP_FILE)
XML = $(OBJ_DIR)/$(PROJECT)_linkInfo.xml
CCXML = $(PROJECT)/target_config.ccxml

# Source file search paths
VPATH = $(PROJECT):.

# Default target
all: build

# Generic target to handle any project name
$(filter-out all clean build debug_vars FORCE flash program,$(MAKECMDGOALS)): build

# Build target
build: check_project $(BIN)

# Flash target
flash: build
	@echo Programming flash...
	@set "PATH=$(PATH);$(FTDI_PATH)" && \
	"$(DSLITE_PATH)" flash -c "$(CCXML)" -f "$(OUT)" -r 0 -u -e
	@echo Programming completed

# Program target (alias for flash)
program: flash

# Check if project exists
check_project:
	@if not exist "$(PROJECT)" if not exist "$(PROJECT)_main.c" if not exist "$(PROJECT)\$(PROJECT)_main.c" (echo Project $(PROJECT) not found && exit 1)

# Directory creation
$(OBJ_DIR):
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"

# Generic compilation rule
$(OBJ_DIR)/%.obj: %.c | $(OBJ_DIR)
	@echo Building $
	$(CC) $(CFLAGS) $< --output_file=$@

# Linking rule
$(OUT): $(OBJS) $(PROJECT)/$(LINKER_CMD) | $(OBJ_DIR)
	@echo Linking $@
	$(LINK) $(LDFLAGS) -m"$(MAP)" --xml_link_info="$(XML)" -o $@ $(OBJS) $(LIBS) $(PROJECT)/$(LINKER_CMD)

# Binary creation rule
$(BIN): $(OUT)
	@echo Creating binary $@
	$(TIOBJ2BIN) $< $@ \
		"$(COMPILER_PATH)/bin/armofd" \
		"$(COMPILER_PATH)/bin/armhex" \
		"$(CCS_BASE)/utils/tiobj2bin/mkhex4bin"

# Clean rules
clean:
	@if "$(MAKECMDGOALS)" == "clean" (
		for /d %%d in (*) do @if exist "%%d\$(CONFIG)" (
			echo Cleaning %%d...
			del /F /Q "%%d\$(CONFIG)\*.*" 2>nul
		)
	) else (
		@echo Cleaning $(PROJECT)...
		@if exist "$(OBJ_DIR)\*.*" del /F /Q "$(OBJ_DIR)\*.*"
	)

# Debug variables
debug_vars:
	@echo Project: $(PROJECT)
	@echo Sources: $(SRCS)
	@echo Objects: $(OBJS)
	@echo Binary: $(BIN)
	@echo Output: $(OUT)
	@echo Map: $(MAP)
	@echo XML: $(XML)
	@echo CCXML: $(CCXML)
	@echo OBJ_DIR: $(OBJ_DIR)
	@echo VPATH: $(VPATH)