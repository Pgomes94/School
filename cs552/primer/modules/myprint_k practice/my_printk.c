/*
 * ioctl test module
 */
 
 #include <linux/kernel.h>
 #include <linux/module.h>
 #include <linux/init.h>
 #include <linux/errno.h>
 #include <linux/proc_fs.h>
 #include <linux/uaccess.h>
 #include <linux/tty.h>
 #include <linux/sched.h>
 
 MODULE_LICENSE("GPL");
 
 /* attribute sections */
 struct ioctl_test_t {
 	int field1;
 	char *field2;
	char field3;
 };
 
 #define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)
 #define KEYBOARD_TEST _IOWR(0, 7, struct ioctl_test_t)
  
 static int pseudo_device_ioctl(struct inode *inode, struct file *file,
 				unsigned int cmd, unsigned long arg);
 				
 static struct file_operations pseudo_dev_proc_operations;
 static struct proc_dir_entry *proc_entry;
 
 static int __init initialization_routine(void) {
 	printk(KERN_INFO "Loading Module\n");
        	
 	pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;
 	
 	/* start creating proc entry */
 	proc_entry = create_proc_entry("ioctl_test", 0444, NULL);
 	if (!proc_entry) {
 		printk( KERN_INFO "Error creating /proc entry. \n"); 
 		return 1;
 	}
 	proc_entry->proc_fops = &pseudo_dev_proc_operations;
 	return 0;
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

 static void __exit cleanup_routine(void) {
	printk (KERN_INFO "Dumping module.\n");
	remove_proc_entry("ioctl_test", NULL);
	return ;
 }

 static inline unsigned char inb( unsigned short usPort ) {
	unsigned char uch;
        asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
	return uch;
 }
               
 static inline void outb( unsigned char uch, unsigned short usPort ) {
        asm volatile( "outb %0,%1" : : "a" (uch), "Nd" (usPort) );
 }
 
 char my_getchar ( void ) {
	char c;
    	static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
     
        /* Poll keyboard status register at port 0x64 checking bit 0 to see if
         * output buffer is full. We continue to poll if the msb of port 0x60
         * (data port) is set, as this indicates out-of-band data or a release
         * keystroke
         */
         while((c = inb(0x60)) == 0x1C);
         
         printk (KERN_INFO "%c", c);
          
         return scancode[ (int)c ];
 }

/*
 * ioctl() entry point
 */
 
 static int pseudo_device_ioctl(struct inode *inode, struct file *file,
 				unsigned int cmd, unsigned long arg) {
 	struct ioctl_test_t ioc;
 	
 	switch(cmd) {
 		case IOCTL_TEST:
 			copy_from_user(&ioc, (struct ioctl_test_t *)arg,
 					sizeof(struct ioctl_test_t));
 			printk (KERN_INFO "ioctl: call to IOCTL_TEST (%d,%s)!\n",
 					ioc.field1, ioc.field2);
			my_printk("Got msg in kernel.\n");
			my_printk(ioc.field2);
			break;
		case KEYBOARD_TEST:
			ioc.field3 = my_getchar();
			copy_to_user((struct ioctl_test_t *) arg, &ioc,
					 sizeof(struct ioctl_test_t));
			my_printk("Field 3 is");
			my_printk(&ioc.field3);
			break;
		default:
			return -EINVAL;
			break;
	}
	
	return 0; 			 	
 } 
 
 module_init(initialization_routine);
 module_exit(cleanup_routine);
