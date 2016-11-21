#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main (int argc, char **argv){

  //Usage
  if(argc != 3){
    fprintf(stderr,"maputil <file> --option \n\n --option:--getwitdh\n          --getheight\n          --getobjects\n          --getinfo\n");
    exit(EXIT_FAILURE);
  }

  //Ouverture
  int fd = open(argv[1], O_RDONLY);
  if (fd == -1){
    fprintf(stderr,"erreur lors de l'ouverture du fichier \n");
  }
  unsigned int buffer;
  char *arg = argv[2];
  
  if (strcmp(arg,"--getwitdh") == 0){
    lseek(fd, 0 ,SEEK_SET);
    read(fd, &buffer, sizeof(unsigned int));
    printf("witdth: %d\n",buffer);
  }
  
  else if (strcmp(arg,"--getheight") == 0){
    lseek(fd, sizeof(unsigned int) ,SEEK_SET);
    read(fd, &buffer, sizeof(unsigned int));
    printf("height: %d\n",buffer);
  }
  
  else if (strcmp(arg,"--getobjects") == 0){
    lseek(fd, 2*sizeof(unsigned int) ,SEEK_SET);
    read(fd, &buffer, sizeof(unsigned int));
    printf("objects number: %d\n",buffer);
  }
  
  else if (strcmp(arg,"--getinfo") == 0){
    //width
    lseek(fd, 0 ,SEEK_SET);
    read(fd, &buffer, sizeof(unsigned int));
    printf("witdth: %d\n",buffer);
    //height
    read(fd, &buffer, sizeof(unsigned int));
    printf("height: %d\n",buffer);
    //objects number
    read(fd, &buffer, sizeof(unsigned int));
    printf("objects number: %d\n",buffer);
  }
  else{
    puts("Option -- invalide !\n");
  }
  close(fd);
  return 1;
}
