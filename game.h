#ifndef GAME_H
#define GAME_H

#include "bst.h"

typedef enum { ARMOR, SWORD } ItemType;
typedef enum { PHANTOM, SPIDER, DEMON, GOLEM, COBRA } MonsterType;
typedef enum { NOT_VISITED, VISITED } IsVisited;

typedef struct Item {
    char* name;
    ItemType type;
    int value;
} Item;

typedef struct Monster {
    char* name;
    MonsterType type;
    int hp;
    int maxHp;
    int attack;
} Monster;

typedef struct Room {
    int id;
    int x, y;
    int visited;
    Monster* monster;
    Item* item;
    struct Room* next;
} Room;

typedef struct Player {
    int hp;
    int maxHp;
    int baseAttack;
    BST* bag;
    BST* defeatedMonsters;
    Room* currentRoom;
} Player;

typedef struct {
    Room* rooms;
    Player* player;
    int roomCount;
    int configMaxHp;
    int configBaseAttack;
} GameState;

// Monster functions
void freeMonster(void* data);
int compareMonsters(void* a, void* b);
void printMonster(void* data);

// Item functions
void freeItem(void* data);
int compareItems(void* a, void* b);
void printItem(void* data);

// Game functions
void addRoom(GameState* g);
Room* findRoom(Room* room, int, int);
Room* findRoomID(Room* room, int);
void freeRooms(Room* room);
int getCoordinates(GameState*, Room*, Room* ,int direction);
void initPlayer(GameState* g);
void playGame(GameState* g);
void freeGame(GameState* g);

#endif
