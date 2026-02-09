##########
# Colors #
##########

GREEN = \033[1;32m
NC = \033[0m

######################
# The SCULL Compiler #
######################


# --- LLVM --- #

LLVM_DIR   := external/llvm-project
LLVM_BUILD := $(LLVM_DIR)/build
LLVM_LIB   := $(LLVM_BUILD)/lib
LLVM_INC   := $(LLVM_BUILD)/include

LLVM_SENTINEL := $(LLVM_LIB)/libLLVM.so

llvm-sync:
	@git submodule sync --recursive
	@git submodule update --init --recursive --depth 1
	@echo -e "$(GREEN)[LLVM]$(NC) Submodule synced"

JOBS ?= $(shell nproc)

llvm:
	cd $(LLVM_DIR) && \
	mkdir -p build && cd build && \
	cmake -S ../llvm -B . \
	  -G Ninja \
	  -DCMAKE_BUILD_TYPE=Release \
	  -DLLVM_TARGETS_TO_BUILD="X86" \
	  -DLLVM_ENABLE_RTTI=ON \
	  -DLLVM_ENABLE_EH=ON \
	  -DLLVM_ENABLE_TERMINFO=OFF \
	  -DLLVM_ENABLE_ZLIB=OFF \
	  -DLLVM_ENABLE_ZSTD=OFF \
	  -DLLVM_ENABLE_LIBXML2=OFF \
	  -DLLVM_INCLUDE_TESTS=OFF \
	  -DLLVM_INCLUDE_EXAMPLES=OFF \
	  -DLLVM_INCLUDE_BENCHMARKS=OFF \
	  -DLLVM_BUILD_LLVM_DYLIB=ON \
		-DLLVM_LINK_LLVM_DYLIB=ON \
	  -DBUILD_SHARED_LIBS=OFF && \
	ninja -j $(JOBS)
	@echo -e "$(GREEN)[LLVM]$(NC) LLVM built"

ifeq ($(wildcard $(LLVM_SENTINEL)),)
LLVM_AVAILABLE := 0
else
LLVM_AVAILABLE := 1
endif

check-llvm:
ifeq ($(LLVM_AVAILABLE),0)
	$(error LLVM not built. Run: make llvm-sync llvm)
endif

ifeq ($(LLVM_AVAILABLE),1)

LLVM_CONFIG := $(LLVM_BUILD)/bin/llvm-config

LLVM_CXXFLAGS := \
	-isystem $(LLVM_INC) \
	$(shell $(LLVM_CONFIG) --cxxflags) \
	-Wno-unused-parameter

LLVM_LDFLAGS := \
	-L$(LLVM_LIB) \
	-Wl,-rpath,'$$ORIGIN/../external/llvm-project/build/lib' \
	-lLLVM \
	$(shell $(LLVM_CONFIG) --system-libs)

endif

# --- Source and Destination directories --- #

INC_DIR = ./sclc/include
SRC_DIR = ./sclc/src
OBJ_DIR = ./obj
BIN_DIR = ./bin

# --- Development Build --- #

CC = clang
CXX = clang++

CFLAGS = -std=c23 -g -O0 -Wall -Wextra -I$(INC_DIR)
CXXFLAGS = -std=c++17 -g -O0 -Wall -Wextra -I$(INC_DIR)

C_SRCS = $(shell find $(SRC_DIR) -name "*.c" -type f)
CXX_SRCS = $(shell find $(SRC_DIR) -name "*.cpp" -type f)

C_OBJS = $(C_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CXX_OBJS = $(CXX_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OBJS = $(C_OBJS) $(CXX_OBJS)

C_DEPS = $(C_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.d)
CXX_DEPS = $(CXX_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.d)
DEPS = $(C_DEPS) $(CXX_DEPS)

TARGET = $(BIN_DIR)/sclc

sclc: check-llvm $(TARGET)
	@echo -e "$(GREEN)[INFO]$(NC) Development Build Successful"

$(TARGET): $(OBJS) | $(BIN_DIR)
	@$(CXX) $(OBJS) -o $@ $(LLVM_LDFLAGS) -lm
	@echo -e "$(GREEN)[LD]$(NC) $@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -MMD -MF $(OBJ_DIR)/$*.d -c $< -o $@
	@echo -e "$(GREEN)[CC]$(NC) $@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) -MMD -MF $(OBJ_DIR)/$*.d -c $< -o $@
	@echo -e "$(GREEN)[CXX]$(NC) $@"

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# --- Release Build --- #

REL_OBJ_DIR = ./obj-release
REL_BIN_DIR = ./bin-release

CFLAGS_RELEASE = -std=c23 -O2 -DNDEBUG -Wall -Wextra -I$(INC_DIR)
CXXFLAGS_RELEASE = -std=c++17 -O2 -DNDEBUG -Wall -Wextra -I$(INC_DIR)

REL_C_OBJS   = $(C_SRCS:$(SRC_DIR)/%.c=$(REL_OBJ_DIR)/%.o)
REL_CXX_OBJS = $(CXX_SRCS:$(SRC_DIR)/%.cpp=$(REL_OBJ_DIR)/%.o)
REL_OBJS     = $(REL_C_OBJS) $(REL_CXX_OBJS)

REL_C_DEPS   = $(C_SRCS:$(SRC_DIR)/%.c=$(REL_OBJ_DIR)/%.d)
REL_CXX_DEPS = $(CXX_SRCS:$(SRC_DIR)/%.cpp=$(REL_OBJ_DIR)/%.d)
REL_DEPS     = $(REL_C_DEPS) $(REL_CXX_DEPS)

REL_TARGET = $(REL_BIN_DIR)/sclc

install: sclc-release
	@echo -e "$(GREEN)[INSTALL]$(NC) $(REL_TARGET) -> /usr/local/bin/sclc"
	@sudo cp $(REL_TARGET) /usr/local/bin/sclc

sclc-release: $(REL_TARGET)
	@echo -e "$(GREEN)[INFO]$(NC) Release Build Successful"

$(REL_TARGET): $(REL_OBJS) | $(REL_BIN_DIR)
	@$(CXX) $(REL_OBJS) -o $@ $(LLVM_LDFLAGS) -lm
	@echo -e "$(GREEN)[LD]$(NC) $@"

$(REL_OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(REL_OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS_RELEASE) -MMD -MF $(REL_OBJ_DIR)/$*.d -c $< -o $@
	@echo -e "$(GREEN)[CC] [REL]$(NC) $@"

$(REL_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(REL_OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS_RELEASE) $(LLVM_CXXFLAGS) -MMD -MF $(REL_OBJ_DIR)/$*.d -c $< -o $@
	@echo -e "$(GREEN)[CXX] [REL]$(NC) $@"

$(REL_OBJ_DIR):
	@mkdir -p $(REL_OBJ_DIR)

$(REL_BIN_DIR):
	@mkdir -p $(REL_BIN_DIR)

# --- Cleanup --- #

clean-sclc:
	@rm -frf $(OBJ_DIR) $(BIN_DIR) $(REL_OBJ_DIR) $(REL_BIN_DIR)
	@echo -e "$(GREEN)[CLEAN]$(NC) Removed all build directories"

#########################
# compile_commands.json #
#########################

compile_commands.json:
	@bear -- $(MAKE) clean-sclc sclc
	@echo -e "$(GREEN)[BEAR]$(NC) Generated compile_commands.json"

clean-compile_commands.json:
	@rm compile_commands.json
	@echo -e "$(GREEN)[CLEAN]$(NC) Removed compile_commands.json"

######################
# Examples for SCULL #
######################

SCLC = $(TARGET)
SCLC_FLAGS = -i ./lib
EXAMPLES_DIR = ./examples
EXAMPLE_SRCS = $(shell find $(EXAMPLES_DIR) -name "*.scl" -type f)
EXAMPLE_BINARIES = $(EXAMPLE_SRCS:.scl=)

examples: sclc $(EXAMPLE_BINARIES)
	@echo -e "$(GREEN)[INFO]$(NC) Examples Compiled Successfully"

$(EXAMPLES_DIR)/%: $(EXAMPLES_DIR)/%.scl $(TARGET) tmp_examples_dir
	@$(SCLC) $(SCLC_FLAGS) $<
	@echo -e "$(GREEN)[SCLC]$(NC) $<"

tmp_examples_dir:
	@mkdir -p /tmp/examples

clean-examples:
	@find ./examples -type f ! -name "*.scl" -delete
	@echo -e "$(GREEN)[CLEAN]$(NC) Removed $(EXAMPLE_BINARIES)"

clean-all: clean-sclc clean-examples clean-compile_commands.json

-include $(DEPS) $(REL_DEPS)

.DEFAULT_GOAL := sclc

.PHONY: llvm-sync llvm check-llvm sclc sclc-release clean-sclc clean-all compile_commands.json clean-compile_commands.json install examples clean-examples
