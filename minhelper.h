#ifndef _MINHELPER_H
#define _MINHELPER_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MINIX_MAGIC        0x4D5A
#define INODE_SIZE_BYTES   64
#define DIR_SIZE_BYTES     64
#define NR_PARTITIONS      4
#define PART_TABLE_ADDR    0x1BE
#define SUPER_BLOCK_SIZE   1000
#define BOOT_BLOCK_SIZE    1000
#define NO_PART            0x0
#define MINIX_PART         0x81
#define INODE_SIZE         64
#define DIR_ENTRY_SIZE     64
#define OFFSET             1024
#define ROOT_DIR           1
#define INODE_BLOCK        4
#define NO_SUBPART         -1
#define NO_PRIPART         -1
#define DIRECT_ZONES       7
#define SECTOR_SIZE        512
#define VALID_TABLE_ADDR1  510
#define VALID_TABLE_ADDR2  511
#define VALID_TABLE_BYTE1  0x55
#define VALID_TABLE_BYTE2  0xAA

/*File Types*/
#define FILE_TYPE 0170000
#define IS_REG    0100000
#define IS_DIR    0040000
#define OWN_R     0000400
#define OWN_W     0000200
#define OWN_X     0000100
#define GRP_R     0000040
#define GRP_W     0000020
#define GRP_X     0000010
#define OTR_R     0000004
#define OTR_W     0000002
#define OTR_X     0000001 

/* TODO: should all structs be padded? */
typedef struct __attribute__ ((__packed__)) { /* Minix Version 3 Superblock
                     * this structure found in fs/super.h
                     * * in minix 3.1.1
                     * */
  /* on disk. These fields and orientation are non–negotiable */
  uint32_t ninodes; /* number of inodes in this filesystem */
  uint16_t pad1; /* make things line up properly */
  int16_t i_blocks; /* # of blocks used by inode bit map */
  int16_t z_blocks; /* # of blocks used by zone bit map */
  uint16_t firstdata; /* number of first data zone */
  int16_t log_zone_size; /* log2 of blocks per zone */
  int16_t pad2; /* make things line up again */
  uint32_t max_file; /* maximum file size */
  uint32_t zones; /* number of zones on disk */
  int16_t magic; /* magic number */
  int16_t pad3; /* make things line up again */
  uint16_t blocksize; /* block size in bytes */
  uint8_t subversion; /* filesystem sub–version */
} superblock;

typedef struct __attribute__ ((__packed__)) {
  uint16_t mode; /* mode */
  uint16_t links; /* number or links */
  uint16_t uid;
  uint16_t gid;
  uint32_t size;
  int32_t atime;
  int32_t mtime;
  int32_t ctime;
  uint32_t zone[DIRECT_ZONES];
  uint32_t indirect;
  uint32_t two_indirect;
  uint32_t unused;
} inode;

/* partition table entry */
/* TODO: will partition table always be the same size? */

typedef struct __attribute__ ((__packed__)) {
  uint8_t bootind; /* boot magic number (0x80 if bootable)*/
  uint8_t start_head; /* start of partition */
  uint8_t start_sec; 
  uint8_t start_cyl;
  uint8_t type;       /* type of partition (0x81 is MINIX) */
  uint8_t end_head;   /* end of partition */
  uint8_t end_sec;
  uint8_t end_cyl;
  uint32_t lFirst;    /* First sectore (LBA addressing)*/
  uint32_t size;      /* size of partitioin (in sectors) */
} part_entry;

typedef struct __attribute__ ((__packed__)) {
  part_entry entries[NR_PARTITIONS];  
} part_table;

typedef struct {
  uint32_t inode;
  unsigned char name[60];
}directory;

/* helper functions */
FILE *openImage(char *fname);
superblock getSuperBlockData(FILE *img, int offset, int verbose);
inode getInode(FILE *img, superblock sb, uint32_t i_num, uint32_t part_offset);
uint32_t getZoneSize(superblock sb);
int get_partition(FILE *img, int partition, int subpartition, int verbose);
void print_inode(inode *node);
void printPermissions(uint16_t mode);


directory getFinalDestination(superblock sb, uint32_t part_offset, 
                      FILE *img, char *pathname);
directory searchZones(FILE *img, superblock sb, uint32_t part_offset, 
                        inode node, char *name);
directory searchSingleZone(FILE *img, uint32_t zone_offset, 
                            uint16_t dir_count, char *name);
directory getDir(FILE *img, uint32_t offset);

#endif /* _MINHELPER_H */
