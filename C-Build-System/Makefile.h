#pragma once
#include <string>
#include <fstream>
#include <filesystem>

namespace make {
	/*
	Parameters (keys):
	- modules
	- includes
	- platform
	- libs
	- debugLibs
	- library
	- debugLibrary
	*/
	std::string makefile =
		"# GNU Make docs: https://www.gnu.org/software/make/manual/make.html\n"
		"# Quick tutorial: https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/\n"
		"\n"
		"CC := gcc\n"
		"LD := gcc\n"
		"AR := ar\n"
		"override CFLAGS := $(sort -Wall -Wextra $(CFLAGS))\n"
		"\n"
		"# Generate a recursive list of subdirectories of src\n"
		"DIR = src\n"
		"recurse = $(foreach D,$1,$(wildcard $D/*) $(call recurse,$(patsubst %/.,%,$(wildcard $D/*/.))))\n"
		"dirpaths := $(call recurse,$(DIR))\n"
		"is_directory = $(wildcard $(1)/*)\n"
		"_MODULES := . $(foreach path,$(dirpaths),$(if $(call is_directory,$(path)),$(path)))\n"
		"MODULES :=  $(subst src/,,$(_MODULES))\n"
		"\n"
		"SRC_DIR := $(addprefix src/,$(MODULES))\n"
		"SRC := $(foreach sdir, $(SRC_DIR),$(wildcard $(sdir)/*.c))\n"
		"OBJ = $(patsubst %.c,build/release/%.o,$(SRC))\n"
		"OBJ_D = $(patsubst %.c,build/debug/%.o,$(SRC))\n"
		"BUILD_DIR = $(addprefix build/release/src/,$(MODULES))\n"
		"BUILD_DIR_D = $(addprefix build/debug/src/,$(MODULES))\n"
		"TEST_SRC := $(wildcard test/*.c)\n"
		"\n"
		"INCLUDES := {{includes}}\n"
		"LIBDIR   := \"./lib/release/{{platform}}\"\n"
		"LIBDIR_D := \"./lib/debug/{{platform}}\"\n"
		"# LIBS should be most ambiguous to least ambiguous\n"
		"LIBS_R   := {{libs}}\n"
		"LIBS_D := {{debugLibs}}\n"
		"\n"
		"LIBRARY   := lib/release/{{platform}}/{{library}}\n"
		"LIBRARY_D := lib/debug/{{platform}}/{{debugLibrary}}\n"
		"EXECUTABLE   := \n"
		"EXECUTABLE_D := \n"
		"ifeq ($(OS), Windows_NT)\n"
		"\tEXECUTABLE   := build/release/main.exe\n"
		"\tEXECUTABLE_D := build/debug/main.exe\n"
		"else\n"
		"\tEXECUTABLE   := build/release/main.out\n"
		"\tEXECUTABLE_D := build/debug/main.out\n"
		"endif\n"
		"\n"
		"\n"
		"#####[ Platform specific variables ]#####\n"
		"DELETE      := \n"
		"RMDIR       := \n"
		"#EXE         := \n"
		"HIDE_OUTPUT := \n"
		"PS          := \n"
		"ifeq ($(OS), Windows_NT)\n"
		"\tDELETE      := del /f\n"
		"\tRMDIR       := rmdir /s /q\n"
		"#\tEXE         :=main.exe\n"
		"\tHIDE_OUTPUT := 2> nul\n"
		"\tPS          :=\\\\n"
		"\t\n"
		"else\n"
		"\tDELETE      := rm -f\n"
		"\tRMDIR       := rm -rf\n"
		"#\tEXE         :=main.out\n"
		"\tHIDE_OUTPUT := > /dev/null\n"
		"\tPS          :=/ \n"
		"endif\n"
		"\n"
		"#####[ recipes ]#####\n"
		"\n"
		"$(BUILD_DIR):\n"
		"ifeq ($(OS), Windows_NT)\n"
		"\t@IF not exist \"$@\" (mkdir \"$@\")\n"
		"else\n"
		"\t@mkdir -p $@\n"
		"endif\n"
		"\n"
		"$(BUILD_DIR_D):\n"
		"ifeq ($(OS), Windows_NT)\n"
		"\t@IF not exist \"$@\" (mkdir \"$@\")\n"
		"else\n"
		"\t@mkdir -p $@\n"
		"endif\n"
		"\n"
		"$(LIBDIR):\n"
		"ifeq ($(OS), Windows_NT)\n"
		"\t@IF not exist $@ (mkdir $@)\n"
		"else\n"
		"\t@mkdir -p $@\n"
		"endif\n"
		"\n"
		"$(LIBDIR_D):\n"
		"ifeq ($(OS), Windows_NT)\n"
		"\t@IF not exist $@ (mkdir $@)\n"
		"else\n"
		"\t@mkdir -p $@\n"
		"endif\n"
		"\n"
		"# filtering and target patterns\n"
		"# https://www.gnu.org/software/make/manual/make.html#Static-Usage\n"
		"$(filter build/release/%.o,$(OBJ)): build/release/%.o: %.c\n"
		"\t@echo Building $< \n"
		"\t$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES) -L $(LIBDIR) $(LIBS_R)\n"
		"\n"
		"$(filter build/debug/%.o,$(OBJ_D)): build/debug/%.o: %.c\n"
		"\t@echo Building $< \n"
		"\t$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES) -L $(LIBDIR_D) $(LIBS_D)\n"
		"\n"
		"\n"
		".PHONY: release debug release_executable debug_executable print clean\n"
		"release:\n"
		"\t$(MAKE) release_executable CFLAGS=\"-DNDEBUG $(CFLAGS)\"\n"
		"\n"
		"debug:\n"
		"\t$(MAKE) debug_executable CFLAGS=\"-DDEBUG $(CFLAGS)\"\n"
		"\n"
		"release_executable: release_library\n"
		"\t$(LD) $(CFLAGS) $(TEST_SRC) -o $(EXECUTABLE) $(INCLUDES) -L . -l $(basename $(LIBRARY)) -L $(LIBDIR) $(LIBS_R)\n"
		"\n"
		"release_library: $(LIBDIR) $(BUILD_DIR) $(OBJ)\n"
		"\t$(AR) rcs $(LIBRARY) $(foreach obj,$(OBJ), -o $(obj))\n"
		"\n"
		"debug_executable: debug_library\n"
		"\t$(LD) $(CFLAGS) $(TEST_SRC) -o $(EXECUTABLE_D) $(INCLUDES) -L . -l $(basename $(LIBRARY_D)) -L $(LIBDIR_D) $(LIBS_D)\n"
		"\n"
		"debug_library: $(LIBDIR_D) $(BUILD_DIR_D) $(OBJ_D)\n"
		"\t$(AR) rcs $(LIBRARY_D) $(foreach obj,$(OBJ_D), -o $(obj))\n"
		"\n"
		"print:\n"
		"\t@echo $(SRC_DIR)\n"
		"\t@echo $(SRC)\n"
		"\t@echo $(OBJ)\n"
		"\t@echo $(OBJ_D)\n"
		"\t@echo $(BUILD_DIR)\n"
		"\n"
		"clean:\n"
		"ifeq ($(OS), Windows_NT)\n"
		"\tIF exist build ($(RMDIR) build)\n"
		"\tIF exist \"$(LIBRARY)\" ($(DELETE) \"$(subst /,\\,$(LIBRARY))\")\n"
		"\tIF exist \"$(LIBRARY_D)\" ($(DELETE) \"$(subst /,\\,$(LIBRARY_D))\")\n"
		"else\n"
		"\t@$(RMDIR) build\n"
		"\t@$(DELETE) $(LIBRARY)\n"
		"\t@$(DELETE) $(LIBRARY_D)\n"
		"endif\n"
		"\tmkdir build\n";

		namespace fs = std::filesystem;

		inline bool replaceVarDeclaration(fs::path makefilePath, const std::string varName, const std::string newValue) {
			if (!fs::exists(makefilePath)) {
				printf("1\n");
				return false;
			}
			
			std::ifstream makefile(makefilePath);
			
			if (!makefile.is_open()) {
				printf("2\n");
				return false;
			}

			// go to beginning of file
			makefile.clear();
			makefile.seekg(0);

			bool foundFlag = false;
			std::vector<std::string> newFileLines;
			std::string line;
			while (std::getline(makefile, line)) {
				std::string::size_type found = line.find(varName);
				std::string::size_type foundEquals = line.find(":=");
				if (found != std::string::npos && foundEquals != std::string::npos && found < foundEquals) {
					printf("[FOUND IT]\n");
					newFileLines.push_back(varName + " := " + newValue);
					foundFlag = true;
				}
				else {
					newFileLines.push_back(line);
				}
			}

			if (!foundFlag) {
				printf("3\n");
				return false;
			}

			makefile.close();

			// rewrite file
			std::ofstream newMakefile(makefilePath);
			if (!newMakefile.is_open()) {
				printf("4\n");
				return false;
			}

			for (auto& newLine : newFileLines) {
				newMakefile << newLine << std::endl;
			}
			
			newMakefile.close();

			return true;
		}
}