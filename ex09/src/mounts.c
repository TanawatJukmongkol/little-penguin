// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/nsproxy.h>
#include <linux/fs_struct.h>
#include <linux/mnt_namespace.h>
#include <linux/security.h>
#include <linux/string.h>
#include <mount.h>
#include "../include/mounts.h"

#if MYMOUNTS_FULL

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

static int mymounts_show(struct seq_file *m, struct vfsmount *mnt)
{
	struct proc_mounts *p = m->private;
	struct mount *r = real_mount(mnt);
	struct path mnt_path = { .dentry = mnt->mnt_root, .mnt = mnt };
	struct super_block *sb = mnt_path.dentry->d_sb;
	int err = 0;

	if (sb->s_op->show_devname)
		err = sb->s_op->show_devname(m, mnt_path.dentry);
	else
		seq_escape(m, r->mnt_devname ? r->mnt_devname : "none", " \t\n\\#");
	if (err)
		return err;
	seq_putc(m, ' ');

	err = seq_path_root(m, &mnt_path, &p->root, " \t\n\\");
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

#else

static int mymounts_show(struct seq_file *m, struct vfsmount *mnt)
{
	struct proc_mounts *p = m->private;
	struct path mnt_path = { .dentry = mnt->mnt_root, .mnt = mnt };
	const char *name;
	char *buf, *path;
	int err = 0;

	buf = __getname();
	if (!buf)
		return -ENOMEM;

	path = __d_path(&mnt_path, &p->root, buf, PATH_MAX);
	if (!path) {
		err = SEQ_SKIP;
		goto out;
	}
	if (IS_ERR(path)) {
		err = PTR_ERR(path);
		goto out;
	}

	name = strrchr(path, '/') + 1;
	if (!*name)
		name = "root";
	seq_printf(m, "%-10s %s\n", name, path);
out:
	__putname(buf);
	return err;
}

#endif

int mymounts_open(struct inode *inode, struct file *file)
{
	struct mnt_namespace *ns;
	struct proc_mounts *p;
	struct path root;
	int ret;

	task_lock(current);
	if (!current->nsproxy || !current->nsproxy->mnt_ns || !current->fs) {
		task_unlock(current);
		return -ENOENT;
	}
	ns = current->nsproxy->mnt_ns;
	get_mnt_ns(ns);
	get_fs_root(current->fs, &root);
	task_unlock(current);

	ret = seq_open_private(file, &mounts_op, sizeof(struct proc_mounts));
	if (ret) {
		path_put(&root);
		put_mnt_ns(ns);
		return ret;
	}

	p = ((struct seq_file *)file->private_data)->private;
	p->ns = ns;
	p->root = root;
	p->show = mymounts_show;
	return 0;
}

int mymounts_release(struct inode *inode, struct file *file)
{
	struct seq_file *m = file->private_data;
	struct proc_mounts *p = m->private;

	path_put(&p->root);
	put_mnt_ns(p->ns);
	return seq_release_private(inode, file);
}
