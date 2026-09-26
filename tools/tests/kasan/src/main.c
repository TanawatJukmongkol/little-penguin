// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/compiler.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tjukmong");
MODULE_DESCRIPTION("Deliberate memory bugs, for checking that KASAN reports them");

/*
 * Two reads KASAN must report: one byte past an 8-byte allocation, and one
 * from the allocation after it's freed. Reads, not writes, so nothing gets
 * corrupted when the kernel carries on after the reports.
 */
static int __init kasan_selftest_init(void)
{
	char *p;
	char oob, uaf;

	p = kmalloc(8, GFP_KERNEL);
	if (!p)
		return -ENOMEM;
	OPTIMIZER_HIDE_VAR(p);
	oob = READ_ONCE(p[8]);
	kfree(p);
	uaf = READ_ONCE(p[0]);
	pr_info("kasan_selftest: read %d and %d\n", oob, uaf);
	return 0;
}

static void __exit kasan_selftest_exit(void)
{
}

module_init(kasan_selftest_init);
module_exit(kasan_selftest_exit);
