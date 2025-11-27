#include "io.h"
#include "pic.h"
#include "types.h"

#define PIC1_PORT_A 0x20
#define PIC2_PORT_A 0xA0

/* The PIC interrupts have been remapped */
#define PIC1_START_INTERRUPT 0x20
#define PIC2_START_INTERRUPT 0x28
#define PIC2_END_INTERRUPT PIC2_START_INTERRUPT + 7
#define PIC_ACK 0x20

/** pic_acknowledge:
 * Acknowledges an interrupt from either PIC 1 or PIC 2.
 *
 * @param num The number of the interrupt
 */
void pic_acknowledge(u32int interrupt)
{
    if (interrupt < PIC1_START_INTERRUPT || interrupt > PIC2_END_INTERRUPT) 
    {
        return;
    }
    if (interrupt < PIC2_START_INTERRUPT) 
    {
        outb(PIC1_PORT_A, PIC_ACK);
    } 
    else 
    {
        outb(PIC2_PORT_A, PIC_ACK);
    }
}

void pic_remap(s32int offset1, s32int offset2) {
    // Initialize the PIC by sending the ICW1
    outb(0x20, 0x11);  // Send ICW1 to PIC1
    outb(0xA0, 0x11);  // Send ICW1 to PIC2

    // Set the offset for IRQs in PIC1 and PIC2
    outb(0x21, offset1);  // Set PIC1 offset (IRQ 0-7)
    outb(0xA1, offset2);  // Set PIC2 offset (IRQ 8-15)

    // Tell PICs to communicate with each other (cascade mode)
    outb(0x21, 0x04);  // Tell PIC1 that PIC2 is on IRQ 2
    outb(0xA1, 0x02);  // Tell PIC2 that PIC1 is on IRQ 2

    // Set the mode to 8086 mode
    outb(0x21, 0x01);  // Set PIC1 to 8086 mode
    outb(0xA1, 0x01);  // Set PIC2 to 8086 mode

    // Mask all interrupts initially (we'll unmask only what we need)
    outb(0x21, 0xFF);  // Mask all IRQs on PIC1
    outb(0xA1, 0xFF);  // Mask all IRQs on PIC2
}
