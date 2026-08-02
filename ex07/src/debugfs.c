// SPDX-License-Identifier: GPL-2.0
#include <linux/err.h>
#include "../include/debugfs.h"

int init_debugfs(t_debug *dbg)
{
	struct dentry *parent_root = dbg->parent ? dbg->parent->root : NULL;
	int error;

	if (!dbg)
		return -ENODEV;

	switch (dbg->type) {
	case DBG_DIR:
		dbg->root = debugfs_create_dir(dbg->name, parent_root);
		if (IS_ERR(dbg->root))
			return -ENODEV;
		pr_info("debugfs: entry \"%s\" {\n", dbg->name);
		for (int i = 0; dbg->entry[i].type != DBG_END; i++) {
			pr_info("\tdebugfs: entry #%d type %d\n", i, dbg->entry[i].type);
			dbg->entry[i].parent = dbg;
			error = init_debugfs(&dbg->entry[i]);
			if (error != 0)
				return error;
		}
		pr_info("}\n");
		break;
	case DBG_FILE:
		pr_info("debugfs: file name: \"%s\"\n", dbg->name);
		if (!dbg->init_file)
			return 0;
		error = dbg->init_file(dbg);
		if (error != 0)
			return error;
		dbg->root = debugfs_create_file(dbg->name, dbg->perm,
						dbg->parent->root, NULL, &dbg->fops);
		if (IS_ERR(dbg->root))
			return -ENODEV;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int dest_debugfs(t_debug *dbg)
{
	int error;

	if (!dbg)
		return -ENODEV;

	switch (dbg->type) {
	case DBG_DIR:
		pr_info("debugfs: destroy entry \"%s\" {\n", dbg->name);
		for (int i = 0; dbg->entry[i].type != DBG_END; i++) {
			error = dest_debugfs(&dbg->entry[i]);
			if (error != 0)
				return error;
		}
		pr_info("}\n");
		break;
	case DBG_FILE:
		pr_info("debugfs: file name: \"%s\"\n", dbg->name);
		if (!dbg->destruct)
			return 0;
		error = dbg->destruct(dbg);
		if (error != 0)
			return error;
		break;
	default:
		return -EINVAL;
	}
	debugfs_remove(dbg->root);
	return 0;
}
