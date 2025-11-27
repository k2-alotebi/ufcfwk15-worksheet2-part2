#ifndef INCLUDE_INPUT_BUFFER_H
#define INCLUDE_INPUT_BUFFER_H

#include "types.h"

/** getc:
 * Gets a single character from the input buffer.
 * Returns 0 if buffer is empty.
 *
 * @return The character read, or 0 if buffer is empty
 */
u8int getc(void);

/** readline:
 * Reads a line from the input buffer until a newline is encountered.
 * The line (without the newline) is stored in the provided buffer.
 *
 * @param buffer The buffer to store the line
 * @param max_len Maximum length to read (including null terminator)
 * @return Number of characters read (not including null terminator), or -1 if error
 */
s32int readline(char *buffer, u32int max_len);

/** input_buffer_available:
 * Checks if there are characters available in the input buffer.
 *
 * @return 1 if characters are available, 0 otherwise
 */
u8int input_buffer_available(void);

#endif /* INCLUDE_INPUT_BUFFER_H */
