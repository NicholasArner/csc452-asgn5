#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "minhelper.h"

int main(int argc, char *argv[]){
  int verbose = 0;
  int sub_part = NO_SUBPART;
  int pri_part = NO_PRIPART;
  int opt, offset, image_fd;
  char * image_name;
  char * path = NULL;

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
    printf("usage: minls [-v] [ -p part [-s subpart] ] imagefile [path]\n");
    exit(EXIT_FAILURE);
  } 

  if (argv[optind] == NULL){
    exit(EXIT_FAILURE);
  }else{
    image_name = argv[optind];
    path = argv[optind + 1]; 
  }

  printf("image: %s\n", image_name);
  printf("path: %s\n", path);
  
  /* open the image */

  if ((image_fd = open(image_name, O_RDONLY)) < 0){
    perror("open");
    exit(EXIT_FAILURE);
  }    

  offset = get_partition(image_fd, pri_part, sub_part, verbose); 
  
  printf("offset: %x\n", offset);
  
  exit(EXIT_SUCCESS); 
}


