#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "kernel.h"
#include "../libc/string.h"
#include "../libc/mem.h"

static int kernel_running;

void kernel_main() {
    kernel_running = 1;

    isr_install();
    irq_install();

    clear_screen();

    kprint("Type something, it will go throught the kernel! (END to stop)\n>");

    while(kernel_running){}
}

void user_input(char *input){
    if(strcmp(input, "END") == 0){
        clear_screen();
        kprint("Stopping... Bye!\n");
        kernel_running = 0;
        return;
    }
    else if(strcmp(input, "PAGE") == 0){
        uint32_t phys_addr;
        uint32_t page = kmalloc(1000, 1, &phys_addr);
        char page_str[16] = "";
        hex_to_ascii(page, page_str);
        char phys_str[16] = "";
        hex_to_ascii(phys_addr, phys_str);
        kprint("Page: ");
        kprint(page_str);
        kprint(", physical address: ");
        kprint(phys_str);
        kprint("\n");
    }
    kprint("You said: ");
    kprint(input);
    kprint("\n>");
}