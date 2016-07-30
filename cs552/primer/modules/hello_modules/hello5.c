/*
 * hello5.c - Demonstrates command line argument passing to kernel modules.
 */
 
 #include <linux/module.h>
 #include <linux/moduleparam.h>
 #include <linux/kernel.h>
 #include <linux/init.h>
 #include <linux/stat.h>
  
  MODULE_LICENSE("GPL");
  MODULE_AUTHOR("Patrick Gomes");
  
  static short int myshort = 1;
  static int myint = 420;
  static long int mylong = 9999;
  static char* mystring = "blah";
  static int myintarray[2] = {-1, -1};
  static int arr_argc = 0;
  
  /*
   * module_param(foo, int, 0000)
   * foo - param's name
   * int - data type
   * 000 - permissions bit
   */

   module_param(myshort, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
   MODULE_PARM_DESC(myshort, "A short integer");
   module_param(myint, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
   MODULE_PARM_DESC(myint, "An integer");
   module_param(mylong, long, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
   MODULE_PARM_DESC(mylong, "A long integer");
   module_param(mystring, charp, 0000);
   MODULE_PARM_DESC(mystring, "a character string");
   
   /*
    * module_param_array(name, type, num, perm);
    * name - (array) param's name
    * type - data type of the array
    * num - pointer to variable that stores number of 
    * elements of array initialized
    * the permission bits
    */
    
    module_param_array(myintarray, int, &arr_argc, 0000);
    MODULE_PARM_DESC(myintarray, "An array of integers.");
    
    static int __init hello_5_init(void) {
    	int i;
	printk (KERN_INFO "Hello world 5!\n");
	printk (KERN_INFO "myshort is a short integer: %hd\n", myshort );
	printk (KERN_INFO "myint is an integer: %d\n", myint);
	printk (KERN_INFO "mylong is a long: %ld\n", mylong);
	printk (KERN_INFO "mystring is a string: %s\n", mystring);
	for (i = 0; i < (sizeof(myintarray) / sizeof(int)); i++) {
		printk (KERN_INFO "myintarray[%d] = %d\n", i, myintarray[i]);
	}
	printk (KERN_INFO "got %d arguments for myintarray.\n", arr_argc);
	return 0;
    }
    
    static void __exit hello_5_exit(void) {
    	printk (KERN_INFO "Goodbye world 5!\n");
    }
    
 module_init(hello_5_init);
 module_exit(hello_5_exit);
