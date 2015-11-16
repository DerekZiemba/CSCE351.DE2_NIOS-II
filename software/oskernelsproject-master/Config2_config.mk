#START GENERATED

# The following TYPE comment allows tools to identify the 'type' of target this 
# makefile is associated with. 
# TYPE: APP_MAKEFILE

# This following VERSION comment indicates the version of the tool used to 
# generate this makefile. A makefile variable is provided for VERSION as well. 
# ACDS_VERSION: 13.1
ACDS_VERSION := 13.1

# This following BUILD_NUMBER comment indicates the build number of the tool 
# used to generate this makefile. 
# BUILD_NUMBER: 162

# Define path to the application ELF. 
# It may be used by the makefile fragments so is defined before including them. 
# 
ELF := Project2.elf

# Paths to C, C++, and assembly source files.
C_SRCS += hello_world.c
C_SRCS += alarm_handler.c
C_SRCS += project1.c
C_SRCS += queue.c
C_SRCS += semaphore.c
C_SRCS += thread_handler.c
CXX_SRCS :=
ASM_SRCS += Injection.S
ASM_SRCS += exception_handler.S


# Path to root of object file tree.
OBJ_ROOT_DIR := obj

# Options to control objdump.
CREATE_OBJDUMP := 1
OBJDUMP_INCLUDE_SOURCE := 1
OBJDUMP_FULL_CONTENTS := 0

# Options to enable/disable optional files.
CREATE_ELF_DERIVED_FILES := 0
CREATE_LINKER_MAP := 1

# Common arguments for ALT_CFLAGSs
APP_CFLAGS_DEFINED_SYMBOLS :=
APP_CFLAGS_UNDEFINED_SYMBOLS :=
APP_CFLAGS_OPTIMIZATION := -O0
APP_CFLAGS_DEBUG_LEVEL := -g
APP_CFLAGS_WARNINGS := -Wall
APP_CFLAGS_USER_FLAGS :=

APP_ASFLAGS_USER :=
APP_LDFLAGS_USER :=

# Linker options that have default values assigned later if not
# assigned here.
LINKER_SCRIPT :=
CRT0 :=
SYS_LIB :=

# Define path to the root of the BSP.
BSP_ROOT_DIR := ../P2_PrototypeOS_bsp

# List of application specific include directories, library directories and library names
APP_INCLUDE_DIRS :=
APP_LIBRARY_DIRS :=
APP_LIBRARY_NAMES :=

# Pre- and post- processor settings.
BUILD_PRE_PROCESS :=
BUILD_POST_PROCESS :=

QUARTUS_PROJECT_DIR := ../../


#END GENERATED
