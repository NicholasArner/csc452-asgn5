#include "minhelper.h"
/* This contains helper functions that are used by both minget and minls */

int check_partition_table(FILE * img, int offset);
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
superblock getSuperBlockData(FILE *img, int offset, int verbose)
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
    fprintf(stderr, "Bad magic number. "
      "(0x%04x)\nThis doesn't look like a MINIX filesystem\n", 
      sblock.magic, strerror(errno));
    exit(EXIT_FAILURE);
  }
  
  if (verbose) print_sb(&sblock);  

  return sblock;
} 

/*main call to search path for given file/directory*/
directory getFinalDestination(superblock sb, uint32_t part_offset, 
                      FILE *img, char *pathname)
{
  directory dir;
  /*starts with root node that is located at inode 1*/
  inode node = getInode(img, sb, ROOT_DIR, part_offset);
  char *path = strdup(pathname);
  char * token = strtok(path, "/");
  
  /*search path*/
  while (token != NULL) {
    dir = searchZones(img, sb, part_offset, node, token);
    if(strcmp(dir.name, token) == 0) {
      node = getInode(img, sb, dir.inode, part_offset);
      token = strtok(NULL, "/");
    }
    else {
      exit(EXIT_FAILURE);
    }
  }
  /*found path*/
  return dir;
}

directory searchZones(FILE *img, superblock sb, uint32_t part_offset, 
                      inode node, char *name)
{
  uint32_t tot_dir_cnt, zone_offset, zone_size;
  uint16_t dir_cnt_zone, cur_zone, cur_dir;
  directory dir;
  cur_zone = 0;
  cur_dir = 0;
  zone_size = getZoneSize(sb);
  tot_dir_cnt = node.size / DIR_ENTRY_SIZE;
  dir_cnt_zone = zone_size / DIR_ENTRY_SIZE;
  while(cur_zone < DIRECT_ZONES && tot_dir_cnt > 0)
  {
    zone_offset = part_offset + (zone_size*node.zone[cur_zone]);
    dir = searchSingleZone(img, zone_offset, dir_cnt_zone, name);
    if(strcmp(dir.name, name) == 0)
    {
      return dir;
    }
    tot_dir_cnt -= dir_cnt_zone;
    cur_zone++;
  }
  return dir;
}

/*searches a zone for the direct*/
directory searchSingleZone(FILE *img, uint32_t zone_offset, 
                          uint16_t dir_count, char *name)
{
  uint32_t offset;
  directory dir;
  uint16_t cur_dir = 0;
  while(cur_dir < dir_count)
  {
    offset = zone_offset + (DIR_ENTRY_SIZE * cur_dir);
    dir = getDir(img, offset);
    if(strcmp(dir.name, name) == 0)
    {
      return dir;
    }
    cur_dir++;
  }
  return dir;
}

/*grabs a given dir in a zone*/
directory getDir(FILE *img, uint32_t offset)
{
  directory dir;
  fseek(img, offset, SEEK_SET);
  fread(&dir, sizeof(directory), 1, img);
  return dir;
}

int getDataFromZone(inode i_info, uint16_t zoneSize, uint16_t cur_zone,
        uint16_t remaining_bytes, uint32_t part_offset, char * buff ,FILE *img)
{
  /* returns the number of bytes read and puts the zone contents in buffer */
  uint32_t offset, buff_size;
  
  /* either read the remaining amount of bytes or a full zone */
  buff_size = (remaining_bytes < zoneSize) ? remaining_bytes : zoneSize; 

  offset = part_offset + i_info.zone[cur_zone]*zoneSize;
  fseek(img, offset, SEEK_SET);
  fread(buff, buff_size, 1, img);
  return buff_size;
}

directory getZone(superblock sb, uint16_t z1_size, 
                  uint16_t cur_zone, uint32_t part_offset, FILE *img)
{
  directory dir;
  uint32_t offset;
  offset = part_offset + (z1_size*sb.firstdata) + (DIR_ENTRY_SIZE * cur_zone);
  fseek(img, offset, SEEK_SET);
  fread(&dir, sizeof(directory), 1, img);
  return dir;
}

uint32_t getZoneSize(superblock sb)
{
  return sb.blocksize << sb.log_zone_size;
}

/*TODO: need to work on indexing currectly only works to get inode 1*/
inode getInode(FILE *img, superblock sb, uint32_t i_num, uint32_t part_offset)
{
  inode i;
  uint32_t offset = part_offset + INODE_SIZE * (i_num - 1);
  fseek(img, ((sb.blocksize*INODE_BLOCK) + offset), SEEK_SET);
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
    perror("get_partition: partition number must be between 0 and 3!\n");
    exit(EXIT_FAILURE);
  }  

  /* check for valid partition table, need to check bytes 510 and 511 */  
  if (!check_partition_table(img, 0)){
    perror("Invalid partition table!\n");
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
   
  offset = pt.entries[partition].lFirst * SECTOR_SIZE; 
  /* check that the partition is a minix partition type */  
  
  if (verbose) {
    printf("Primary Partition Table: \n");
    print_pt(&pt);
  }

  if (pt.entries[partition].type != MINIX_PART){
    printf("Invalid partition type (%x)\n", pt.entries[partition].type);
    perror("This doesn't look like a MINIX filesystem.\n");
    exit(EXIT_FAILURE);
  }
  
  if (subpartition != NO_SUBPART){
      
    part_table sub_pt; 
 
    /* TODO: do subpartitions need to be betwween 1 and 4? */
    if (subpartition < 0 || subpartition > 3){
      perror("get_partition: subpartition number must be between 0 and 3!\n");
      exit(EXIT_FAILURE);
    }  
  
    /* check for valid partition table, need to check bytes 510 and 511 */  
    if (!check_partition_table(img, offset)){
      perror("Invalid subpartition table!\n");
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
    
    if (verbose) {
      printf("Subpartition Table: \n");
      print_pt(&sub_pt);
    }
  
    if (sub_pt.entries[subpartition].type != MINIX_PART){
      printf("Invalid partition type (%x)\n", pt.entries[partition].type);
      perror("This doesn't look like a MINIX filesystem.\n");
      exit(EXIT_FAILURE);
    } 
    
    /* even subpartitions are absolute addresses */
    offset = sub_pt.entries[subpartition].lFirst * SECTOR_SIZE;
  }
  
  return offset; 
}

int check_partition_table(FILE * img, int offset){
  /* 
   * helper function that checks for valid partition table 
   * offset is only used when checking a subpartition table, otherwise it is 0
   */
  char valid1, valid2;  
  int bytes;

  if (fseek(img, VALID_TABLE_ADDR1, SEEK_SET) < 0){
    perror("lseek");
    exit(EXIT_FAILURE);
  }

  if ((bytes = fread(&valid1, sizeof(char), 1, img)) < 0){
    perror("read");
    exit(EXIT_FAILURE);
  } 
 
  if (fseek(img, VALID_TABLE_ADDR2, SEEK_SET) < 0){
    perror("lseek");
    exit(EXIT_FAILURE);
  }

  if ((bytes = fread(&valid2, sizeof(char), 1, img)) < 0){
    perror("read");
    exit(EXIT_FAILURE);
  } 

  return (valid1 == VALID_TABLE_BYTE1) || (valid2 == VALID_TABLE_BYTE2);
  
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
      continue;
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

void print_inode(inode * i){
  /* print data for each valid entry in the partition table */
  if (i == NULL){
    printf("print_inode: null pointer!\n");
    return;
  }
  
  printf("File inode:\n");
  printf("\tuint16_t mode: %x  ", i->mode);
  printPermissions(i->mode);
  printf("\n\tuint16_t links: %d\n", i->links);
  printf("\tuint16_t uid: %d\n", i->uid);
  printf("\tuint16_t gid: %d\n", i->gid);
  printf("\tuint32_t size: %d\n", i->size); 
  printf("\tuint32_t atime %d --- %s\n", i->atime);
  printf("\tuint32_t mtime %d--- %s\n", i->mtime);
  printf("\tuint32_t ctime %d--- %s\n", i->ctime);
  printf("\tDirect zones:\n");
  printf("\tzone[0] = %d\n", i->zone[0]); 
  printf("\tzone[1] = %d\n", i->zone[1]); 
  printf("\tzone[2] = %d\n", i->zone[2]); 
  printf("\tzone[3] = %d\n", i->zone[3]); 
  printf("\tzone[4] = %d\n", i->zone[4]); 
  printf("\tzone[5] = %d\n", i->zone[5]); 
  printf("\tzone[6] = %d\n", i->zone[6]); 
  printf("\tuint32_t indirect: %d\n", i->indirect);
  printf("\tuint32_t double: %d\n", i->two_indirect);
}
void printPermissions(uint16_t mode)
{
  char perm[11] = "";
  char r = 'r';
  char w = 'w';
  char x = 'x';
  char d = 'd';
  char not = '-';
  (mode & IS_DIR) ? strncat(perm, &d, 1) : strncat(perm, &not, 1);
  (mode & OWN_R) ? strncat(perm, &r, 1) : strncat(perm, &not, 1);
  (mode & OWN_W) ? strncat(perm, &w, 1) : strncat(perm, &not, 1);
  (mode & OWN_X) ? strncat(perm, &x, 1) : strncat(perm, &not, 1);
  (mode & GRP_R) ? strncat(perm, &r, 1) : strncat(perm, &not, 1);
  (mode & GRP_W) ? strncat(perm, &w, 1) : strncat(perm, &not, 1);
  (mode & GRP_X) ? strncat(perm, &x, 1) : strncat(perm, &not, 1);
  (mode & OTR_R) ? strncat(perm, &r, 1) : strncat(perm, &not, 1);
  (mode & OTR_W) ? strncat(perm, &w, 1) : strncat(perm, &not, 1);
  (mode & OTR_X) ? strncat(perm, &x, 1) : strncat(perm, &not, 1);
  printf("%s ", perm);
}




