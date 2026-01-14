#include "linux/stat.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/dcache.h>
#include <linux/stat.h>

enum e_dbg_type
{
    DBG_DIR,
    DBG_FILE,
    DBG_END
};

typedef struct s_debug {
    // shared
    const char              *name;
	struct dentry           *root;
    struct s_debug          *parent;
    int                     perm;
    enum e_dbg_type         type;

    // directory
    struct s_debug          *entry;

    // files
    struct file_operations  fops;
    int                     (*init_file)(struct s_debug *node);
} t_debug;

int init_debugfs(t_debug *dbg);

int debug_id_init(t_debug *dbg);
