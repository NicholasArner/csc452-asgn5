#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "minhelper.h"

void getCurFile(inode i_info, uint32_t zoneSize, 
                uint32_t part_offset, FILE * img, FILE * out); 
void getDirContents(uint16_t zoneSize, superblock sb, uint32_t part_offset,   
                       char * target, FILE *img, FILE * out);
int pop_dir(char * path, char ** cur_dir, char ** rest_of_path);
char * format_path(char * path);

int main(int argc, char *argv[]){
  int verbose = 0;                                                              
  int sub_part = NO_SUBPART;                                                    
  int pri_part = NO_PRIPART;                                                    
  int opt, part_offset = 0;                                                     
  char * image_name;                                                            
  superblock sb;                                                                
  uint16_t zone_size;                                                           
  FILE * img = NULL;  
  FILE * out = NULL;
  char * srcpath = NULL;
  char * dstpath = NULL;
  
  char * dir = NULL; 
  char * new_path = NULL;

  while ((opt = getopt(argc, argv, "vp:s:")) != -1){
    switch(opt){
      case 'v': {
        printf("verbose\n");
        verbose = 1;
        break;
      }
      case 'p':{
        printf("partition: %s\n", optarg); 
        pri_part = atoi(optarg);
        break;
      }
      case 's':{
        printf("subpartition: %s\n", optarg);
        sub_part = atoi(optarg);
        break;
      }
      case '?':{
       exit(EXIT_FAILURE);
      }
    }
  }
  
  if (sub_part != NO_SUBPART && pri_part == NO_PRIPART){
    printf("usage: minget [-v] [ -p part [-s subpart] ] ");
    printf("imagefile srcpath [ dstpath ]\n");
    exit(EXIT_FAILURE);
  } 

  if ((argv[optind] == NULL) || (argv[optind + 1] == NULL)){
    printf("usage: minget [-v] [ -p part [-s subpart] ] ");
    printf("imagefile srcpath [ dstpath ]\n");
    exit(EXIT_FAILURE);
  }else{
    image_name = argv[optind];
    srcpath = argv[optind + 1];
    if (argv[optind + 2] != NULL)
      dstpath = argv[optind + 2];
    else{ 
      dstpath = ".";
      out = stdout;
    }
     
  }
 
  /* add a leading / in front of paths in case not specified */ 

  /*srcpath = format_path(srcpath);*/

  if (dstpath[0] != '.') {
    dstpath = format_path(dstpath);
  } 
  
  printf("image: %s\n", image_name);
  printf("srcpath: %s\n", srcpath);
  printf("dstpath: %s\n", dstpath);

  /* open the image */
  img = openImage(image_name);  
                                                                              
  if (pri_part != NO_PRIPART)                                                   
    part_offset = get_partition(img, pri_part, sub_part, verbose);              
                                                                                
  printf("looking for superblock at: %x\n", part_offset + OFFSET);              
  sb = getSuperBlockData(img, part_offset + OFFSET, verbose);                   
  zone_size = getZoneSize(sb);                                                  
  getDirContents(zone_size, sb, part_offset, srcpath, img, out);
  
  while ( !pop_dir(srcpath, &dir, &new_path) ){
    printf("dir: %s\n", dir);
    printf("new path: %s\n", new_path);
    srcpath = new_path;
  }                            
                                                                                
  exit(EXIT_SUCCESS);    
}

void getCurFile(inode i_info, uint32_t zoneSize, 
                uint32_t part_offset, FILE * img, FILE * out) 
{                                                                               
  /*drwxr-xr-x 64 .*/                                                           
  /*printf("%9u %s\n", i_info.size, dir.name);*/
  /* write until either run out of blocks or size */
  int zone_index = 0; 
  int remaining_bytes = i_info.size;
  int read_bytes;  
  char * buff; 
 
  /* allocate a buffer */
                                                                                
  if ((buff = malloc(zoneSize)) == NULL){                                       
    perror("malloc");                                                           
    exit(EXIT_FAILURE);                                                         
  }        

  while (remaining_bytes > 0){
    /* copy data from the correct zones to the output file */ 
    read_bytes = getDataFromZone(i_info, zoneSize, zone_index, remaining_bytes,
                    part_offset, buff, img);   
    fwrite(buff, read_bytes, 1, out);
 
    zone_index++; 
    remaining_bytes = remaining_bytes - read_bytes;
  }
                      
}                                                                               
                                                                              
void getDirContents(uint16_t zoneSize, superblock sb, uint32_t part_offset,   
                       char * target, FILE *img, FILE * out)
{                                                                               
  directory dir;                                                                
  inode i_info;                                                                 
  uint32_t i = 0;                                                               
  while(i >= 0)                                                                 
  {                                                                             
    dir = getZone(sb, zoneSize, i, part_offset, img);                           
    /*if name is null, at end of directory*/                                    
    if(dir.name[0] == '\0')                                                     
    {                                                                           
      break;                                                                    
    }                                                                           
    /*if dir or file exists*/                                                   
    else if(dir.inode != 0)                                                     
    {                                                                           
      /*add inode info like read permission*/                                   
      i_info = getInode(img, sb, dir.inode, part_offset);       
      /* check for target file */
      if (strncmp((char *) dir.name, target, DIRENT_NAME_SIZE) == 0){  
        if (i_info.mode & IS_DIR){
          printf("src path must be regular file!\n");
          exit(EXIT_FAILURE);
        }
        else 
          getCurFile(i_info, zoneSize, part_offset, img, out);
      }
    }                                                                           
    i++;                                                                        
  }                                                                             
}

int pop_dir(char * path, char ** cur_dir, char ** rest_of_path){
  /* pop the first directory off the path */
  /* find the first occurance of the '/' character and remove */
  /* everything before it */

  char * new_pos; 
 
  new_pos = strchr(path, '/');
  if (new_pos != NULL){
    *new_pos = 0; /* split up the string */
    *cur_dir = path;
    *rest_of_path = new_pos + 1;
    return 0;
  }
  return -1;
}

char * format_path(char * path){
  /* format path string */
  int len;
  char * ret;  
  
  len = strlen(path);  

  if ((ret = malloc(len + sizeof(char))) == NULL){
    perror("malloc");
    exit(EXIT_FAILURE);
  } 

  if (path[0] != '/'){
    ret[0] = '/';
    ret[1] = 0;
    ret = strcat(ret, path);
  }
  else ret = path;

  return ret;  
}
