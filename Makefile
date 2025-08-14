
CC = gcc
SRC = $(wildcard *.c multiThread/*.c msgq/*.c)
ASM = $(patsubst %.c,release/%.s,$(notdir $(SRC)))
OBJ = $(patsubst %.c,release/%.o,$(notdir $(SRC)))
DBG = $(patsubst %.c,debug/%.o,$(notdir $(SRC)))
WFLAGS = -Wall -Werror -Wextra 
INCLUDE_DIR += -I. -ImultiThread -Imsgq
FOLDER = release debug
VPATH = multiThread msgq

# create release and debug folders
create_dir:
	mkdir -p $(FOLDER)

# To create executable file in release folder
all : linux 
	$(CC) $(WFLAGS) $(INCLUDE_DIR) $(OBJ) -o multiThread.exe

# To create object,assembly,debug files
linux : files_asm files_obj files_dbg

# To create Assembly files in release folder
files_asm : create_dir $(ASM)
release/%.s : %.c
	$(CC) -S $(WFLAGS) $(INCLUDE_DIR) $< -o $@

# To create object files in release folder
files_obj : create_dir $(OBJ)
release/%.o : %.c
	$(CC) -c $(WFLAGS) $(INCLUDE_DIR) $< -o $@

# To create Debug files in debug folder
files_dbg : create_dir $(DBG)
debug/%.o : %.c
	$(CC) -g -O0 -c $(WFLAGS) $(INCLUDE_DIR) $< -o $@

# To clear release and debug folder.
clean : 
	rm -rf $(FOLDER)