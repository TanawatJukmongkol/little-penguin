#include "../../include/debugfs.h"
#include "linux/slab.h"
#include "linux/mutex.h"

static const char EXPECTED_STRING[] = "tjukmong\n";
static const size_t EXPECTED_LEN = sizeof(EXPECTED_STRING) - 1;

static DEFINE_MUTEX(id_mutex);
static int id_open;

t_debug *debug_fs_id;

int debug_id_open (struct inode *inode, struct file *filp);
int debug_id_release (struct inode *inode, struct file *filp);
ssize_t debug_id_read (struct file *filp, char __user * buf, size_t size, loff_t * f_pos);
ssize_t debug_id_write (struct file *filp, const char __user * buf, size_t size, loff_t * f_pos);

int debug_id_init(t_debug *dbg)
{
    dbg->fops.open = debug_id_open;
    dbg->fops.release = debug_id_release;
    dbg->fops.read = debug_id_read;
    dbg->fops.write = debug_id_write;
    debug_fs_id = dbg;
    return 0;
}

int debug_id_open (struct inode *inode, struct file *filp)
{
    int ret = 0;

    mutex_lock(&id_mutex);

    if (id_open) {
        ret = -EBUSY;
        goto out_unlock;
    }

    id_open++;

out_unlock:
    mutex_unlock(&id_mutex);

    printk(KERN_INFO
           "debugfs: file '%s' opened (open count = %d)\n",
           debug_fs_id->name, id_open);

    return ret;
}

int debug_id_release (struct inode *inode, struct file *filp)
{
    int ret = 0;

    mutex_lock(&id_mutex);

    if (!id_open) {
        ret = -EINVAL;
        goto out_unlock;
    }

    id_open--;

out_unlock:
    mutex_unlock(&id_mutex);

    printk(KERN_INFO
           "debugfs: file '%s' closed (open count = %d)\n",
           debug_fs_id->name, id_open);

    return ret;
}

ssize_t debug_id_read (struct file *filp, char __user * buf, size_t size, loff_t * f_pos)
{
    const char *data_to_send = EXPECTED_STRING;
    size_t data_len = EXPECTED_LEN;
    size_t bytes_to_copy;
    unsigned long uncopied;

    if (*f_pos == data_len)
        return 0;

    bytes_to_copy = min(size, data_len);

    printk(KERN_INFO
           "debugfs: read() called on '%s'. User requested %zu bytes.\n",
           debug_fs_id->name, size);

    uncopied = copy_to_user(buf, data_to_send, bytes_to_copy);

    if (uncopied) {
        printk(KERN_ERR
               "debugfs: Failed to copy %lu bytes to user space.\n",
               uncopied);
        return -EFAULT;
    }

    printk(KERN_INFO "debugfs: Successfully copied %zu bytes to user.\n",
           bytes_to_copy);

    *f_pos += bytes_to_copy;

    return bytes_to_copy;
}

ssize_t debug_id_write (struct file *filp, const char __user * buf, size_t size, loff_t * f_pos)
{
    char *kbuf;
    ssize_t ret = -EINVAL;

    printk(KERN_INFO
           "debugfs: write() called. User provided %zu bytes.\n",
           size);

    if (size != EXPECTED_LEN) {
        printk(KERN_WARNING
               "debugfs: Write failed. Expected length %zu, got %zu.\n",
               EXPECTED_LEN, size);
        return -EINVAL;
    }

    kbuf = kmalloc(size + 1, GFP_KERNEL);
    if (!kbuf) {
        printk(KERN_ERR
               "debugfs: Failed to allocate kernel buffer.\n");
        return -ENOMEM;
    }

    if (copy_from_user(kbuf, buf, size)) {
        printk(KERN_ERR
               "debugfs: Failed to copy data from user space.\n");
        ret = -EFAULT;
        goto out;
    }
    kbuf[size] = '\0';

    if (strncmp(kbuf, EXPECTED_STRING, size) == 0) {
        printk(KERN_INFO
               "debugfs: SUCCESS! Received expected value.\n");
        ret = size;
    } else {
        printk(KERN_WARNING
               "debugfs: Invalid value received: '%s'. Returning -EINVAL.\n",
               kbuf);
        ret = -EINVAL;
    }

out:
    kfree(kbuf);

    return ret;
}
