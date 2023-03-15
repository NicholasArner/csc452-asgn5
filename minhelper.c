#include "minhelper.h"
/* This contains helper functions that are used by both minget and minls */

void print_pt(part_table * pt);
void print_sb(superblock * sb);

/*Finding filesystem*/
FILE *openImage(char *fname)
{
  FILE *img_file;
  img_file = fopen(fname, "r");
  if(img_file == NULL)
  {
    perror("can't open file");
    exit(EXIT_FAILURE);
  }
  return img_file;
}

/*Finding superblock*/
superblock getSuperBlockData(FILE *img, int offset)
{
  int foundBlock;
  superblock sblock;
  foundBlock = fseek(img, offset, SEEK_SET);
  if(foundBlock != 0)
  {
    perror("can't seek");
    exit(foundBlock);
  }
  fread(&sblock, sizeof(superblock), 1, img);
  if(sblock.magic != MINIX_MAGIC)
  {
    printf("Bad magic number. (0x%04x)\nThis doesn't look like a MINIX filesystem\n", sblock.magic);
    exit(EXIT_FAILURE);
  }
  return sblock;
} 

directory getZone(uint16_t z1_size, uint16_t cur_zone, FILE *img)
{
  directory dir;
  uint32_t offset;
  offset = (z1_size*16) + (DIR_ENTRY_SIZE * cur_zone);

  fseek(img, offset, SEEK_SET);
  fread(&dir, sizeof(directory), 1, img);
  return dir;
}

uint16_t getZoneSize(superblock sb)
{
  return sb.blocksize << sb.log_zone_size;
}

/*TODO: need to work on indexing currectly only works to get inode 1*/
inode getInode(FILE *img, superblock sb, uint32_t i_num)
{
  inode i;
  fseek(img, ((sb.blocksize*INODE_BLOCK) + (sb.blocksize *i_num)), SEEK_SET);
  fread(&i, sizeof(inode), 1, img);
  return i;
}

int get_partition(FILE * img, int partition, int subpartition, int verbose){
  /* read the partition table at address 0x1BE and check that it is valid */
  /* returns the absolute offset of the requested partition */
  /* subpartition is optional */
  /* TODO: subpartition tables are at the same address? */
  
  part_table pt;
  int offset;  
  int bytes;
  if (partition < 0 || partition > 3){
    printf("get_partition: partition number must be between 1 and 4!\n");
    exit(EXIT_FAILURE);
  }  
  
  /* skip the first 1k bytes to read the super block */
  if (fseek(img, PART_TABLE_ADDR, SEEK_SET) < 0){
    perror("lseek");
    exit(EXIT_FAILURE);
  }

  if ((bytes = fread(&pt, sizeof(part_table), 1, img)) < 0){
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
      
    part_table sub_pt; 
 
    /* TODO: do subpartitions need to be betwween 1 and 4? */
    if (subpartition < 0 || subpartition > 3){
      printf("get_partition: subpartition number must be between 1 and 4!\n");
      exit(EXIT_FAILURE);
    }  
  
    /* skip the first 1k bytes to read the super block */
    if (fseek(img, offset + PART_TABLE_ADDR, SEEK_SET) < 0){
      perror("lseek");
      exit(EXIT_FAILURE);
    }
    
    if ((bytes = fread(&sub_pt, 1, sizeof(part_table), img)) < 0){
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

void print_pt(part_table * pt){
  /* print data for each valid entry in the partition table */
  int i;
  part_entry entry; 
  if (pt == NULL){
    printf("print_pt: null pointer!\n");
    return;
  }

  for (i=0; i<NR_PARTITIONS; i++){
    entry = pt->entries[i];
    if (entry.type == NO_PART){
      return; 
    }
    
    printf("Entry %d:\n", i);
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

void print_sb(superblock * sb){
  /* print data for each valid entry in the partition table */
  if (sb == NULL){
    printf("print_sb: null pointer!\n");
    return;
  }
  
  printf("Superblock:\n");
  printf("\tninodes: %d\n", sb->ninodes);
  printf("\ti_blocks: %d\n", sb->i_blocks);
  printf("\tz_blocks: %d\n", sb->z_blocks);
  printf("\tfirstdata: %x\n", sb->firstdata);
  printf("\tlog_zone_size: %d\n", sb->log_zone_size);
  printf("\tmax_file: %d\n", sb->max_file);
  printf("\tzones: %d\n", sb->zones);
  printf("\tmagic: %x\n", sb->magic);
  printf("\tblocksize: %d\n", sb->blocksize);
  printf("\tsubversion: %d\n", sb->subversion); 
}
