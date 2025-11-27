
#include "test-part2.h"


void test_getc() {
    // Test 1: Empty buffer
    u8int c = getc();
    if (c == 0) {
        fb_write_string("Test 1 Passed: Empty buffer handled correctly.\n");
    } else {
        fb_write_string("Test 1 Failed: Empty buffer not handled.\n");
    }

    // Simulate adding characters to the buffer
    input_buffer.buffer[input_buffer.write_index++] = 'A';
    input_buffer.count++;
    input_buffer.buffer[input_buffer.write_index++] = 'B';
    input_buffer.count++;
    input_buffer.buffer[input_buffer.write_index++] = 'C';
    input_buffer.count++;

    // Test 2: Retrieve characters
    c = getc();
    if (c == 'A') {
        fb_write_string("Test 2 Passed: Retrieved first character correctly.\n");
    } else {
        fb_write_string("Test 2 Failed: First character incorrect.\n");
    }

    // Test 3: Circular buffer logic
    input_buffer.read_index = INPUT_BUFFER_SIZE - 1;  // Simulate near-wrap condition
    input_buffer.buffer[input_buffer.read_index] = 'Z';
    input_buffer.count++;

    c = getc();
    if (c == 'Z') {
        fb_write_string("Test 3 Passed: Circular buffer logic handled correctly.\n");
    } else {
        fb_write_string("Test 3 Failed: Circular buffer logic incorrect.\n");
    }
}


void test_readline() {
    char line[10];  // Limit buffer to 10 characters for testing

    // Simulate adding characters to the input buffer
    input_buffer.buffer[input_buffer.write_index++] = 'H';
    input_buffer.count++;
    input_buffer.buffer[input_buffer.write_index++] = 'e';
    input_buffer.count++;
    input_buffer.buffer[input_buffer.write_index++] = 'l';
    input_buffer.count++;
    input_buffer.buffer[input_buffer.write_index++] = 'l';
    input_buffer.count++;
    input_buffer.buffer[input_buffer.write_index++] = 'o';
    input_buffer.count++;
    input_buffer.buffer[input_buffer.write_index++] = '\n';
    input_buffer.count++;

    // Test 1: Read a line
    readline(line, sizeof(line));
    if (strcmp(line, "Hello") == 0) {
        fb_write_string("Test 1 Passed: Line read correctly.\n");
    } else {
        fb_write_string("Test 1 Failed: Line incorrect.\n");
    }

    // Test 2: Buffer overflow
    input_buffer.write_index = 0;
    input_buffer.count = 0;
    for (int i = 0; i < 15; i++) {
        input_buffer.buffer[input_buffer.write_index++] = 'X';
        input_buffer.count++;
    }
    input_buffer.buffer[input_buffer.write_index++] = '\n';
    input_buffer.count++;

    readline(line, sizeof(line));
    if (strlen(line) == sizeof(line) - 1) {
        fb_write_string("Test 2 Passed: Buffer overflow handled correctly.\n");
    } else {
        fb_write_string("Test 2 Failed: Buffer overflow incorrect.\n");
    }

    // Test 3: Empty line input
    input_buffer.buffer[input_buffer.write_index++] = '\n';
    input_buffer.count++;
    readline(line, sizeof(line));
    if (strlen(line) == 0) {
        fb_write_string("Test 3 Passed: Empty line handled correctly.\n");
    } else {
        fb_write_string("Test 3 Failed: Empty line incorrect.\n");
    }
}

