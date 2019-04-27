#if 0
exit `$1 $2 | ./test/FileCheck $0`
#endif

#include "vfs.h"
#include "hal.h"
#include "stdio.h"
#include "assert.h"
#include "string.h"
#include "kmalloc.h"
#include "errno.h"

/*
  /
  +-+ a
  | +-- b
  | +-+ c
  | | +-- d
  | +-- e
  +-+ f
    +-- g
    +-- h  -> "g"
    +-- i  -> "/a/b"
    +-- j  -> "h"
*/

enum {
  n_root, n_a, n_b, n_c, n_d, n_e, n_f, n_g, n_h, n_i, n_j, n_END
 } node_num;

typedef struct dummyfs {
  inode_t nodes[n_END];
  inode_t *root_inode;
} dummyfs_t;

char *datas[n_END] = {
  "",
  "",
  "my name is b, and b is my name\n",
  "",
  "I am d, son of c, first of my name\n",
  "Fool of a Took!\n",
  "",
  "You shall not pass!\n",
  "g",
  "/a/b",
  "h"
};

static int num_for_inode(dummyfs_t *dfs, inode_t *inode) {
  int n = -1;
  if (inode == dfs->root_inode) n = n_root;
  else if (inode == &dfs->nodes[n_a]) n = n_a;
  else if (inode == &dfs->nodes[n_b]) n = n_b;
  else if (inode == &dfs->nodes[n_c]) n = n_c;
  else if (inode == &dfs->nodes[n_d]) n = n_d;
  else if (inode == &dfs->nodes[n_e]) n = n_e;
  else if (inode == &dfs->nodes[n_f]) n = n_f;
  else if (inode == &dfs->nodes[n_g]) n = n_g;
  else if (inode == &dfs->nodes[n_h]) n = n_h;
  else if (inode == &dfs->nodes[n_i]) n = n_i;
  else if (inode == &dfs->nodes[n_j]) n = n_j;
  
  return n;
}

vector_t dreaddir(filesystem_t *fs, inode_t *dir) {
  dummyfs_t *dfs = fs->data;
  vector_t v = vector_new(sizeof(dirent_t), 4);
  dirent_t de;

  vector_t* inode_children = (vector_t*)dir->data;
  return *inode_children;
 
}

int64_t dread(filesystem_t *fs, inode_t *inode, uint64_t offset, void *buf, uint64_t sz) {
	memcpy(buf,inode->data+offset,sz);
	if(inode->type == it_chardev) {
		return read_console(buf, (int)sz);
	}
	return sz;
}

int64_t dwrite(filesystem_t *fs, inode_t *inode, uint64_t offset, void *buf, uint64_t sz) {
  dummyfs_t *dfs = fs->data;
  if(inode->type == it_chardev) {
        write_console(buf, (int)sz);
	return sz;
  }
 

  int new_size = inode->size-offset + sz;
  if(new_size > inode->size) {
	 void* new_data=kmalloc(new_size);
	 __builtin_memcpy(new_data, inode->data, inode->size);
	 inode->data=new_data;
  }
  memcpy(inode->data + offset, buf, sz);
  inode->size = new_size;
  return sz;
}

int dget_root(filesystem_t *fs, inode_t *inode) {
  dummyfs_t *dfs = fs->data;
  inode->data = kmalloc(sizeof(vector_t));
  vector_t* v = (vector_t*)inode->data;
  
  dirent_t de;
  v->itemsz = sizeof(dirent_t);
  v->nitems = 0;
  v->data   = 0;
  v->sz     = 0;



  inode->data=v;

  return 0;
}

int dmknod(filesystem_t *fs, inode_t* dir_ino, inode_t* dest_ino, const char *name) {
    vector_t* v = dir_ino->data;
    dirent_t de;
    de.name = name;
    de.ino = dest_ino;
    vector_add(v,&de);
    dest_ino->data = kmalloc(sizeof(vector_t));
    vector_t* new_v = dest_ino->data;
    new_v->itemsz = sizeof(dirent_t);
    new_v->nitems = 0;
    new_v->data   = 0;
    new_v->sz     = 0;
    return 0;
}

filesystem_t dummyfs = {
  .read = &dread,
  .write = &dwrite,
  .readdir = &dreaddir,
  .mknod = &dmknod,
  .get_root = &dget_root,
  .destroy = NULL
};

int dprobe(dev_t dev, filesystem_t *fs) {
  memcpy(fs, &dummyfs, sizeof(filesystem_t));

 
  return 0;
}

void emit_indent(int indent) {
  for (int i = 0; i < indent; ++i)
    kprintf(" ");
}

void emit_tree(const char *name, inode_t *ino, int indent, vector_t *done) {
  for (unsigned i = 0; i < vector_length(done); ++i)
    if (ino == *(inode_t**)vector_get(done, i))
      return;
  
  vector_add(done, &ino);

  emit_indent(indent);
  if(ino->type == it_chardev || ino->type == it_blockdev) {
	  kprintf("'%s' DEV %d %d\n", name, major(ino->u.dev),minor(ino->u.dev));
  } else {	  
	  kprintf("'%s' %s (size %d)\n", name,
        	  ((ino->type == it_dir) ? "DIR" : ""),
	          ino->size);
  }
  if (ino->type == it_dir) {
    vector_t files = vfs_readdir(ino);
    for (unsigned i = 0; i < vector_length(&files); ++i) {
      dirent_t *dent = vector_get(&files, i);
      emit_tree(dent->name, dent->ino, indent + 2, done);
    }
  }
}

bool daccess(int mode) {
  return true;
}

int init_mock_initrd() {
  // Check mounting
  // ----------------------------------------------------------------------

  // CHECK: register_filesystem = 0
  kprintf("register_filesystem = %d\n",
          register_filesystem("dummyfs", &dprobe));

  // CHECK: mount = 0
  kprintf("mount = %d\n",
          vfs_mount(makedev(DEV_MAJ_NULL, 0), vfs_get_root(), "dummyfs"));


  return 0;
}

static prereq_t r[] = { {"vfs",NULL}, {NULL,NULL} };
static module_t x run_on_startup = {
  .name = "vfs-test",
  .required = r,
  .load_after = NULL,
  .init = &init_mock_initrd,
  .fini = NULL
};
