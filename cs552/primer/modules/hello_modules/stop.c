/*
 * stop.c - stopping multi file kernel
 */
 
 #include <linux/kernel.h>
 #include <linux/module.h>
 
 void cleanup_module(void) {
 	printk(KERN_INFO "closing multi file kenrel");
 }
