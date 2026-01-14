// SPDX-License-Identifier: GPL-2.0
#include "../include/debugfs.h"
#include "asm-generic/errno-base.h"
#include "linux/fs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tjukmong");

static t_debug	debugfs = (t_debug) {
    .name = "fortytwo", .type = DBG_DIR,
    .perm = S_IRUGO | S_IWUGO,
    .entry = (t_debug []) {
        {
            .name = "id", .type = DBG_FILE,
            .perm = S_IRUGO | S_IWUGO,
			.init_file = debug_id_init
        },
        {
            .name = "jiffies", .type = DBG_FILE,
            .perm = S_IRUGO
        },
        {
            .name = "foo", .type = DBG_FILE,
            .perm = S_IRUGO | S_IWUSR
        },
        { .type = DBG_END }
    }
};

int my_module_init(void);
void my_module_exit(void);

int my_module_init(void)
{
	pr_info("debugfs: creating debug structure...\n");
	init_debugfs(&debugfs);
	return 0;
}

void my_module_exit(void)
{
	pr_info("Cleaning up module.\n");
	debugfs_remove(debugfs.root);
}

module_init(my_module_init);
module_exit(my_module_exit);
