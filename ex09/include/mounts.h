#include <linux/stat.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/dcache.h>
#include <linux/stat.h>

int mymounts_show(struct seq_file *m, void *v);
int mymounts_open(struct inode *inode, struct file *file);
