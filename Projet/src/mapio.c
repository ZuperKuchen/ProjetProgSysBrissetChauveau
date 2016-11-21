#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "error.h"

#ifdef PADAWAN


void map_new (unsigned width, unsigned height)
{
  map_allocate (width, height);

  for (int x = 0; x < width; x++)
    map_set (x, height - 1, 0); // Ground

  for (int y = 0; y < height - 1; y++) {
    map_set (0, y, 1); // Wall
    map_set (width - 1, y, 1); // Wall
  }

  map_object_begin (6);

  // Texture pour le sol
  map_object_add ("images/ground.png", 1, MAP_OBJECT_SOLID);
  // Mur
  map_object_add ("images/wall.png", 1, MAP_OBJECT_SOLID);
  // Gazon
  map_object_add ("images/grass.png", 1, MAP_OBJECT_SEMI_SOLID);
  // Marbre
  map_object_add ("images/marble.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE);
  // Fleurs
  map_object_add ("images/flower.png", 1, MAP_OBJECT_AIR);
  // Pieces
  map_object_add ("images/coin.png", 20, MAP_OBJECT_AIR | MAP_OBJECT_COLLECTIBLE);

  map_object_end ();

}

void map_save (char *filename)
{
  int saveMap = open(filename, O_WRONLY | O_CREAT, 0666);
  unsigned width = map_width();
  unsigned height = map_height();

  printf("Taille Width : %d Height : %d \n", width, height);
  
  // On enregistre en premier la taille de la carte
  write(saveMap, &width, sizeof(unsigned));
  write(saveMap, &height, sizeof(unsigned));

  // On enregistre le nombre d'objets disponibles
  int nbObj = map_objects();
  write(saveMap, &nbObj, sizeof(unsigned));
  
  // On enregistre la matrice, ligne par ligne en partant du haut (ligne 0)
  int matBuf;
  for(int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
      matBuf = map_get(x, y);
      write(saveMap, &matBuf, sizeof(int));
    }
  }

  // On enregistre les objets
  unsigned fraBuf;
  int solBuf;
  int desBuf;
  int colBuf;
  int genBuf;
  char *nameBuf;
  int strSizeBuf;
  
  for(int i = 0; i < nbObj; i++){
    fraBuf = map_get_frames(i);
    solBuf = map_get_solidity(i);
    desBuf = map_is_destructible(i);
    colBuf = map_is_collectible(i);
    genBuf = map_is_generator(i);
    nameBuf = map_get_name(i);
    strSizeBuf = strlen(nameBuf)+1;

    write(saveMap, &fraBuf, sizeof(unsigned));
    write(saveMap, &solBuf, sizeof(int));
    write(saveMap, &desBuf, sizeof(int));
    write(saveMap, &colBuf, sizeof(int));
    write(saveMap, &genBuf, sizeof(int));
    write(saveMap, &strSizeBuf, sizeof(int));
    write(saveMap, nameBuf, sizeof(char)*strSizeBuf);
  }  
}

void map_load (char *filename)
{
  int saveMap = open(filename, O_RDONLY);
  if(saveMap == -1){
    fprintf(stderr, "Le fichier n'existe pas\n");
    return;
  }
  unsigned width;
  unsigned height;

  // On récupère la taille de la carte
  read(saveMap, &width, sizeof(unsigned));
  read(saveMap, &height, sizeof(unsigned));

  // On récupère le nombre d'objets
  int nbObj;
  read(saveMap, &nbObj, sizeof(unsigned));

  // On crée et on récupère la matrice
  int matrice[height][width];
  for(int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
      read(saveMap, &matrice[y][x], sizeof(int));
      if(matrice[y][x] < -1 || matrice[y][x] > nbObj-1){
	fprintf(stderr, "Le fichier contient une ou plusieurs erreurs\n");
	return;
      }
    }
  }

  // On commence à créer la carte (allocation et matrice)
  map_allocate (width, height);

  for(int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
      map_set(x, y, matrice[y][x]);
    }
  }

  // On récupère les informations des objets et on les add dans la map
  map_object_begin (nbObj);

  unsigned fraBuf;
  int solBuf;
  int desBuf;
  int colBuf;
  int genBuf;
  char *nameBuf;
  int strSizeBuf;

  int objSpec;

  for(int i = 0; i < nbObj; i++){
    read(saveMap, &fraBuf, sizeof(unsigned));
    read(saveMap, &solBuf, sizeof(int));
    read(saveMap, &desBuf, sizeof(int));
    read(saveMap, &colBuf, sizeof(int));
    read(saveMap, &genBuf, sizeof(int));
    read(saveMap, &strSizeBuf, sizeof(int));
    nameBuf = malloc(sizeof(char) * strSizeBuf);
    read(saveMap, nameBuf, sizeof(char) * strSizeBuf);
    
    objSpec = solBuf;
    if(desBuf == 1) objSpec += 4;
    if(colBuf == 1) objSpec += 8;
    if(genBuf == 1) objSpec += 16;

    map_object_add (nameBuf, fraBuf, objSpec);

    free(nameBuf);
  }
    
  map_object_end ();
}

#endif
