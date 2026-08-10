# nls

"Neo ls" is a small "ls" style cli tool, built as a hobby project.

## What it does
- `-a`  show hidden files
- `-l`  long format (permissions, link count, size, modified time)
- `-r`  reverse order
- `-t`  sort by modification time (newest first)
- `-S`  sort by size (largest first)
- `-R`  recurse into subdirectories
- `-d`  list a directory itself, not its contents

## Notes / known limitations

- Permission display is POSIX-accurate on Linux/macOS. On Windows,
  `std::filesystem::perms` can't map NTFS ACLs to POSIX rwx bits, so
  permissions show as `rwxrwxrwx` for most files — a limitation of
  the standard library, not this tool.
- Owner/group columns aren't implemented, since `std::filesystem`
  has no portable concept of them (would require POSIX-only APIs).

  ## Building

Requires a C++23 compiler (uses `std::print`) and CMake version 3.28.


​```
cmake -B build
cmake --build build
​```