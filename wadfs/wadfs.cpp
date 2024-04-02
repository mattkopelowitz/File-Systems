#include "../libWad/Wad.h"
#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

Wad* wad;

static int getattr_callback(const char* path, struct stat* buf) {    
    memset(buf, 0, sizeof(struct stat));

    if (wad->isDirectory(path)) { 
        buf->st_mode = S_IFDIR | 0755;
        buf->st_nlink = 2;
        return 0;
    }

    if (wad->isContent(path)) {
        buf->st_mode = S_IFREG | 0777;
        buf->st_nlink = 1;
        buf->st_size = wad->getSize(path);
        return 0;
    }
    return -ENOENT;
}

static int open_callback(const char* path, struct fuse_file_info* fi) {
    return 0;
}

static int read_callback(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    int contents = wad->getContents(path, buf, size, offset);
    if (contents != -1) {
        return contents;
    }
    return 0;
}

static int release_callback(const char* path, struct fuse_file_info* fi) {
    return 0;
}

static int opendir_callback(const char* path, struct fuse_file_info* fi) {
    return 0;
}

static int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
    (void) offset;
    (void) fi;
    vector<string> components;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    
    wad->getDirectory(path, &components);

    for (string s : components) {
        filler(buf, s.c_str(), NULL, 0);
    }
    return 0;
}

static int releasedir_callback(const char* path, struct fuse_file_info* fi) {
    return 0;
}

static struct fuse_operations fuse_example_operations = {
  .getattr = getattr_callback,
  .open = open_callback,
  .read = read_callback,
  .release = release_callback,
  .opendir = opendir_callback,
  .readdir = readdir_callback,
  .releasedir = releasedir_callback,
};

int main(int argc, char* argv[]) {
  wad = Wad::loadWad(argv[1]);

  argv[1] = argv[2];
  argv[2] = NULL;
  argc--;

  return fuse_main(argc, argv, &fuse_example_operations, NULL);
}