#ifndef _DISKFUNCS_H
#define _DISKFUNCS_H

int check_for_partition(int image_fd);
int check_for_superblock(int image_fd);
int check_files(int image_fd);

#endif /* _DISKFUNCS_H */
