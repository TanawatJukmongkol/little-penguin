// SPDX-License-Identifier: GPL-2.0
#include "../include/debugfs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tjukmong");
MODULE_DESCRIPTION("A simple debugfs driver.");

static t_debug debugfs = (t_debug) {
	.name = "fortytwo", .type = DBG_DIR,
	.perm = 0666,
	.entry = (t_debug []) {
		{
			.name = "id", .type = DBG_FILE,
			.perm = 0666,
			.init_file = debug_id_init
		},
		{
			.name = "jiffies", .type = DBG_FILE,
			.perm = 0444,
			.init_file = debug_jiffies_init
		},
		{
			.name = "foo", .type = DBG_FILE,
			.perm = 0644,
			.init_file = debug_foo_init
		},
		{ }
	}
};

static int my_module_init(void)
{
	pr_info("debugfs: creating debug structure...\n");
	init_debugfs(&debugfs);
	return 0;
}

static void my_module_exit(void)
{
	pr_info("Cleaning up module.\n");
	dest_debugfs(&debugfs);
}

module_init(my_module_init);
module_exit(my_module_exit);
