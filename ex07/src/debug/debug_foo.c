#include "../../include/debugfs.h"
#include "linux/slab.h"
#include "linux/mutex.h"

ssize_t debug_foo_read (struct file *filp, char __user * buf, size_t size, loff_t * f_pos);
ssize_t debug_foo_write (struct file *filp, const char __user * buf, size_t size, loff_t * f_pos);

static  char        *buffer;
static  size_t      buffer_len;
static  DEFINE_MUTEX(foo_lock);
t_debug             *debug_fs_foo;

int debug_foo_init(t_debug *dbg)
{
    dbg->fops.read = debug_foo_read;
    dbg->fops.write = debug_foo_write;
    debug_fs_foo = dbg;
    if (!(buffer = kzalloc(PAGE_SIZE, GFP_KERNEL)))
        return -ENOMEM;
    return 0;
}

ssize_t debug_foo_read (struct file *filp, char __user * buf, size_t size, loff_t * f_pos)
{
    ssize_t ret;

    mutex_lock(&foo_lock);

    /* End of file */
    if (*f_pos >= buffer_len) {
        ret = 0;
        goto out_unlock;
    }

    /* Trim size to remaining data */
    if (size > buffer_len - *f_pos)
        size = buffer_len - *f_pos;

    if (copy_to_user(buf, buffer + *f_pos, size)) {
        ret = -EFAULT;
        goto out_unlock;
    }

    *f_pos += size;
    ret = size;

out_unlock:
    mutex_unlock(&foo_lock);

    return ret;
}

ssize_t debug_foo_write (struct file *filp, const char __user * buf, size_t size, loff_t * f_pos)
{
    ssize_t ret;

    if (size > PAGE_SIZE - 1)
        size = PAGE_SIZE - 1;

    mutex_lock(&foo_lock);

    if (copy_from_user(buffer, buf, size)) {
        ret = -EFAULT;
        goto out_unlock;
    }

    buffer[size] = '\0';  // Ensure null-terminated string
    buffer_len = size;
    ret = size;

out_unlock:
    mutex_unlock(&foo_lock);

    return ret;
}
