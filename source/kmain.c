#include "drivers/frame_buffer.h"
#include "drivers/interrupts.h"
#include "drivers/hardware_interrupt_enabler.h"
#include "drivers/terminal.h"

/* Main kernel function called from loader.asm */
void kmain()
{
    /* Initialize interrupts */
    interrupts_install_idt();
    
    /* Enable hardware interrupts */
    enable_hardware_interrupts();
    
    /* Initialize and run terminal */
    terminal_init();
    terminal_run();
    
    /* Should never reach here */
    while(1) {
        __asm__ volatile("hlt");
    }
}
