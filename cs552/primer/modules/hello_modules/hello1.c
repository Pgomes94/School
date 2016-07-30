/*
 * hello1.c -simplest kernel module
 */
 #include <linux/module.h> /*Needed by all modules*/
 #include <linux/kernel.h> /*Needed for KERN_INFO*/
 
 int init_module(void) {
   printk(KERN_INFO "Hello World 1.\n");
   
   /*
    * A non-zero return means init_module failed; module wasn't loaded.
    */
    
    return 0;
 }
 
 void cleanup_module(void) {
   printk(KERN_INFO "Goodbye world 1.\n");
 }
