/*
 * start.c - starting a multi file kernel
 */
 
 #include <linux/module.h>
 #include <linux/kernel.h>
 
 int init_module(void) {
 	printk(KERN_INFO "starting multi file kernel.\n");
 	return 0;
 }
