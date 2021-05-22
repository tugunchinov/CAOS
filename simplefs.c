#define _GNU_SOURCE
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <unistd.h>

typedef struct {
  int count;
  char paths[BUFSIZ][BUFSIZ];
  int size[BUFSIZ];
  int total_size;
  int data_start;
  FILE* img_src;
} info_t;
info_t info;


typedef struct {
  char* image_src;
} options_t;
options_t options;

int
stat_callback(const char* path, struct stat* st, struct fuse_file_info* fi) {
  if (strcmp("/", path) == 0) {
    st->st_mode = 0555 | S_IFDIR;
    st->st_nlink = 2;
    return 0;
  }
  for (int i = 0; i < info.count; ++i) {
    if (strcmp(basename(path), info.paths[i]) == 0) {
      st->st_mode = S_IFREG | 0444;
      st->st_nlink = 1;
      st->st_size = info.size[i];
      return 0;
    }
  }
  return -ENOENT;
}

int
readdir_callback(const char* path,
                 void* out,
                 fuse_fill_dir_t filler,
                 off_t off,
                 struct fuse_file_info* fir,
                 enum fuse_readdir_flags flags) {
  if (strcmp(path, "/") != 0) {
    return -ENOENT;
  }

  filler(out, ".", NULL, 0, 0);
  filler(out, "..", NULL, 0, 0);

  for (int i = 0; i < info.count; ++i) {
    filler(out, info.paths[i], NULL, 0, 0);
  }
  
  return 0;
}

int
open_callback(const char* path, struct fuse_file_info* fi) {
  for (int i = 0; i < info.count; ++i) {
    if (strcmp(basename(path), info.paths[i]) == 0) {
      return 0;
    }
  }
  return -ENOENT;
}

int
read_callback(const char* path,
              char* out,
              size_t size,
              off_t offset,
              struct fuse_file_info* fi) {
  char* filename = basename(path);
  int skip_bytes = 0;
  for (int i = 0; i < info.count; ++i) {
    if (strcmp(info.paths[i], filename) == 0) {
      if (offset > info.size[i]) {
        return 0;
      }
      if (offset + size > info.size[i]) {
        size = info.size[i] - offset;
      }
      fseek(info.img_src, info.data_start + skip_bytes, SEEK_SET);
      fread(out, sizeof(char), size, info.img_src);
      return size;
    }
    skip_bytes += info.size[i];
  }
  return -ENOENT;
}

static struct fuse_operations operations = {.read = read_callback,
                                            .open = open_callback,
                                            .getattr = stat_callback,
                                            .readdir = readdir_callback,};

struct fuse_opt opt_specs[] = {
  {"--src %s", offsetof(options_t, image_src), 0},
  FUSE_OPT_END
};

void
init_image() {
  char image_path[BUFSIZ];
  memset(image_path, 0, sizeof(image_path));
  realpath(options.image_src, image_path);
  FILE* image = fopen(options.image_src, "r");
  fscanf(image, "%d", &info.count);
  for (int i = 0; i < info.count; ++i) {
    fscanf(image, "%s", info.paths[i]);
    fscanf(image, "%d", &info.size[i]);
    info.total_size += info.size[i];
  }
  info.data_start = ftell(image) + 2;
  info.img_src = image;
}

int
main(int argc, char** argv) {
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  
  fuse_opt_parse(&args, &options, opt_specs, NULL);
  init_image();

  int ret = fuse_main(args.argc, args.argv, &operations, NULL);

  fuse_opt_free_args(&args);

  fclose(info.img_src);
  
  return ret;
}
