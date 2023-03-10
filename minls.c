#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
  int verbose = 0;
  int sub_part = -1;
  int pri_part = -1;
  int opt;
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
        break;
      }
      case 's':{
        printf("subpartition: %s\n", optarg);
        break;
      }
      case '?':{
       exit(EXIT_FAILURE);
      }
    }
  }
  
  if (sub_part != -1 && pri_part == -1){
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

  exit(EXIT_SUCCESS); 
}

