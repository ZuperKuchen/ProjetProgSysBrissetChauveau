#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <stdbool.h>

void usage(int i){
  fprintf(stderr,"maputil <file> --option \n\n --option:--getwitdh\n          --getheight\n          --getobjects\n          --getinfo\n");
}


void file_trunc(char* filePath){
  int width;
  int height;
  int nbObj;

  int fd = open(filePath, O_RDWR);

  read(fd, &width, sizeof(unsigned));
  read(fd, &height, sizeof(unsigned));
  read(fd, &nbObj, sizeof(unsigned));

  lseek(fd, sizeof(int)*width*height, SEEK_CUR);

  int size;
  int tmpStr;
  
  for(int i = 0; i < nbObj; i++){
    lseek(fd, sizeof(unsigned) + sizeof(int)*4, SEEK_CUR);
    read(fd, &tmpStr, sizeof(int));
    size = lseek(fd, sizeof(char)*tmpStr, SEEK_CUR);
  }

  int realSize = lseek(fd, 0, SEEK_END);

  if(size < realSize){
    ftruncate(fd, size);
  }
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

  for(int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
      read(saveMap, &tmp, sizeof(int));
      matrice[y][x] = tmp;
    }
  }

  // On récupère le reste du fichier
  int fin = lseek(saveMap, 0, SEEK_END);
  int debut = lseek(saveMap, sizeof(unsigned)*3 + width*height*sizeof(int), SEEK_SET);
  char *buffer = malloc(fin - debut);
  read(saveMap, buffer, fin - debut);

  // On change les infos de taille
  lseek(saveMap, 0, SEEK_SET);
  write(saveMap, &newWidth, sizeof(unsigned));
  write(saveMap, &newHeight, sizeof(unsigned));
  lseek(saveMap, sizeof(unsigned), SEEK_CUR);

  // On écrit la nouvelle matrice
  int noObject = -1;
  
  for(int y = 0; y < newHeight; y++){
    for(int x = 0; x < newWidth; x++){
      if(y < (newHeight - height) || x >= width){
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

  file_trunc(file);
}

/*                    
 *
 *            MAIN
 *
 */


int main (int argc, char **argv){
  //Usages
  if (argc < 3) usage(0);
  char* file = argv[1];
  int fd = open(file, O_RDONLY);
  if (fd == -1) usage(1);
  

  //Initialisation variables getopt
  static struct option long_options[] = {                                                
    {"getwidth", no_argument, 0,'w'},
    {"getheight", no_argument, 0, 'h'},
    {"getobjects", no_argument, 0, 'o'},
    {"getinfo", no_argument, 0, 'a'},
    {"setwidth",required_argument,0,'W'},
    {"setheight", required_argument, 0,'H'},
    {"setobjects", required_argument, 0, 'O'},
    {"pruneobjects", no_argument, 0, 'p'}
  };
  char *optstring = "whoaW:H:O:P";
  int opt;
  int long_index=0;

  
  //Initialisation variables de test
  int newHeight = 0;
  int newWidth = 0;
  bool setobj = false;
  
  unsigned int buffer;
  
  while( (opt = getopt_long(argc, argv, optstring, long_options, &long_index)) != -1){
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
      printf("Height set to  %s \n", optarg);
      newHeight = atoi(optarg);
      break;
      
    case 'W':
      printf("Width set to %s \n", optarg);
      newWidth = atoi(optarg);
      break;
    case 'O':
      printf("--setobjects %c et ses arguments %s \n", opt, optarg);

      /*//
      
      FILE* args_file = fopen(optarg, "r");
      if (args_file != NULL){
	puts("Ca AVance \n");
      }
      char buf[1000];                                                               //tests
      fgets(buf,1000, args_file) != NULL;
      printf("%s",buf);
      printf("\n %d \n ",(int) strlen(buf));
      printf("--setobjects %c et ses arguments %s \n", opt, optarg+6);

      //*/

      setobj = true;
      break;
    case 'p':
      printf("--pruneobjects : %c \n", opt);
      break;
    default:
      usage(1);
      exit(EXIT_FAILURE);
    }
  }  
  
  // TRAITEMENT
  if( newHeight != 0 || newWidth != 0){
    change_size(newHeight, newWidth, file);
  }
  if ( setobj ){
    printf("setobjects is what you need \n");
  }


  
  close(fd);
  return 1;

    


}
