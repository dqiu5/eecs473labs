#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/uaccess.h>	/* copy_from/to_user */

MODULE_LICENSE ("Dual BSD/GPL");

int memory_open (struct inode *inode, struct file *filp);
int memory_release (struct inode *inode, struct file *filp);
ssize_t memory_read (struct file *filp, char *buf, size_t count,
		     loff_t * f_pos);
ssize_t memory_write (struct file *filp, const char *buf, size_t count,
		      loff_t * f_pos);
void memory_exit (void);
int memory_init (void);

struct file_operations memory_fops = 
{
  .read = memory_read,
  .write = memory_write,
  .open = memory_open,
  .release = memory_release
};

module_init (memory_init);
module_exit (memory_exit);

int memory_major = 61;
char *memory_buffer;

int
memory_init (void)
{
  int result;
  result = register_chrdev (memory_major, "memory", &memory_fops);
  if (result < 0)
    {
      printk ("<1>memory: cannot obtain major number %d\n", memory_major);
      return result;
    }

  /* Allocating memory for the buffer */
  memory_buffer = kmalloc (1, GFP_KERNEL);
  if (!memory_buffer)
    {
      result = -ENOMEM;
      goto fail;
    }

  memset (memory_buffer, 0, 1);
  printk ("<1> Inserting memory module\n");
  return 0;

fail:
  memory_exit ();
  return result;
}

void
memory_exit (void)
{
  unregister_chrdev (memory_major, "memory");
  if (memory_buffer)
    {
      kfree (memory_buffer);
    }
  printk ("<1>Removing memory module\n");
}

int
memory_open (struct inode *inode, struct file *filp)
{
  return 0;
}

int
memory_release (struct inode *inode, struct file *filp)
{
  return 0;
}

ssize_t
memory_read (struct file * filp, char *buf, size_t count, loff_t * f_pos)
{
  int rv;
  /* Transfering data to user space */
  /* Changing reading position as best suits */
  printk("Read started.");
  if (*f_pos == 0)
    {
      rv=copy_to_user (buf, memory_buffer, 1);
      if(rv)
      {
        printk("copy to user failed");
        return(0);
      }
      *f_pos += 1;
      printk("%d", count);
      return 1;
    }
  else
    {
      return 0;
    }
  printk("Read finished.");
}

ssize_t
memory_write (struct file * filp, const char *buf, size_t count, loff_t * f_pos)
{
  int rv;
//   const char *tmp;
//   tmp = buf + count - 1;
  rv=copy_from_user (memory_buffer, buf + count - 1, 1);
    if(rv)
    {
       printk("copy from user failed");
       return(0);
    }
  *f_pos += 1;
  printk("Write finished.");
  return count;
}
