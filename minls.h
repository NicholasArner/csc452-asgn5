#ifndef MINLS_H
#define MINLS_H
#include "minhelper.h"
void printCurFile(directory dir, inode i_info);
void printDirContents(uint16_t zoneSize, superblock sb, FILE *img);
void printPermissions(uint16_t mode);
#endif
