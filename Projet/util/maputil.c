#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <stdbool.h>

typedef struct Object{
  unsigned frames;
  int solid;
  int destructible;
  int collectible;
  int generator;
  int sizeName;
  char* name;
    }Object;

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

void print_objects(Object* objet[],int size){
  for (int i=0; i<size ; i++){
    printf("%s %d\n", objet[i]->name, objet[i]->solid);
  }
}

			

			
  

void change_objects(Object** tab, unsigned nbObj, char *file){
  int fd = open(file, O_RDWR);
  int width;
  int height;
  
  read(fd, &width, sizeof(unsigned));
  read(fd, &height, sizeof(unsigned));
  write(fd, &nbObj, sizeof(unsigned));       // On change le nombre d'objets

  lseek(fd, sizeof(int)*width*height, SEEK_CUR);    // On se déplace jusqu'aux objets

  for(int i = 0; i < nbObj; i++){
    write(fd, &tab[i]->frames, sizeof(unsigned));
    write(fd, &tab[i]->solid, sizeof(int));
    write(fd, &tab[i]->destructible, sizeof(int));
    write(fd, &tab[i]->collectible, sizeof(int));
    write(fd, &tab[i]->generator, sizeof(int));
    write(fd, &tab[i]->sizeName, sizeof(int));
    write(fd, &tab[i]->name, sizeof(char)*tab[i]->sizeName);
  }
  file_trunc(file);
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
  unsigned setobj = 0;
  
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
      read(fd, &buffer, sizeof(unsigned));
      printf("witdth: %d\n",buffer);
      //height
      read(fd, &buffer, sizeof(unsigned));
      printf("height: %d\n",buffer);
      //objects number
      read(fd, &buffer, sizeof(unsigned));
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
      lseek(fd, 2*sizeof(unsigned) ,SEEK_SET);
      read(fd, &buffer, sizeof(unsigned));
      unsigned newNbObj = (argc - 3) / 6;                   ///!\\\ buffer = 0 ? 
      if (newNbObj < buffer){
	puts("Less objects than expected \n");
	exit(EXIT_FAILURE);
      }
      setobj = newNbObj;
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
  if (setobj !=0){
    // printf("%d \n", setobj);
    // On Passe les arguments dans un tableau
    Object* Objets[setobj];
    unsigned tmpFra;
    int tmpSol;
    int tmpDestruct;
    int tmpCol;
    int tmpGen;
    int tmpSize ;
    char* tmpName;
    printf("%s\n", argv[4]);
    for(int i = 3, j = 0; i < argc-1; i += 6, j++){
      tmpSize = strlen(argv[i]);
      tmpName = argv[i];
      if (strcmp("destructible", argv[i+3]) == 0){
	tmpDestruct = 1;
      }
      else tmpDestruct = 0;
      if (strcmp("collectible", argv[i+4]) == 0){
	tmpCol = 1;
      }
      else tmpCol = 0;
      if (strcmp("generator",argv[i+5]) == 0){
	tmpGen = 1;
      }
      else tmpGen = 0;
      tmpFra = atoi(argv[i+1]);
      if (strcmp("air",argv[i+2]) == 0){
	tmpSol = 0;
      }
      else if (strcmp("semi-solid",argv[i+2]) == 0){
	tmpSol = 1;
      }
      else tmpSol = 2;
      Objets[j] = malloc(sizeof(Object));
      Objets[j]->frames = tmpFra;
      Objets[j]->solid = tmpSol;
      Objets[j]->destructible = tmpDestruct;
      Objets[j]->collectible = tmpCol;
      Objets[j]->generator = tmpGen;
      Objets[j]->sizeName = tmpSize;
      Objets[j]->name = tmpName;
    }
    //On effectue les changements
    print_objects(Objets, setobj);
    change_objects(Objets, setobj, argv[1]);
    printf("setobjects is what you need \n");

  }
  
  close(fd);
  return 1;

    


}
