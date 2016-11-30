#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <stdbool.h>

void read_map(char *file){
  int saveMap = open(file, O_RDONLY); 
  unsigned width;
  unsigned height;
  unsigned nbObj;
  read(saveMap, &width, sizeof(unsigned));
  read(saveMap, &height, sizeof(unsigned));
  read(saveMap, &nbObj, sizeof(unsigned));
  
  printf("%d %d %d\n", width, height, nbObj);
 
  // On récupere la matrice avant changement de taille
  int matrice[height][width];
  int tmp;

  for(int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
      read(saveMap, &tmp, sizeof(int));
      matrice[y][x] = tmp;
      printf("%d ",tmp);
    }
    printf("\n");
  }

  
  // On récupère la liste d'objets
  //Int debut = lseek(saveMap, sizeof(unsigned)*3 + width*height*sizeof(int), SEEK_SET);
  int tmpInt;
  unsigned tmpUnsigned;
  char buffer;
  for(int i = 0; i < nbObj; i++){
    //Frames
    read(saveMap, &tmpUnsigned, sizeof(unsigned));
    printf("%u ", tmpUnsigned);
    //Solid 
    read(saveMap, &tmpInt, sizeof(int));
    printf("%d ", tmpInt);
    //Destructible
    read(saveMap, &tmpInt, sizeof(int));
    printf("%d ", tmpInt);
    //Collectible
    read(saveMap, &tmpInt, sizeof(int));
    printf("%d ", tmpInt);
    //Generator
    read(saveMap, &tmpInt, sizeof(int));
    printf("%d ", tmpInt);
    //SizeName
    read(saveMap, &tmpInt, sizeof(int));
    printf("%d ", tmpInt);
    //Name
    for (int j = 0; j < tmpInt; j++){     
      read(saveMap, &buffer, sizeof(char));
      if( buffer == 0 ){
	printf(" \\0");
      }
      else {
	printf("%c", buffer);
      }
    }      
    printf("\n");
  }
  close(saveMap);
  }

int main(int argc, char** argv){
  read_map(argv[argc-1]);
  return EXIT_SUCCESS;
}
