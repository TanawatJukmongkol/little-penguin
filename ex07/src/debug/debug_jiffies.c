#include "../../include/debugfs.h"
#include "linux/jiffies.h"

ssize_t debug_jiffies_read (struct file *filp, char __user * buf, size_t size, loff_t * f_pos);

t_debug *debug_fs_jiffies;

int debug_jiffies_init(t_debug *dbg)
{
    dbg->fops.read = debug_jiffies_read;
    debug_fs_jiffies = dbg;
    return 0;
}

ssize_t debug_jiffies_read (struct file *filp, char __user * buf, size_t size, loff_t * f_pos)
{
    char    tmp[32];
    int     len;

    len = snprintf(tmp, sizeof(tmp), "%lu\n", jiffies);

    return simple_read_from_buffer(buf, size, f_pos, tmp, len);
}
