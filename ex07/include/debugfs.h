/* SPDX-License-Identifier: GPL-2.0 */
#include <linux/stat.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/dcache.h>
#include <linux/stat.h>

enum e_dbg_type {
	DBG_END,
	DBG_DIR,
	DBG_FILE
};

struct s_debug {
	// shared
	const char *name;
	struct dentry *root;
	struct s_debug *parent;
	int perm;
	enum e_dbg_type type;

	// directory
	struct s_debug *entry;

	// files
	const struct file_operations *fops;
	int (*init_file)(struct s_debug *node);
	int (*destruct)(struct s_debug *node);
};

int init_debugfs(struct s_debug *dbg);
int dest_debugfs(struct s_debug *dbg);

int debug_foo_init(struct s_debug *dbg);
int debug_id_init(struct s_debug *dbg);
int debug_jiffies_init(struct s_debug *dbg);
