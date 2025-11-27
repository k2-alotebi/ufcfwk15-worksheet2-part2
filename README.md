# Worksheet 2 Part 2 - Inputs & Interrupts

**UFCFWK-15-2 Operating Systems**  
**Student:** Khaled Alotebi  
**Repository:** https://github.com/k2-alotebi/ufcfwk15-worksheet2-part2

## Overview

This undertaking incorporates the management of keyboard input via interrupts, an input buffer API, and a simple terminal interface for Tiny OS. The realization is in accord with Chapter 6 of the OS Development handbook, where interrupt handling and basic I/O operations are being dealt with.

## Project Structure

```
worksheet2-part2/
├── drivers/
│   ├── types.h                   
│   ├── io.h, io.s                
│   ├── pic.h, pic.c               
│   ├── interrupts.h               
│   ├── interrupts.c              
│   ├── interrupt_handlers.s        
│   ├── interrupt_asm.s            
│   ├── hardware_interrupt_enabler.h, .s 
│   ├── keyboard.h, keyboard.c       
│   ├── frame_buffer.h, frame_buffer.c     
│   ├── input_buffer.h, input_buffer.c     
│   └── terminal.h, terminal.c     
├── source/
│   ├── loader.asm                
│   ├── kmain.c                    
│   └── link.ld                   
├── iso/boot/grub/
│   ├── menu.lst                   
│   └── stage2_eltorito           
└── Makefile                     
```

## Task 1: Display Keyboard Input (20%)

### Implementation

The processing of keyboard input takes place via interrupt 33 (IRQ1 changed to 0x21). A key press causes the keyboard controller to emit an interrupt, our interrupt handler then takes care of it.
#### Key Components

**1. PIC (Programmable Interrupt Controller) Setup**

PIC was remapped to avoid conflict with CPU exception:
```c
void pic_remap(s32int offset1, s32int offset2) {
    // Initialize PICs
    outb(0x20, 0x11);  // ICW1 to PIC1
    outb(0xA0, 0x11);  // ICW1 to PIC2
    
    // Set interrupt offsets
    outb(0x21, offset1);  // PIC1 offset (0x20)
    outb(0xA1, offset2);  // PIC2 offset (0x28)
    
    // Configure cascade mode
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    
    // Set 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // Mask all interrupts initially
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}
```

**2. Interrupt Descriptor Table (IDT)**

IDT is set up to handle keyboard interrupts:

```c
void interrupts_install_idt() {
    interrupts_init_descriptor(INTERRUPTS_KEYBOARD, (u32int) interrupt_handler_33);
    
    idt.address = (s32int) &idt_descriptors;
    idt.size = sizeof(struct IDTDescriptor) * INTERRUPTS_DESCRIPTOR_COUNT;
    load_idt((s32int) &idt);
    
    pic_remap(PIC_1_OFFSET, PIC_2_OFFSET);
    
    // Unmask keyboard interrupt (IRQ1)
    outb(0x21, inb(0x21) & ~(1 << 1));
}
```

**3. Keyboard Interrupt Handler**

The interrupt handler reads scan codes and converts them to ASCII:

```c
void interrupt_handler(struct cpu_state cpu, u32int interrupt, struct stack_state stack) {
    switch (interrupt) {
        case INTERRUPTS_KEYBOARD:
            input = keyboard_read_scan_code();
            
            if (!(input & 0x80)) {  // Key press, not release
                ascii = keyboard_scan_code_to_ascii(input);
                if (ascii != 0) {
                    if (ascii == '\b') {
                        fb_backspace();
                    } else if (ascii == '\n') {
                        fb_newline();
                    } else {
                        fb_write_char(ascii);
                    }
                }
            }
            pic_acknowledge(interrupt);
            break;
    }
}
```

**4. Scan Code to ASCII Conversion**

The keyboard driver converts scan codes to ASCII characters:

```c
u8int keyboard_scan_code_to_ascii(u8int scan_code) {
    if (scan_code & 0x80) {
        return 0;  // Ignore key releases
    }
    
    switch(scan_code) {
        case 0x02: return '1';
        case 0x10: return 'q';
        case 0x1C: return '\n';  // Enter
        case 0x0E: return '\b';  // Backspace
        // ... more mappings
    }
}
```

### Features Implemented

-  Keyboard interrupt handling
-  Real-time character display
-  Backspace functionality
-  Enter/newline handling
-  Input buffer for future use

### Screenshots
<img width="754" height="206" alt="Screenshot 2025-11-27 041339" src="https://github.com/user-attachments/assets/e9da047f-bc1d-48a1-b0b2-610ec47e934e" />
<img width="733" height="231" alt="Screenshot 2025-11-27 041345" src="https://github.com/user-attachments/assets/a72ab989-bc9a-45fd-9a98-8dabb7f550a0" />
<img width="649" height="270" alt="Screenshot 2025-11-27 041437" src="https://github.com/user-attachments/assets/8f9dfc81-016f-47ab-83b6-8976be2b91db" />


---

## Task 2: Input Buffer API (20%)

### Implementation

The input buffer API offers some facilities in the built-in API to read characters from the keyboard buffer that was maintained for propagation by the interrupt handler mechanism.
#### Key Functions

**1. `getc()` - Read Single Character**

```c
u8int getc(void) {
    u8int character = 0;
    
    __asm__ volatile("cli");  // Disable interrupts
    
    if (buffer_count > 0) {
        character = input_buffer[buffer_index];
        buffer_index = (buffer_index + 1) % INPUT_BUFFER_SIZE;
        buffer_count--;
    }
    
    __asm__ volatile("sti");  // Re-enable interrupts
    
    return character;
}
```

**Key Features:**
- Thread-safe (disables interrupts during read)
- Returns 0 if buffer is empty
- Uses circular buffer implementation
- Properly manages buffer indices

**2. `readline()` - Read Full Line**

```c
s32int readline(char *buffer, u32int max_len) {
    u32int i = 0;
    u8int c;
    
    if (buffer == 0 || max_len == 0) {
        return -1;
    }
    
    while (i < (max_len - 1)) {
        // Wait for characters
        while (buffer_count == 0) {
            __asm__ volatile("hlt");
        }
        
        c = getc();
        
        if (c != 0) {
            if (c == '\n' || c == '\r') {
                buffer[i] = '\0';
                return (s32int)i;
            }
            
            buffer[i] = c;
            i++;
        }
    }
    
    buffer[max_len - 1] = '\0';
    return (s32int)(max_len - 1);
}
```

**Key Features:**
- Reads until newline is encountered
- Handles buffer overflow safely
- Waits for input when buffer is empty
- Returns length of line read

**3. Buffer Management**

The buffer uses a circular buffer implementation:

```c
#define INPUT_BUFFER_SIZE 256

u8int input_buffer[INPUT_BUFFER_SIZE];
u8int buffer_index = 0;      // Read position
u8int buffer_count = 0;       // Number of characters in buffer

// Write position = (buffer_index + buffer_count) % INPUT_BUFFER_SIZE
```

### Testing

The functions were tested with the following code:

```c
char line_buffer[256];
s32int len;

len = readline(line_buffer, 256);
if (len > 0) {
    fb_puts("You typed: ");
    fb_puts(line_buffer);
    fb_puts("\nLength: ");
    // Display length...
}
```

**Test Results:**
-  `getc()` correctly reads individual characters
-  `readline()` reads full lines until Enter
-  Buffer handles overflow correctly
-  Empty buffer returns appropriate values

### Screenshots

<img width="1132" height="72" alt="image" src="https://github.com/user-attachments/assets/ed5a7e5a-3ac9-4273-921f-dae945bb6541" />
<img width="1124" height="671" alt="image" src="https://github.com/user-attachments/assets/eeb6ad49-3a91-4170-8d33-b14312452f69" />


---

## Task 3: Terminal Implementation (40%)

### Implementation

A basic terminal interface that processes user commands, similar to a Unix shell.

#### Command Structure

Commands are defined using a structure:

```c
struct command {
    const char* name;
    void (*function)(char* args);
};

struct command commands[] = {
    {"echo", cmd_echo},
    {"clear", cmd_clear},
    {"help", cmd_help},
    {"version", cmd_version},
    {"shutdown", cmd_shutdown},
    {0, 0}  // End marker
};
```

#### Command Parsing

Input is parsed into command and arguments:

```c
void terminal_parse_command(char* input, char* command, char* args) {
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
```

#### Command Execution

Commands are looked up and executed:

```c
void terminal_execute_command(char* input) {
    char command[MAX_COMMAND_LEN];
    char args[MAX_ARGS_LEN];
    
    terminal_parse_command(input, command, args);
    
    if (command[0] == '\0') {
        return;  // Empty command
    }
    
    // Find command in table
    u32int i = 0;
    while (commands[i].name != 0) {
        if (strcmp(commands[i].name, command) == 0) {
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
```

#### Implemented Commands

**1. `echo [text]`**
```c
void cmd_echo(char* args) {
    if (args[0] == '\0') {
        fb_puts("\n");
    } else {
        fb_puts(args);
        fb_puts("\n");
    }
}
```

**2. `clear`**
```c
void cmd_clear(char* args) {
    (void)args;
    fb_clear();
}
```

**3. `help`**
```c
void cmd_help(char* args) {
    (void)args;
    fb_puts("\nAvailable commands:\n");
    fb_puts("  echo [text]    - Display the provided text\n");
    fb_puts("  clear          - Clear the screen\n");
    fb_puts("  help           - Show this help message\n");
    fb_puts("  version        - Display OS version\n");
    fb_puts("  shutdown       - Prepare system for shutdown\n\n");
}
```

**4. `version`**
```c
void cmd_version(char* args) {
    (void)args;
    fb_puts("\nTiny OS v1.0\n");
    fb_puts("Worksheet 2 Part 2 - Terminal Implementation\n");
    fb_puts("Built with keyboard input and interrupt handling\n\n");
}
```

**5. `shutdown`**
```c
void cmd_shutdown(char* args) {
    (void)args;
    fb_puts("\nSystem shutdown requested.\n");
    fb_puts("In a real OS, this would save data and power off.\n");
    fb_puts("For now, the system will continue running.\n\n");
}
```

#### Terminal Main Loop

```c
void terminal_run(void) {
    char input[256];
    s32int len;
    
    while (1) {
        fb_puts("myos> ");  // Display prompt
        
        len = readline(input, 256);  // Read user input
        
        if (len > 0) {
            terminal_execute_command(input);  // Execute command
        }
        
        __asm__ volatile("hlt");  // Wait for next interrupt
    }
}
```

### Features

-  Command prompt display
-  Command parsing (command + arguments)
-  Command lookup and execution
-  Error handling for unknown commands
-  5 commands implemented
-  Clean command structure for easy extension

### Screenshots

<img width="722" height="547" alt="Screenshot 2025-11-27 043955" src="https://github.com/user-attachments/assets/06e6c6af-ad20-40c2-8ffc-90e8db70702a" />
and this after the clear
<img width="637" height="149" alt="Screenshot 2025-11-27 044012" src="https://github.com/user-attachments/assets/1f2698d1-3349-472a-8369-a112da3d7f60" />

---

## Building and Running

### Prerequisites

- NASM (Netwide Assembler)
- GCC with 32-bit support
- GNU LD (linker)
- genisoimage
- QEMU
- GRUB stage2_eltorito

### Build Instructions

```bash
cd ~/worksheet2-part2
make clean
make all
```

### Running

**Curses mode (recommended for framebuffer):**
```bash
make run-simple
```

To quit: Press `ESC` then `2`, then type `quit` and press Enter.

**With telnet monitor:**
```bash
make run-curses
```

In another terminal:
```bash
telnet localhost 45454
quit
```

---

## Technical Details

### Interrupt Handling Flow

1. Keyboard key pressed -> Hardware generates IRQ1
2. PIC remaps IRQ1 to interrupt 33 (0x21)
3. CPU looks up interrupt 33 in IDT
4. Jumps to `interrupt_handler_33` (assembly stub)
5. Assembly stub saves registers and calls `interrupt_handler()` (C function)
6. C handler reads scan code from keyboard port (0x60)
7. Converts scan code to ASCII
8. Displays character or handles special keys (backspace, enter)
9. Adds character to input buffer
10. Acknowledges interrupt to PIC
11. Returns to interrupted code

### Buffer Management

The input buffer uses a circular buffer:
- **Write position**: `(buffer_index + buffer_count) % INPUT_BUFFER_SIZE`
- **Read position**: `buffer_index`
- **Count**: `buffer_count` tracks number of characters

This ensures efficient FIFO (First In, First Out) operation.

### Command Parsing Algorithm

1. Skip leading whitespace
2. Extract command name until space or end of string
3. Skip whitespace after command
4. Extract remaining text as arguments
5. Lookup command in command table
6. Execute command function with arguments

---

## Challenges and Solutions

### Challenge 1: Keyboard Interrupts Not Firing

**Problem:** Keyboard input wasn't being detected.

**Solution:** 
- Ensured PIC was properly remapped
- Verified keyboard interrupt (IRQ1) was unmasked
- Confirmed interrupts were enabled with `sti` instruction
- Fixed buffer write position calculation

### Challenge 2: Buffer Read/Write Synchronization

**Problem:** `getc()` and `readline()` weren't reading characters correctly.

**Solution:**
- Aligned read index with write position
- Used `buffer_index` for both reading and as base for writing
- Implemented proper circular buffer arithmetic
- Added interrupt disabling during critical sections

### Challenge 3: Command Parsing

**Problem:** Commands with arguments weren't being parsed correctly.

**Solution:**
- Implemented proper string parsing with whitespace handling
- Separated command extraction from argument extraction
- Added bounds checking to prevent buffer overflow

---

## Testing

### Task 1 Testing
-  Typed various characters - all displayed correctly
-  Backspace deleted characters
-  Enter created new lines
-  Special characters handled properly

### Task 2 Testing
-  `getc()` reads individual characters
-  `readline()` reads full lines
-  Empty buffer handled correctly
-  Buffer overflow prevented

### Task 3 Testing
-  All 5 commands work correctly
-  Command parsing handles arguments
-  Unknown commands show error message
-  Empty commands handled gracefully

---

## Future Improvements

1. **Command History**: Implement up/down arrow key support
2. **Tab Completion**: Auto-complete commands
3. **Color Support**: Add colors to terminal output
4. **More Commands**: Add file system commands (ls, cat, etc.)
5. **Input Editing**: Better cursor movement and editing

---

## References

- The Little Book of OS Development - Chapter 6
- OSDev Wiki: https://wiki.osdev.org/
- Worksheet 2 Part 2 Assignment Specification

---

## Conclusion

This project successfully implements:
-  Keyboard interrupt handling
-  Input buffer API with `getc()` and `readline()`
-  Full terminal interface with command processing

All three tasks are complete and fully functional. The terminal provides a solid foundation for future OS development.

---

**Date Completed:** November 2025  
**Status:**  Complete

