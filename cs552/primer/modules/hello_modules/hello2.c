/*
 * hello2.c - Demonstrating the module_init() and module_exit() macros.
 * This is the current preference for writing modules.
 */
 
 #include <linux/module.h>   /*Needed by all modules.*/
 #include <linux/kernel.h>   /*Needed for KERN_INFO*/
 #include <linux/init.h>     /*Needed for the macros module_init and module_exit*/
 
 static int __init hello_2_init(void) {
 	printk (KERN_INFO "Hello world 2!\n");
 	return 0;
 }
 
 static void __exit hello_2_exit(void) {
 	printk (KERN_INFO "Goodbye world 2!\n");
 }
 
 module_init(hello_2_init);
 module_exit(hello_2_exit);
