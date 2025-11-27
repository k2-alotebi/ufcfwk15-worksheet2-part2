#ifndef TERMINAL_H
#define TERMINAL_H

#include "drivers/type.h"

#define MAX_COMMAND_LENGTH 256
#define MAX_ARGS 16
#define PROMPT "TestimonyOS> "
#define OS_VERSION "TestimonyOS v0.1"

// Command structure
struct command {
    const char* name;
    void (*function)(char* args);
    const char* help;
};

void terminal_initialize(void);
void terminal_run(void);
void terminal_execute_command(char* input);

#endif
