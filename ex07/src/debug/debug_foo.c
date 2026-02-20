#include "../include/debugfs.h"
#include "linux/slab.h"

ssize_t debug_foo_read (struct file *filp, char __user * buf, size_t size, loff_t * f_pos);
ssize_t debug_foo_write (struct file *filp, const char __user * buf, size_t size, loff_t * f_pos);

static  char    *buffer;
t_debug         *debug_fs;

int debug_foo_init(t_debug *dbg)
{
    dbg->fops.read = debug_foo_read;
    dbg->fops.write = debug_foo_write;
    debug_fs = dbg;
    if (!(buffer = kzalloc(PAGE_SIZE, GFP_KERNEL)))
        return -ENOMEM;
    return 0;
}

ssize_t debug_foo_read (struct file *filp, char __user * buf, size_t size, loff_t * f_pos)
{
    size_t len;

    if (!buffer)
        return -ENOMEM;

    len = strnlen(buffer, PAGE_SIZE);

    /* End of file */
    if (*f_pos >= len)
        return 0;

    /* Trim size to remaining data */
    if (size > len - *f_pos)
        size = len - *f_pos;

    if (copy_to_user(buf, buffer + *f_pos, size))
        return -EFAULT;

    *f_pos += size;

    return size;
}

ssize_t debug_foo_write (struct file *filp, const char __user * buf, size_t size, loff_t * f_pos)
{
    if (!buffer)
    return -ENOMEM;

    if (size > PAGE_SIZE - 1)
        size = PAGE_SIZE - 1;

    if (copy_from_user(buffer, buf, size))
        return -EFAULT;

    buffer[size] = '\0';  // Ensure null-terminated string

    return size;
}

