#ifndef _MINHELPER_H
#define _MINHELPER_H

#include <stdint.h>

#define MINIX_FS           0x4D5A
#define INODE_SIZE_BYTES   64
#define DIR_SIZE_BYTES     64
#define NR_PARTITIONS      4
#define PART_TABLE_ADDR    0x1BE
#define SUPER_BLOCK_SIZE   1000
#define BOOT_BLOCK_SIZE    1000
#define NO_PART            0x0
#define MINIX_PART         0x81
#define NO_SUBPART         -1
#define NO_PRIPART         -1
#define DIRECT_ZONES 7

/* TODO: should all structs be padded? */
struct __attribute__ ((__packed__)) superblock { /* Minix Version 3 Superblock
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
};

struct __attribute__ ((__packed__)) inode {
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
};

/* partition table entry */
/* TODO: will partition table always be the same size? */

struct __attribute__ ((__packed__)) part_entry {
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
};

struct __attribute__ ((__packed__)) part_table{
  struct part_entry entries[NR_PARTITIONS];  
};

int get_partition(int image_fd, int partition, int subpartition, int verbose);
void get_superblock(int image_fd, int offset, struct superblock * sb);

#endif /* _MINHELPER_H */
