#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include "minhelper.h"

void print_pt(struct part_table * pt);
void print_sb(struct superblock * sb);

int get_partition(int image_fd, int partition, int subpartition, int verbose){
  /* read the partition table at address 0x1BE and check that it is valid */
  /* returns the absolute offset of the requested partition */
  /* subpartition is optional */
  /* TODO: subpartition tables are at the same address? */
  
  struct part_table pt;
  int offset;  
  int bytes;
  if (partition < 0 || partition > 3){
    printf("get_partition: partition number must be between 1 and 4!\n");
    exit(EXIT_FAILURE);
  }  

  /* skip the first 1k bytes to read the super block */
  if (lseek(image_fd, PART_TABLE_ADDR, SEEK_SET) < 0){
    perror("lseek");
    exit(EXIT_FAILURE);
  }

  if ((bytes = read(image_fd, &pt, sizeof(struct part_table))) < 0){
    perror("read");
    exit(EXIT_FAILURE);
  } 
   
  offset = pt.entries[partition].lFirst; 
  /* check that the partition is a minix partition type */  
  
  if (verbose) print_pt(&pt);

  if (pt.entries[partition].type != MINIX_PART){
    printf("Invalid partition type (%x)\n", pt.entries[partition].type);
    printf("This doesn't look like a MINIX filesystem.\n");
  }
  
  if (subpartition != NO_SUBPART){
      
    struct part_table sub_pt; 
 
    /* TODO: do subpartitions need to be betwween 1 and 4? */
    if (subpartition < 0 || subpartition > 3){
      printf("get_partition: subpartition number must be between 1 and 4!\n");
      exit(EXIT_FAILURE);
    }  
  
    /* skip the first 1k bytes to read the super block */
    if (lseek(image_fd, offset + PART_TABLE_ADDR, SEEK_SET) < 0){
      perror("lseek");
      exit(EXIT_FAILURE);
    }
    
    if ((bytes = read(image_fd, &sub_pt, sizeof(struct part_table))) < 0){
      perror("read");
      exit(EXIT_FAILURE);
    }
    
    if (verbose) print_pt(&pt);
  
    if (pt.entries[partition].type != MINIX_PART){
      printf("Invalid partition type (%x)\n", pt.entries[partition].type);
      printf("This doesn't look like a MINIX filesystem.\n");
    } 

    offset = offset + sub_pt.entries[subpartition].lFirst;
  }
  
  return offset; 
}

void get_superblock(int image_fd, int offset, struct superblock * sb){
  /* read the first 1k bytes and check the magic number for MINIX filesystem */
  int bytes;
  
  if (sb == NULL){
    printf("ERROR: invalid superblock pointer %p\n", sb);
    exit(EXIT_FAILURE);
  }  
 
  /* skip the first 1k bytes to read the super block */
  if (lseek(image_fd, offset + BOOT_BLOCK_SIZE, SEEK_SET) < 0){
    perror("lseek");
    exit(EXIT_FAILURE);
  }

  if ((bytes = read(image_fd, sb, SUPER_BLOCK_SIZE)) < 0){
    perror("read");
    exit(EXIT_FAILURE);
  }  
   
  /* check that the filesystem is minix */ 
  if (sb->magic != MINIX_FS){
    printf("Bad magic number (%x)\n", sb->magic);
    printf("This doesn't look like a MINIX filesystem.\n");
  } 
}

void print_pt(struct part_table * pt){
  /* print data for each valid entry in the partition table */
  int i;
  struct part_entry entry; 
  if (pt == NULL){
    printf("print_pt: null pointer!\n");
    return
  }

  for (i=0; i<NR_PARTITIONS; i++){
    entry = pt->entries[i];
    if (entry.type == NO_PART){
      return; 
    }
    
    printf("Entry %d\n:", i);
    printf("\tbootind: %d\n", entry.bootind);
    printf("\tstart_head: %d\n", entry.start_head);
    printf("\tstart_sec: %d\n", entry.start_sec);
    printf("\ttype: %x\n", entry.type);
    printf("\tend_head: %d\n", entry.end_head);
    printf("\tend_sec: %d\n", entry.end_sec);
    printf("\tend_cyl: %d\n", entry.end_cyl);
    printf("\tlFirst: %x\n", entry.lFirst);
    printf("\tsize: %d\n", entry.size);
  }   
}

void print_sb(struct part_table * sb){
  /* print data for each valid entry in the partition table */
  if (sb == NULL){
    printf("print_sb: null pointer!\n");
    return;
  }
  
  printf("Superblock:\n");
  printf("\tninodes: %d\n", sb->inodes);
  printf("\ti_blocks: %d\n", sb->i_blocks);
  printf("\tz_blocks: %d\n", sb->zblocks;
  printf("\tfirstdata: %x\n", sb->firstdata);
  printf("\tlog_zone_size: %d\n", sb->log_zone_size);
  printf("\tmax_file: %d\n", sb->max_file);
  printf("\tzones: %d\n", sb->zones);
  printf("\tmagic: %x\n", sb->magic);
  printf("\tblocksize: %d\n", sb->blocksize);
  printf("\tsubversion: %d\n", sb->subversion); 
}
