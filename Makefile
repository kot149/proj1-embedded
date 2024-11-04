# Configuration
CCS_BASE = C:/ti/ccs1280/ccs
COMPILER_PATH = $(CCS_BASE)/tools/compiler/ti-cgt-arm_20.2.7.LTS
TIOBJ2BIN = $(CCS_BASE)/utils/tiobj2bin/tiobj2bin
TIVAWARE_PATH = C:/ti/TivaWare_C_Series-1.0

# Tools
CC = "$(COMPILER_PATH)/bin/armcl"
LINK = "$(COMPILER_PATH)/bin/armcl"

# Project configuration
PROJECTS = blinky blinky_smooth
DEFAULT_PROJECT = blinky_smooth
CONFIG = Debug

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

# Project-specific settings
ifeq ($(PROJECT),blinky_smooth)
    SRCS = blinky_smooth_main.c \
           my_util.c \
           periphConf.c \
           startup_ccs.c
    MAP_FILE = app_TM4C123GE6PM_ccs.map
    LINKER_CMD = tm4c123ge6pm.cmd
else ifeq ($(PROJECT),blinky)
    SRCS = blinky_main.c \
           my_util.c \
           periphConf.c \
           startup_ccs.c
    MAP_FILE = app_TM4C123GE6PM_ccs.map
    LINKER_CMD = tm4c123ge6pm.cmd
endif

# Derived settings
OBJ_DIR = $(PROJECT)/$(CONFIG)
OBJS = $(addprefix $(OBJ_DIR)/,$(notdir $(SRCS:.c=.obj)))
BIN = $(OBJ_DIR)/$(PROJECT).bin
OUT = $(OBJ_DIR)/$(PROJECT).out
MAP = $(OBJ_DIR)/$(MAP_FILE)
XML = $(OBJ_DIR)/$(PROJECT)_linkInfo.xml

# Source file search paths
VPATH = $(PROJECT):.

# Rules
.PHONY: all clean $(PROJECTS) build_project clean_project debug_vars

# Default target
all: $(DEFAULT_PROJECT)

# Project targets
$(PROJECTS):
	$(MAKE) -f makefile PROJECT=$@ build_project

# Build rule
build_project: $(BIN)

# Directory creation
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# Generic compilation rule
$(OBJ_DIR)/%.obj: %.c | $(OBJ_DIR)
	@echo Building $<
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
	@for %%p in ($(PROJECTS)) do $(MAKE) -f makefile PROJECT=%%p clean_project

clean_project:
	@echo Cleaning $(PROJECT)...
	@if exist $(OBJ_DIR)\*.obj del /F /Q $(OBJ_DIR)\*.obj
	@if exist $(OBJ_DIR)\*.out del /F /Q $(OBJ_DIR)\*.out
	@if exist $(OBJ_DIR)\*.map del /F /Q $(OBJ_DIR)\*.map
	@if exist $(OBJ_DIR)\*.bin del /F /Q $(OBJ_DIR)\*.bin
	@if exist $(OBJ_DIR)\*.xml del /F /Q $(OBJ_DIR)\*.xml

# Debug variables
debug_vars:
	@echo Project: $(PROJECT)
	@echo Sources: $(SRCS)
	@echo Objects: $(OBJS)
	@echo Binary: $(BIN)
	@echo Output: $(OUT)
	@echo Map: $(MAP)
	@echo XML: $(XML)
	@echo OBJ_DIR: $(OBJ_DIR)
	@echo VPATH: $(VPATH)
	@echo "Current directory:"
	@cd
	@echo "Directory contents:"
	@dir
	@echo "Project directory contents:"
	@if exist $(PROJECT) dir $(PROJECT)