#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
MODULE_LICENSE("Dual BSD/GPL");
static int hello_init(void) {
   printk("<1> Pork chop sandwiches!\n");
   return 0;
}
static void hello_exit(void) {
   printk("<1> What are you doing? Get out of here!\n");
}
module_init(hello_init);
module_exit(hello_exit);
