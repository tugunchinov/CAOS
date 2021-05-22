#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

extern "C" {
#define FUSE_USE_VERSION 30
#include <fuse.h>
};

struct info_t {
  int count;
  std::vector<std::filesystem::path> paths;
};
info_t info;

int stat_callback(const char* p,
                  struct stat* st,
                  struct fuse_file_info* /*fi*/) {
  std::filesystem::path path(p);
  if (path == "/") {
    st->st_mode = 0555 | S_IFDIR;
    st->st_nlink = 2 + info.count;
    return 0;
  }
  bool reg = false;
  bool exists = false;
  st->st_nlink = 2;
  st->st_mode = 0555 | S_IFDIR;
  std::filesystem::path matched_file;
  for (const auto& m_path : info.paths) {
    std::filesystem::path cur = m_path;
    cur += path;
    if (std::filesystem::exists(cur)) {
      exists = true;
      if (std::filesystem::is_directory(cur)) {
        ++st->st_nlink;
      } else if (std::filesystem::is_regular_file(cur) &&
                 (!reg ||
                 std::filesystem::last_write_time(cur) >
                 std::filesystem::last_write_time(matched_file))) {
        reg = true;
        matched_file = cur;
      }
    }
  }

  if (!exists) {
    return -ENOENT;
  }

  if (reg) {
    st->st_mode = S_IFREG | 0444;
    st->st_nlink = 1;
    st->st_size = std::filesystem::file_size(matched_file);
  }

  return 0;
}

int readdir_callback(const char* path,
                    void* out,
                    fuse_fill_dir_t filler,
                    off_t /*off*/,
                    struct fuse_file_info* /*fir*/,
                    enum fuse_readdir_flags /*flags*/) {
  filler(out, ".", NULL, 0, (fuse_fill_dir_flags)0);
  filler(out, "..", NULL, 0, (fuse_fill_dir_flags)0);

  bool exists = false;

  std::unordered_map<std::string, std::filesystem::file_time_type> files;

  for (const auto& m_path : info.paths) {
    std::filesystem::path cur = m_path;
    cur += path;
    if (std::filesystem::exists(cur)) {
      exists = true;
      if (!std::filesystem::is_directory(cur)) {
        return -ENOTDIR;
      }
      for (const auto& file : std::filesystem::directory_iterator(cur)) {
        if (std::filesystem::is_directory(file)) {
          if (files.find(file.path().filename().c_str()) == files.end()) {
            files[file.path().filename().c_str()] = file.last_write_time();
          }
        } else if (files.find(file.path().filename().c_str()) == files.end() ||
          file.last_write_time() > files[file.path().filename().c_str()]) {
          files[file.path().filename().c_str()] = file.last_write_time();
        }
      }
    }
  }

  for (const auto& file : files) {
    filler(out, file.first.c_str(), NULL, 0, (fuse_fill_dir_flags)0);
  }

  return exists ? 0 : -ENOENT;
}

int open_callback(const char* p, struct fuse_file_info* /*fi*/) {
  std::filesystem::path path(p);
  for (const auto& m_path : info.paths) {
    std::filesystem::path cur = m_path;
    cur += path;
    if (std::filesystem::exists(cur)) {
      return 0;
    }
  }
  return -ENOENT;
}

int read_callback(const char* path,
                  char* out,
                  size_t size,
                  off_t offset,
                  struct fuse_file_info* /*fi*/) {
  std::filesystem::path matched_file;
  bool exists = false;
  for (const auto& m_path : info.paths) {
    std::filesystem::path cur = m_path;
    cur += path;
    if (std::filesystem::exists(cur)) {
      if (std::filesystem::is_regular_file(cur) &&
          (!exists ||
           std::filesystem::last_write_time(cur) > std::filesystem::last_write_time(matched_file))) {
        exists = true;
        matched_file = cur;
      }
    }
  }
  if (!exists) {
    return -ENOENT;
  }

  if (offset > (off_t)std::filesystem::file_size(matched_file)) {
    return 0;
  }

  if (offset + size > std::filesystem::file_size(matched_file)) {
    size = std::filesystem::file_size(matched_file) - offset;
  }

  std::ifstream input(matched_file, std::ios::binary);
  input.seekg(offset);
  input.read(out, size);
  input.close();

  return size;
}

static struct fuse_operations operations = {
    .getattr = stat_callback,
    .open = open_callback,
    .read = read_callback,
    .readdir = readdir_callback
};

struct options_t {
  char* image_src;
};
options_t options;

struct fuse_opt opt_specs[] = {
  {"--src %s", offsetof(options_t, image_src), 0},
  FUSE_OPT_END
};

void init_image() {
  char* dir = strtok(options.image_src, ":");
  while (dir) {
    info.paths.push_back(std::filesystem::canonical(dir));
    ++info.count;
    dir = strtok(NULL, ":");
  }
}

int main(int argc, char** argv) {
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

  fuse_opt_parse(&args, &options, opt_specs, NULL);
  init_image();

  int ret = fuse_main(args.argc, args.argv, &operations, NULL);

  fuse_opt_free_args(&args);

  return ret;
}
