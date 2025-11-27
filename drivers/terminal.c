#include "terminal.h"
#include "frame_buffer.h"
#include "input_buffer.h"
#include "types.h"

#define MAX_COMMAND_LEN 64
#define MAX_ARGS_LEN 192
#define PROMPT "myos> "

// Command function prototypes
void cmd_echo(char* args);
void cmd_clear(char* args);
void cmd_help(char* args);
void cmd_version(char* args);
void cmd_shutdown(char* args);

// Command table
struct command commands[] = {
    {"echo", cmd_echo},
    {"clear", cmd_clear},
    {"help", cmd_help},
    {"version", cmd_version},
    {"shutdown", cmd_shutdown},
    {0, 0}  // End marker
};

/** terminal_init:
 * Initializes the terminal
 */
void terminal_init(void)
{
    fb_clear();
    fb_puts("Tiny OS Terminal\n");
    fb_puts("Type 'help' for available commands\n\n");
}

/** terminal_parse_command:
 * Parses input into command and arguments
 */
void terminal_parse_command(char* input, char* command, char* args)
{
    u32int i = 0;
    u32int j = 0;
    
    // Skip leading spaces
    while (input[i] == ' ') {
        i++;
    }
    
    // Extract command
    while (input[i] != '\0' && input[i] != ' ' && j < (MAX_COMMAND_LEN - 1)) {
        command[j++] = input[i++];
    }
    command[j] = '\0';
    
    // Skip spaces after command
    while (input[i] == ' ') {
        i++;
    }
    
    // Extract arguments
    j = 0;
    while (input[i] != '\0' && j < (MAX_ARGS_LEN - 1)) {
        args[j++] = input[i++];
    }
    args[j] = '\0';
}

/** terminal_execute_command:
 * Executes a command with arguments
 */
void terminal_execute_command(char* input)
{
    char command[MAX_COMMAND_LEN];
    char args[MAX_ARGS_LEN];
    u32int i = 0;
    
    // Parse command and arguments
    terminal_parse_command(input, command, args);
    
    // Skip empty commands
    if (command[0] == '\0') {
        return;
    }
    
    // Find and execute command
    while (commands[i].name != 0) {
        // Simple string comparison
        u32int j = 0;
        u8int match = 1;
        
        while (commands[i].name[j] != '\0' && command[j] != '\0') {
            if (commands[i].name[j] != command[j]) {
                match = 0;
                break;
            }
            j++;
        }
        
        // Check if both strings ended at the same time
        if (match && commands[i].name[j] == '\0' && command[j] == '\0') {
            // Execute the command
            commands[i].function(args);
            return;
        }
        
        i++;
    }
    
    // Command not found
    fb_puts("Unknown command: ");
    fb_puts(command);
    fb_puts(". Type 'help' for available commands.\n");
}

/** terminal_run:
 * Runs the terminal main loop
 */
void terminal_run(void)
{
    char input[256];
    s32int len;
    
    while (1) {
        // Display prompt
        fb_puts(PROMPT);
        
        // Read line from user
        len = readline(input, 256);
        
        if (len > 0) {
            // Execute command
            terminal_execute_command(input);
        } else if (len == 0) {
            // Empty line, just show prompt again
        }
        
        // Small delay
        __asm__ volatile("hlt");
    }
}

// Command implementations

/** cmd_echo:
 * Echo command - displays the provided text
 */
void cmd_echo(char* args)
{
    if (args[0] == '\0') {
        fb_puts("\n");
    } else {
        fb_puts(args);
        fb_puts("\n");
    }
}

/** cmd_clear:
 * Clear command - clears the screen
 */
void cmd_clear(char* args)
{
    (void)args;  // Unused parameter
    fb_clear();
}

/** cmd_help:
 * Help command - shows available commands
 */
void cmd_help(char* args)
{
    (void)args;  // Unused parameter
    fb_puts("\nAvailable commands:\n");
    fb_puts("  echo [text]    - Display the provided text\n");
    fb_puts("  clear          - Clear the screen\n");
    fb_puts("  help           - Show this help message\n");
    fb_puts("  version        - Display OS version\n");
    fb_puts("  shutdown       - Prepare system for shutdown\n\n");
}

/** cmd_version:
 * Version command - displays OS version
 */
void cmd_version(char* args)
{
    (void)args;  // Unused parameter
    fb_puts("\nTiny OS v1.0\n");
    fb_puts("Worksheet 2 Part 2 - Terminal Implementation\n");
    fb_puts("Built with keyboard input and interrupt handling\n\n");
}

/** cmd_shutdown:
 * Shutdown command - prepares system for shutdown
 */
void cmd_shutdown(char* args)
{
    (void)args;  // Unused parameter
    fb_puts("\nSystem shutdown requested.\n");
    fb_puts("In a real OS, this would save data and power off.\n");
    fb_puts("For now, the system will continue running.\n\n");
}
