#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>



void change_size(int newHeight, int newWidth, char *file){
  int saveMap = open(file, O_RDWR);
  
  int width;
  int height;
  int nbObj;
  read(saveMap, &width, sizeof(unsigned));
  read(saveMap, &height, sizeof(unsigned));
  read(saveMap, &nbObj, sizeof(unsigned));
  
  // On récupere la matrice avant changement de taille
  int matrice[height][width];
  for(int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
      read(saveMap, &matrice[y][x], sizeof(int));
    }
  }

  // On récupère le reste du fichier
  int fin = lseek(saveMap, 0, SEEK_END);
  int debut = lseek(saveMap, sizeof(unsigned)*3 + width*height*sizeof(int), SEEK_SET);
  char *buffer = malloc(fin - debut);
  read(saveMap, buffer, fin - debut);

  // On change les info de taille
  lseek(saveMap, 0, SEEK_SET);
  write(saveMap, &newWidth, sizeof(unsigned));
  write(saveMap, &newHeight, sizeof(unsigned));
  lseek(saveMap, sizeof(unsigned), SEEK_CUR);

  // On écrit la nouvelle matrice
  int noObject = -1;
  
  for(int y = 0; y < newHeight; y++){
    for(int x = 0; x < newWidth; x++){
      if(y <= (newHeight - height) || x >= width){
	write(saveMap, &noObject, sizeof(int));
      }else{
	write(saveMap, &matrice[y-(newHeight-height)][x]);
      }
    }
  }

  // On réécrit le reste du fichier
  write(saveMap, buffer, fin - debut);
}
						


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
