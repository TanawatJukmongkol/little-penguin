#include "../include/debugfs.h"
#include "asm-generic/errno-base.h"
#include "linux/err.h"

int init_debugfs(t_debug *dbg)
{
    struct dentry *parent_root = dbg->parent ? dbg->parent->root : NULL;
    int error;

    if (!dbg) return -ENODEV;

    switch(dbg->type)
    {
        case DBG_DIR:
            dbg->root = debugfs_create_dir(dbg->name, parent_root);
            if (IS_ERR(dbg->root))
                return -ENODEV;
            printk("debugfs: entry \"%s\" {\n", dbg->name);
            for (int i = 0; dbg->entry[i].type != DBG_END; i++) {
                printk("\tdebugfs: entry #%d type %d \n", i, dbg->entry[i].type);
                dbg->entry[i].parent = dbg;
                if ((error = init_debugfs(&dbg->entry[i])) != 0)
                    return error;
            }
            printk("}\n");
        break ;
        case DBG_FILE:
            printk("debugfs: file name: \"%s\"\n", dbg->name);
            if ((error = dbg->init_file(dbg)) != 0)
                return error;
            dbg->root = debugfs_create_file(dbg->name, dbg->perm, dbg->parent->root, NULL, &dbg->fops);
            if (IS_ERR(dbg->root))
                return -ENODEV;
        break ;
        default:
            return -EINVAL;
    }
    return 0;
}
