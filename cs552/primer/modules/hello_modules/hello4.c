/* 
 * hello4.c - demonstrating module documentation.
 */
 
 #include <linux/module.h>
 #include <linux/kernel.h>
 #include <linux/init.h>
 
 #define DRIVER_AUTHOR "Patrick Gomes"
 #define DRIVER_DESC "Testing making kernels"
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR(DRIVER_AUTHOR);
 MODULE_DESCRIPTION(DRIVER_DESC);
 
 static int __init hello_4_init(void) {
 	printk (KERN_INFO "Hello world 4!\n");
 	return 0;
 }
 
 static void __exit hello_4_exit(void) {
 	printk (KERN_INFO "Goodbye world 4!\n");
 }
 
 module_init(hello_4_init);
 module_exit(hello_4_exit);
