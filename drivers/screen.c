#include "screen.h"
#include "../cpu/ports.h"
#include "../libc/mem.h"

// Declaration of private methods
int get_cursor_offset();
void set_cursor_offset(int offset);
int print_char(char c, int col, int row, char attr);
int get_offset(int col, int row);
int get_offset_row(int offset);
int get_offset_col(int offset);

// Public kernel API functions

// Print a message on the specified location
// If col or row is negative, use current offset
void kprint_at(char *message, int col, int row){
    // Set cursor if col or row is negative
    int offset;
    if(col >= 0 && row >= 0){
        offset = get_offset(col, row);
    }
    else{
        offset = get_cursor_offset();
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }

    // Loop throught message and print it
    int i = 0;
    while(message[i] != 0){
        offset = print_char(message[i++], col, row, WHITE_ON_BLACK);
        // Compute col and row for next iteration
        row = get_offset_row(offset);
        col = get_offset_col(offset);
    }
}

// Print a message on current offset
void kprint(char *message){
    kprint_at(message, -1, -1);
}

// Clear the full screen
void clear_screen(){
    int screen_size = MAX_COLS * MAX_ROWS;
    int i;
    char *screen = VIDEO_ADDRESS;

    for(i = 0; i < screen_size; i++){
        screen[i*2] = ' ';
        screen[i*2+1] = WHITE_ON_BLACK;
    }
    set_cursor_offset(get_offset(0, 0));
}

// Print a backspace ("un-print")
void kprint_backspace(){
    int offset = get_cursor_offset()-2;
    int row = get_offset_row(offset);
    int col = get_offset_col(offset);
    print_char(0x08, col, row, WHITE_ON_BLACK);
}

// Private functions declaration

// Innermost print funcion, directly accesses the video memory
// If col and row are negative, we will print at the current cursor location
// If attr is zero it will use 'white on black' as default
// Returns the offset of the next character 
// Sets the video cursor to the returned offset
int print_char(char c, int col, int row, char attr){
    unsigned char *vidmem = (unsigned char*) VIDEO_ADDRESS;
    if(!attr) attr = WHITE_ON_BLACK;

    // Error handling: print a red 'E' if the coordinates are invalid
    if(col >= MAX_COLS || row >= MAX_ROWS){
        vidmem[2*(MAX_COLS)*(MAX_ROWS)-2] = 'E';
        vidmem[2*(MAX_COLS)*(MAX_ROWS)-1] = RED_ON_WHITE;
        return get_offset(col, row);
    }

    int offset;
    if(col >= 0 && row >= 0) offset = get_offset(col, row);
    else offset = get_cursor_offset();
    
    if(c == '\n'){
        row = get_offset_row(offset);
        offset = get_offset(0, row+1);
    }
    else if(c == 0x08){
        vidmem[offset] = ' ';
        vidmem[offset+1] = attr;
    }
    else{
        vidmem[offset] = c;
        vidmem[offset+1] = attr;
        offset += 2;
    }

    // Check if the offset is over screen and scroll
    if(offset >= MAX_ROWS * MAX_COLS * 2){
        int i;
        for(i = 0; i < MAX_ROWS; i++){
            memory_copy((uint8_t *)(get_offset(0, i) + VIDEO_ADDRESS), (uint8_t *)(get_offset(0, i-1) + VIDEO_ADDRESS), MAX_COLS * 2);
        }

        char *last_line = get_offset(0, MAX_ROWS-1) + VIDEO_ADDRESS;
        for(i = 0; i < MAX_COLS * 2; i++) last_line[i] = 0;
        
        offset -= 2 * MAX_COLS;
    }

    set_cursor_offset(offset);
    return offset;
}

// Use VGA ports to get the current cursor position
int get_cursor_offset(){
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8; // High byte
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA); // Low byte
    return offset * 2;
}

// Use VGA ports to set the current cursor position
void set_cursor_offset(int offset){
    offset /= 2;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

// Get screen offset from col and row
int get_offset(int col, int row){
    return 2 * (row * MAX_COLS + col);
}

//Get screen row from offset
int get_offset_row(int offset){
    return offset / (2 * MAX_COLS);
}

//Get screen col from offset
int get_offset_col(int offset){
    return (offset - (get_offset_row(offset)*2*MAX_COLS))/2;
}