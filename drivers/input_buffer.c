#include "input_buffer.h"
#include "types.h"

// External buffer variables from interrupts.c
extern u8int input_buffer[];
extern u8int buffer_index;
extern u8int buffer_read_index;
extern u8int buffer_count;

#define INPUT_BUFFER_SIZE 256

/** getc:
 * Gets a single character from the input buffer.
 * Returns 0 if buffer is empty.
 *
 * @return The character read, or 0 if buffer is empty
 */
u8int getc(void)
{
    u8int character = 0;
    
    // Disable interrupts while reading from buffer
    __asm__ volatile("cli");
    
    // Check if buffer has characters
    if (buffer_count > 0) {
        // Read from buffer_index (start of valid data)
        character = input_buffer[buffer_index];
        // Move buffer_index forward and decrease count
        buffer_index = (buffer_index + 1) % INPUT_BUFFER_SIZE;
        buffer_count--;
    }
    
    // Re-enable interrupts
    __asm__ volatile("sti");
    
    return character;
}

/** input_buffer_available:
 * Checks if there are characters available in the input buffer.
 *
 * @return 1 if characters are available, 0 otherwise
 */
u8int input_buffer_available(void)
{
    return (buffer_count > 0) ? 1 : 0;
}

/** readline:
 * Reads a line from the input buffer until a newline is encountered.
 * The line (without the newline) is stored in the provided buffer.
 *
 * @param buffer The buffer to store the line
 * @param max_len Maximum length to read (including null terminator)
 * @return Number of characters read (not including null terminator), or -1 if error
 */
s32int readline(char *buffer, u32int max_len)
{
    u32int i = 0;
    u8int c;
    
    if (buffer == 0 || max_len == 0) {
        return -1;
    }
    
    // Read characters until newline or buffer full
    while (i < (max_len - 1)) {
        // Wait for characters to be available
        while (buffer_count == 0) {
            __asm__ volatile("hlt");
        }
        
        c = getc();
        
        // If we got a character
        if (c != 0) {
            // Check for newline
            if (c == '\n' || c == '\r') {
                buffer[i] = '\0';
                return (s32int)i;
            }
            
            // Add character to buffer
            buffer[i] = c;
            i++;
        }
    }
    
    // Buffer full, null terminate
    buffer[max_len - 1] = '\0';
    return (s32int)(max_len - 1);
}
