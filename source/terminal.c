#include "terminal.h"
#include "drivers/frame_buffer.h"
#include "drivers/keyboard.h"
#include "strings.h"


// Command handlers
static void cmd_echo(char* args) {
    if (args) {
        fb_write_string(args);
    }
    fb_write_char('\n');
}

static void cmd_clear(char* args) {
    (void)args; // Unused parameter
    fb_clear();
    // Reset cursor position
    // fb_move(0, 0);
    // Make sure we maintain proper text visibility after clear
    fb_write_string(PROMPT);
}

static void cmd_help(char* args) {
    (void)args; // Unused parameter
    fb_write_string("Available commands:\n");
    fb_write_string("  echo [text] - Display the provided text\n");
    fb_write_string("  clear      - Clear the screen\n");
    fb_write_string("  help       - Show this help message\n");
    fb_write_string("  version    - Display OS version\n");
    fb_write_string("  shutdown   - Prepare system for shutdown\n");
}

static void cmd_version(char* args) {
    (void)args; // Unused parameter
    fb_write_string(OS_VERSION);
    fb_write_char('\n');
}

static void cmd_shutdown(char* args) {
    (void)args; // Unused parameter
    fb_write_string("Preparing to shutdown...\n");
    fb_write_string("Saving system state...\n");
    
    // Add any cleanup operations here
    
    fb_write_string("System is ready to power off.\n");
    fb_write_string("You can now turn off your computer.\n");
    
    __asm__ __volatile__ (
        "cli\n"  // Disable interrupts
        "hlt\n"  // Halt the CPU
    );
}

// Command table
static const struct command commands[] = {
    {"echo", cmd_echo, "Display the provided text"},
    {"clear", cmd_clear, "Clear the screen"},
    {"help", cmd_help, "Show available commands"},
    {"version", cmd_version, "Display OS version"},
    {"shutdown", cmd_shutdown, "Prepare system for shutdown"}
};

static u32int get_command_count(void) {
    return sizeof(commands) / sizeof(commands[0]);
}

// Split command and arguments
static void split_command(char* input, char** command, char** args) {
    *command = input;
    *args = NULL;

    // Skip leading spaces
    while (*input == ' ') input++;
    *command = input;

    // Find first space after command
    while (*input != '\0' && *input != ' ') input++;

    // If we found a space, split the string
    if (*input == ' ') {
        *input = '\0';  // Null terminate command
        input++;        // Move past null terminator
        // Skip additional spaces
        while (*input == ' ') input++;
        if (*input != '\0') {
            *args = input;
        }
    }
}

void terminal_initialize(void) {
    fb_clear();
    fb_write_string(PROMPT);
}

void terminal_run(void) {
    static char line[MAX_COMMAND_LENGTH];
    static int pos = 0;

    // Check for keyboard input
    u8int scan_code = getc();
    if (scan_code != 0) {
        // Convert scan code to ASCII
        char ascii = keyboard_scan_code_to_ascii(scan_code);
        
        // Handle the character
        
        if (ascii != 0) {
            if (ascii == '\b') {  // Backspace (scan code 0x0E)
                if (pos > 0) {
                    pos--;  // Move position back
                    // Clear the character on screen
                    fb_write_char('\b');   // Move cursor back
                    fb_write_char(' ');    // Write space over character
                    fb_write_char('\b');   // Move cursor back again
                }
            }
            else if (ascii == '\n') {  // Enter
                fb_write_char('\n');
                line[pos] = '\0';
                if (pos > 0) {
                    terminal_execute_command(line);
                }
                pos = 0;
                fb_write_string(PROMPT);
            }
            else if (ascii == 0x0C) {  // Ctrl+L (clear screen)
                fb_clear();
                pos = 0;
                fb_write_string(PROMPT);
            }
            else if (pos < MAX_COMMAND_LENGTH - 1 && ascii >= 32 && ascii <= 126) {
                line[pos++] = ascii;
                fb_write_char(ascii);
            }
        }
    }
}

void terminal_execute_command(char* input) {
    char* command;
    char* args;
    
    // Skip empty lines
    if (!input || input[0] == '\0') {
        return;
    }

    split_command(input, &command, &args);

    // Look for command in command table
    for (u32int i = 0; i < get_command_count(); i++) {
        if (strcmp(commands[i].name, command) == 0) {
            commands[i].function(args);
            return;
        }
    }

    // Command not found
    fb_write_string("Unknown command: ");
    fb_write_string(command);
    fb_write_char('\n');
}
