#ifndef INCLUDE_TERMINAL_H
#define INCLUDE_TERMINAL_H

#include "types.h"

/** Command structure */
struct command {
    const char* name;
    void (*function)(char* args);
};

/** terminal_init:
 * Initializes the terminal
 */
void terminal_init(void);

/** terminal_run:
 * Runs the terminal main loop
 */
void terminal_run(void);

/** terminal_execute_command:
 * Executes a command with arguments
 *
 * @param input The input string containing command and arguments
 */
void terminal_execute_command(char* input);

/** terminal_parse_command:
 * Parses input into command and arguments
 *
 * @param input The input string
 * @param command Buffer to store command (must be at least 64 bytes)
 * @param args Buffer to store arguments (must be at least 192 bytes)
 */
void terminal_parse_command(char* input, char* command, char* args);

#endif /* INCLUDE_TERMINAL_H */
