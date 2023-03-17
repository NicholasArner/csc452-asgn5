#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "minhelper.h"

void printCurFile(directory dir, inode i_info);
void printDirContents(superblock sb, uint32_t part_offset,
                      FILE *img, inode node);
void printDir(FILE *img, uint32_t offset, superblock sb, 
              uint32_t part_offset);
int main(int argc, char *argv[]){
  int verbose = 0;
  int sub_part = NO_SUBPART;
  int pri_part = NO_PRIPART;
  int opt, part_offset = 0;
  char * image_name;
  char * path = NULL;
  superblock sb; 
  uint32_t zone_size;
  FILE * img;
  directory dir;
  inode node;

  while ((opt = getopt(argc, argv, "vp:s:")) != -1){
    switch(opt){
      case 'v': {
        printf("verbose\n");
        verbose = 1;
        break;
      }
      case 'p':{
        pri_part = atoi(optarg);
        break;
      }
      case 's':{
        sub_part = atoi(optarg);
        break;
      }
      case '?':{
       exit(EXIT_FAILURE);
      }
    }
  }
  
  if (sub_part != NO_SUBPART && pri_part == NO_PRIPART){
    printf("usage: minls [-v] [ -p part [-s subpart] ] imagefile [path]\n");
    exit(EXIT_FAILURE);
  } 

  if (argv[optind] == NULL){
    exit(EXIT_FAILURE);
  }else{
    image_name = argv[optind];
    path = argv[optind + 1]; 
  }

  /* open the image */
  img = openImage(image_name);
  
  if (pri_part != NO_PRIPART)
    part_offset = get_partition(img, pri_part, sub_part, verbose); 

  sb = getSuperBlockData(img, part_offset + OFFSET, verbose);
  zone_size = getZoneSize(sb);
  if(path == NULL)
  {
    node = getInode(img, sb, ROOT_DIR, part_offset);
    printf("/:\n");
    printDirContents(sb, part_offset, img, node);
    exit(EXIT_SUCCESS);
  }
  else
  {
    dir = getFinalDestination(sb, part_offset, img, path);
    node = getInode(img, sb, dir.inode, part_offset);
    /*print directory*/
    if(node.mode & IS_DIR)
    {
      printf("%s:\n", path);
      printDirContents(sb, part_offset, img, node); 
    }
    /*print files*/
    else if(dir.inode == 0)
    {
      perror("deleted file");
      exit(EXIT_FAILURE);
    }
    else 
    {
      printPermissions(node.mode);
      printf("%9u %s\n", node.size, path);
    }
    exit(EXIT_SUCCESS); 
  }
  
}

void printCurFile(directory dir, inode i_info)
{
  printf("%9u %s\n", i_info.size, dir.name);
}


void printDirContents(superblock sb, uint32_t part_offset, 
                      FILE *img, inode i_info)
{
  uint32_t tot_dir_cnt, offset, zone_size;
  uint16_t dir_cnt_zone, cur_zone, cur_dir, tot_dir;
  cur_dir = 0;
  tot_dir = 0;
  cur_zone = 0;
  zone_size = getZoneSize(sb);
  tot_dir_cnt = i_info.size / DIR_ENTRY_SIZE;
  dir_cnt_zone = zone_size / DIR_ENTRY_SIZE;

  while(cur_zone < DIRECT_ZONES)
  {
    while(cur_dir < dir_cnt_zone && tot_dir < tot_dir_cnt)
    {
      offset = part_offset + (zone_size*i_info.zone[cur_zone]) 
                + (DIR_ENTRY_SIZE * cur_dir);
      printDir(img, offset, sb, part_offset);
      cur_dir++;
      tot_dir++;
    }
    cur_dir = 0;
    /*iterates through zone 0 of inode 1 (firstdata loc)*/
    cur_zone++;    
  }
}
void printDir(FILE *img, uint32_t offset, superblock sb, uint32_t part_offset)
{
  directory dir;
  inode i_info;
  dir = getDir(img, offset);
  /*if name is null, at end of directory*/
  /*if dir or file exists*/
  if(dir.inode != 0)
  {
    /*add inode info like read permission*/
    i_info = getInode(img, sb, dir.inode, part_offset);
    printPermissions(i_info.mode);
    printCurFile(dir, i_info);
  }
}
