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

#define MEMSIZE 5

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

int memory_major = 60;
char *memory_buffer;

int mem_offset = 0;

void increase_mem_offset(int val) {
  mem_offset = (mem_offset + val) % (MEMSIZE);
}

char* get_buffer(int offset) {
  return memory_buffer + ((mem_offset + offset) % (MEMSIZE));
}

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
  memory_buffer = kmalloc (MEMSIZE, GFP_KERNEL);
  if (!memory_buffer)
    {
      result = -ENOMEM;
      goto fail;
    }

  memset (memory_buffer, 0, MEMSIZE);
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
  printk("read started");
  if (*f_pos == 0)
    {
      if (count > MEMSIZE) {
        printk("count > MEMSIZE");
        int i = 0;
        for (; i < MEMSIZE; i ++) {
          rv = copy_to_user (buf + i, get_buffer(4 - i), 1);
          printk("%c\n", *(memory_buffer + i));
          if(rv)
          {
            printk("copy to user failed");
            return(0);
          }
          *f_pos += 1; 
        }
        return MEMSIZE;
      }
      else if (count == 0) {
        printk("count equal 0.");
        return 0;
      }
      else {
        int i = 0;
        for (; i < count; i ++) {
          rv = copy_to_user (buf + i, get_buffer(count - i - 1), 1);
          if(rv)
          {
            printk("copy to user failed");
            return(0);
          }
          *f_pos += 1; 
        }
        return count;
      }
      // printk("%d", count);
      // int i = 0;
      // for (; i < MEMSIZE; i ++) {
      //     printk("%c\n", *(buf + i));
      // }
    }
  else
    {
      return 0;
    }
}

ssize_t
memory_write (struct file * filp, const char *buf, size_t count, loff_t * f_pos)
{
  int rv;
  const char* tmp;



  // char* local_buffer = kmalloc(1, GFP_KERNEL);
  // int i = 0;
  // for (; i < MEMSIZE - count; i++) {
  //   rv = copy_to_user(local_buffer, memory_buffer + MEMSIZE - count - i - 1, 1);
  //   if(rv)
  //     {
  //       printk("copy to user failed");
  //       return(0);
  //     }
    
  //   rv = copy_from_user(memory_buffer + MEMSIZE - i - 1, local_buffer, 1);
  //   if(rv)
  //   {
  //      printk("copy from user failed");
  //      return(0);
  //   }
  // }

  if (count > MEMSIZE) {
    tmp = buf + count - MEMSIZE;
    int i = 0;
    for (; i < MEMSIZE; i ++) {
      rv = copy_from_user (get_buffer(i), tmp + i, 1);
      if(rv)
      {
        printk("copy from user failed");
        return(0);
      }
    }
    increase_mem_offset(MEMSIZE);
  }
  else {
    tmp = buf;
    int i = 0;
    for (; i < count; i ++) {
      rv = copy_from_user (get_buffer(i), tmp + i, 1);
      if(rv)
      {
        printk("copy from user failed");
        return(0);
      }
    }
    increase_mem_offset(count);
    //rv=copy_from_user (memory_buffer, tmp, MEMSIZE - count);
  }
  if (count > MEMSIZE) {
    *f_pos += MEMSIZE;
  }
  else {
    *f_pos += count;
  }
  printk("write finished");
  
  return count;
}
