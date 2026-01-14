#include "../include/debugfs.h"
#include "linux/slab.h"

ssize_t debug_read (struct file *filp, char __user * buf, size_t size, loff_t * f_pos);
ssize_t debug_write (struct file *filp, const char __user * buf, size_t size, loff_t * f_pos);

static  char    *buffer;
t_debug         *debug_fs;

int debug_id_init(t_debug *dbg)
{
    dbg->fops.read = debug_read;
    dbg->fops.write = debug_write;
    debug_fs = dbg;
    if (!(buffer = kzalloc(PAGE_SIZE, GFP_KERNEL)))
        return -ENOMEM;
    return 0;
}

ssize_t debug_read (struct file *filp, char __user * buf, size_t size, loff_t * f_pos)
{
    return 0;
}

ssize_t debug_write (struct file *filp, const char __user * buf, size_t size, loff_t * f_pos)
{
    return 0;
}

