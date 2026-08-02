// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/seq_file.h>
#include <linux/nsproxy.h>
#include <linux/ns_common.h>
#include <linux/fs_struct.h>
#include <linux/security.h>
#include <mount.h>
#include "../include/mounts.h"


struct mymounts_flag {
	int flag;
	const char *str;
};

static const struct mymounts_flag mymounts_mnt_flags[] = {
	{ MNT_NOSUID,      ",nosuid" },
	{ MNT_NODEV,       ",nodev" },
	{ MNT_NOEXEC,      ",noexec" },
	{ MNT_NOATIME,     ",noatime" },
	{ MNT_NODIRATIME,  ",nodiratime" },
	{ MNT_RELATIME,    ",relatime" },
	{ MNT_NOSYMFOLLOW, ",nosymfollow" },
	{}
};

static const struct mymounts_flag mymounts_sb_flags[] = {
	{ SB_SYNCHRONOUS, ",sync" },
	{ SB_DIRSYNC,     ",dirsync" },
	{ SB_MANDLOCK,    ",mand" },
	{ SB_LAZYTIME,    ",lazytime" },
	{}
};

static int mymounts_show_opts(struct seq_file *m, struct vfsmount *mnt, struct super_block *sb)
{
	const struct mymounts_flag *f;

	for (f = mymounts_mnt_flags; f->str; f++)
		if (mnt->mnt_flags & f->flag)
			seq_puts(m, f->str);
	for (f = mymounts_sb_flags; f->str; f++)
		if (sb->s_flags & f->flag)
			seq_puts(m, f->str);
	return security_sb_show_options(m, sb);
}

static int mymounts_show_one(struct seq_file *m, struct mount *r, struct path *root)
{
	struct vfsmount *mnt = &r->mnt;
	struct path mnt_path = { .dentry = mnt->mnt_root, .mnt = mnt };
	struct super_block *sb = mnt_path.dentry->d_sb;
	size_t mark = m->count;
	int err = 0;

	if (sb->s_op->show_devname)
		err = sb->s_op->show_devname(m, mnt_path.dentry);
	else
		seq_escape(m, r->mnt_devname ? r->mnt_devname : "none", " \t\n\\#");
	if (err)
		return err;
	seq_putc(m, ' ');

	err = seq_path_root(m, &mnt_path, root, " \t\n\\");
	if (err == SEQ_SKIP) {
		m->count = mark;
		return 0;
	}
	if (err)
		return err;

	seq_putc(m, ' ');
	seq_escape(m, sb->s_type->name, " \t\n\\#");
	seq_puts(m, __mnt_is_readonly(mnt) ? " ro" : " rw");
	err = mymounts_show_opts(m, mnt, sb);
	if (err)
		return err;
	if (sb->s_op->show_options)
		err = sb->s_op->show_options(m, mnt_path.dentry);
	seq_puts(m, " 0 0\n");
	return err;
}

int mymounts_show(struct seq_file *m, void *v)
{
	struct rb_node *node;
	struct path root;
	int err = 0;

	get_task_struct(current);
	task_lock(current);
	if (!current->nsproxy || !current->nsproxy->mnt_ns || !current->fs) {
		task_unlock(current);
		put_task_struct(current);
		return 0;
	}
	get_fs_root(current->fs, &root);

	for (node = rb_first(&current->nsproxy->mnt_ns->mounts);
	     node && !err; node = rb_next(node)) {
		struct mount *r = rb_entry(node, struct mount, mnt_node);

		err = mymounts_show_one(m, r, &root);
	}

	task_unlock(current);
	put_task_struct(current);
	path_put(&root);
	return err;
}

/* Open implementation linking seq_file */
int mymounts_open(struct inode *inode, struct file *file)
{
	return single_open(file, mymounts_show, NULL);
}
