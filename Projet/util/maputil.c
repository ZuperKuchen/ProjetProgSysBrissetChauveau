#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>

void usage(int i){
  fprintf(stderr,"maputil <file> --option \n\n --option:--getwitdh\n          --getheight\n          --getobjects\n          --getinfo\n");
}

void change_size(int newHeight, int newWidth, char *file){
  int saveMap = open(file, O_RDWR); 
  int width;
  int height;
  int nbObj;
  read(saveMap, &width, sizeof(unsigned));
  read(saveMap, &height, sizeof(unsigned));
  read(saveMap, &nbObj, sizeof(unsigned));

  if (newHeight == 0) newHeight = height;
  if (newWidth == 0) newWidth = width;
  
  // On récupere la matrice avant changement de taille
  int matrice[height][width];
  int tmp;

  //Attention mesdames et messieurs, l'erreur va commencer ! 
  for(int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
      read(saveMap, &tmp, sizeof(int));
      matrice[x][y] = tmp;
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
	tmp = matrice[y-(newHeight-height)][x];
	write(saveMap, &tmp, sizeof(int));
      }
    }
  }

  // On réécrit le reste du fichier
  write(saveMap, buffer, fin - debut);

  close(saveMap);
}


int main (int argc, char **argv){
  //Usages
  if (argc < 3) usage(0);
  char* file = argv[1];
  int fd = open(file, O_RDONLY);
  if (fd == -1) usage(1);
  

  
  static struct option long_options[] = {                                                 ////
    {"getwidth", no_argument, 0,'w'},
    {"getheight", no_argument, 0, 'h'},
    {"getobjects", no_argument, 0, 'o'},
    {"getinfo", no_argument, 0, 'a'},
    {"setwidth",required_argument,0,'W'},
    {"setheight", required_argument, 0,'H'}
  };
  int newHeight = 0;
  int newWidth = 0;
  unsigned int buffer;
  int opt;
  int long_index=0;
  while( (opt = getopt_long(argc, argv,"whoaW:H:", long_options, &long_index)) != -1){       ///
    switch(opt){
    case 'w':
      lseek(fd, 0 ,SEEK_SET);
      read(fd, &buffer, sizeof(unsigned int));
      printf("witdth: %d\n",buffer);
      break;
      
    case 'h':
      lseek(fd, sizeof(unsigned int) ,SEEK_SET);
      read(fd, &buffer, sizeof(unsigned int));
      printf("height: %d\n",buffer);
      break;
      
    case 'o':
      lseek(fd, 2*sizeof(unsigned int) ,SEEK_SET);
      read(fd, &buffer, sizeof(unsigned int));
      printf("objects number: %d\n",buffer);
      break;
      
    case 'a':
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
      break;
      
    case 'H':
      printf("%c et %s \n",opt, optarg);
      newHeight = atoi(optarg);
      break;
      
    case 'W':
      printf("%c et %s \n",opt, optarg);
      newWidth = atoi(optarg);
      printf("%d \n",newWidth);
      break;
      
    default:
      usage(1);
      exit(EXIT_FAILURE);
    }
  }  
  
  // TRAITEMENT
  change_size(newHeight, newWidth, file);
  close(fd);
  return 1;

    


}
