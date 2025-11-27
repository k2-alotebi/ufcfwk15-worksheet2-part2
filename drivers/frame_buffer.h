#ifndef INCLUDE_FB_H
#define INCLUDE_FB_H

/* Framebuffer colors */
#define FB_BLACK         0
#define FB_BLUE          1
#define FB_GREEN         2
#define FB_CYAN          3
#define FB_RED           4
#define FB_MAGENTA       5
#define FB_BROWN         6
#define FB_LIGHT_GREY    7
#define FB_DARK_GREY     8
#define FB_LIGHT_BLUE    9
#define FB_LIGHT_GREEN   10
#define FB_LIGHT_CYAN    11
#define FB_LIGHT_RED     12
#define FB_LIGHT_MAGENTA 13
#define FB_LIGHT_BROWN   14
#define FB_WHITE         15

/* Framebuffer dimensions */
#define FB_WIDTH  80
#define FB_HEIGHT 25

/** fb_write_cell:
 * Writes a character with the given foreground and background to position i
 * in the framebuffer.
 *
 * @param i  The location in the framebuffer
 * @param c  The character
 * @param fg The foreground color
 * @param bg The background color
 */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);

/** fb_move_cursor:
 * Moves the cursor of the framebuffer to the given position
 *
 * @param pos The new position of the cursor
 */
void fb_move_cursor(unsigned short pos);

/** fb_write:
 * Writes the contents of the buffer buf of length len to the screen
 *
 * @param buf The buffer to write
 * @param len The length of the buffer
 * @return 0 on success
 */
int fb_write(char *buf, unsigned int len);

/** fb_move:
 * Moves the cursor to the given x,y position
 *
 * @param x The x position (column)
 * @param y The y position (row)
 */
void fb_move(unsigned short x, unsigned short y);

/** fb_clear:
 * Clears the entire framebuffer
 */
void fb_clear(void);

/** fb_putc:
 * Writes a single character at the current cursor position
 *
 * @param c The character to write
 */
void fb_putc(char c);

/** fb_puts:
 * Writes a null-terminated string at the current cursor position
 *
 * @param str The string to write
 */
void fb_puts(char *str);

/** fb_backspace:
 * Removes the last character from the screen
 */
void fb_backspace(void);

/** fb_newline:
 * Moves to a new line
 */
void fb_newline(void);

/** fb_write_char:
 * Writes a single character (alias for fb_putc for compatibility)
 *
 * @param c The character to write
 */
void fb_write_char(char c);

#endif /* INCLUDE_FB_H */
