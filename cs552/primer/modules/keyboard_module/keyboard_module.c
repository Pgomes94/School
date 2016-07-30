/*
 * Patrick Gomes 
 * U02663517 pgomes94@bu.edu
 * CS 552 - Operating Systems - Primer 
 *
 * keyboard_module.c
 * interrupt driven keyboard module
 */
 
 #include <linux/kernel.h>
 #include <linux/module.h>
 #include <linux/init.h>
 #include <linux/errno.h>
 #include <linux/proc_fs.h>
 #include <asm/uaccess.h>
 #include <linux/tty.h>
 #include <linux/sched.h>
 #include <linux/interrupt.h> // IRQF_SHARED
 
 MODULE_LICENSE("GPL");

 #define KEYBOARD_TEST _IOR(0, 6, struct keyboard_char_t)

 DECLARE_WAIT_QUEUE_HEAD(keyboard_wq);
 
 /* function definitions */
 irqreturn_t keyboard_handler(int irq, void* dev_id);
 void my_printk(char *string);
 static int pseudo_device_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);

/* structs for keyboard information */
 struct keyboard_char_t {
    char character;
 };

 struct keyboard_buffer_t {
    short shiftFlag;
    short readyFlag;
    char character;
 };

 /* global variables */
 static struct file_operations pseudo_dev_proc_operations;
 static struct proc_dir_entry *proc_entry;
 static struct keyboard_char_t keyboard_char;
 static struct keyboard_buffer_t keyboard_buffer;
 static char keyboard_module_identifier[] = "identifier";

 static int __init startup_routine(void) {
    printk( KERN_INFO "Starting up interrupt driven keyboard module.\n");

    request_irq(1, keyboard_handler, IRQF_SHARED, 
        "keyboard_module", (void*) &keyboard_module_identifier);
    
    pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;
    proc_entry = create_proc_entry("keyboard_test", 0444, NULL);
    if (!proc_entry) {
        printk( KERN_INFO "Error creating /proc entry. \n"); 
        return 1;
    }
    proc_entry->proc_fops = &pseudo_dev_proc_operations;
    
    return 0;
 }

 static void __exit cleanup_routine(void) {
    printk( KERN_INFO "Cleaning up interrupt driven keyboard module");
    remove_proc_entry("keyboard_test", NULL);
    free_irq(1, &keyboard_module_identifier);
    return;
 }

 /* printk that prints to active tty */
 void my_printk(char *string) {
    struct tty_struct *my_tty;
    my_tty = current->signal->tty;
    
    if (my_tty != NULL) {
        (*my_tty->driver->ops->write)(my_tty, string, strlen(string));
        (*my_tty->driver->ops->write)(my_tty, "\015\012", 2); /* equivalent to \r\n */
    }
 }

 static inline unsigned char inb( unsigned short usPort ) {
    unsigned char uch;
    asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
    return uch;
 }

 /*
 * ioctl() entry point
 */
 static int pseudo_device_ioctl(struct inode *inode, struct file *file,
                unsigned int cmd, unsigned long arg) {
    unsigned long copied;

    switch(cmd) {
        case KEYBOARD_TEST:
            wait_event_interruptible(keyboard_wq,(keyboard_buffer.readyFlag==1));
            keyboard_char.character = keyboard_buffer.character;
            keyboard_buffer.readyFlag = 0;
            copied = copy_to_user((struct keyboard_char_t *) arg, &keyboard_char,
                    sizeof(struct keyboard_char_t));
            printk (KERN_INFO "keyboard driver called, %c returned, %u bytes not copied",
                    keyboard_char.character, copied);
            break;
        default:
            return -EINVAL;
            break;
    }

    return 0;
}

irqreturn_t keyboard_handler(int irq, void* dev_id) {
    char c;
    static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

    c = inb(0x60);
    if (c & 0x80) { // key was released
        printk( KERN_INFO "Key was released\n");
        c &= 0x7F;
        if (c == 0x2a || c == 0x36) { // shift key
            keyboard_buffer.shiftFlag = 0;
        }
        return IRQ_HANDLED;
    } else {
        printk( KERN_INFO "Key was pressed\n");
        if (c == 0x2a || c == 0x36) { // shift key
            keyboard_buffer.shiftFlag = 1;
            return IRQ_HANDLED;
        }
        keyboard_buffer.character = scancode[(int)c];
        if (c == 0x0E) { // backspace key
            keyboard_buffer.character = 0x08;
        } else if (c == 0x1C) { // handling enter shift
            keyboard_buffer.character = '\n';
        } else if (keyboard_buffer.shiftFlag) {
            if (keyboard_buffer.character >= 0x61 && keyboard_buffer.character <= 0x7D) { // is a letter
                keyboard_buffer.character -= 0x20;
            }
        }
    }
    keyboard_buffer.readyFlag = 1;
    wake_up_interruptible(&keyboard_wq);
    return IRQ_HANDLED;
}


 module_init(startup_routine);
 module_exit(cleanup_routine);

