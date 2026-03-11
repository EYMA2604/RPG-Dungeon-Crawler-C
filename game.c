#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "utils.h"
#define YES 'V'
#define NO 'X'
// Map display functions
static void displayMap(GameState* g) {
    if (!g->rooms) return;
    
    // Find bounds
    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    for (Room* r = g->rooms; r; r = r->next) {
        if (r->x < minX) minX = r->x;
        if (r->x > maxX) maxX = r->x;
        if (r->y < minY) minY = r->y;
        if (r->y > maxY) maxY = r->y;
    }
    
    int width = maxX - minX + 1;
    int height = maxY - minY + 1;
    
    // Create grid
    int** grid = malloc(height * sizeof(int*));
    for (int i = 0; i < height; i++) {
        grid[i] = malloc(width * sizeof(int));
        for (int j = 0; j < width; j++) grid[i][j] = -1;
    }
    
    for (Room* r = g->rooms; r; r = r->next)
        grid[r->y - minY][r->x - minX] = r->id;
    
    printf("=== SPATIAL MAP ===\n");
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (grid[i][j] != -1) printf("[%2d]", grid[i][j]);
            else printf("    ");
        }
        printf("\n");
    }
    
    for (int i = 0; i < height; i++) free(grid[i]);
    free(grid);
}
static void displayRoomLegend(GameState* g) {
    //Displays a list of all rooms and whether they contain monsters (M) or items (I).
    printf("=== ROOM LEGEND ===\n");
    Room* currentR = g->rooms;
    while (currentR) {
        char isMonster,isItem;
        if (currentR->monster)
            isMonster = YES;
        else
            isMonster = NO;
        if (currentR->item)
            isItem = YES;
        else
            isItem = NO;
        printf("ID %d: [M:%c] [I:%c]\n",currentR->id,isMonster,isItem);
        currentR = currentR->next;
    }
    printf("===================\n");
}
static void displayInfo(GameState* g) {
    //display the current room's info.
    if (!g->player->currentRoom)
        return;
    printf("--- Room %d ---\n",g->player->currentRoom->id);
    if (g->player->currentRoom->monster) {
        printf("Monster: %s (HP:%d)\n",g->player->currentRoom->monster->name,
                                                g->player->currentRoom->monster->hp);
    }
    if (g->player->currentRoom->item) {
        printf("Item: %s\n",g->player->currentRoom->item->name);
    }
    printf("HP: %d/%d\n",g->player->hp,g->player->maxHp);
}
static void generalDisplay(GameState* g) {
    displayMap(g);
    displayRoomLegend(g);
    displayInfo(g);
}
/***PLAY FUNCTIONS***/
static void moveFunction(GameState*);
static void fightFunction(GameState*);
static void pickupFunction(GameState*);
static void bagFunction(GameState*);
static void defeatedFunction(GameState*);
/***GAME FUNCTIONS***/
static int victoryCheck(Room* room) {
    //// Check if all rooms are visited and all monsters are cleared (Victory condition).
    Room* currentRoom = room;
    while (currentRoom) {
        if (currentRoom->visited != VISITED)
            return 0;
        if (currentRoom->monster)
            return 0;
        currentRoom = currentRoom->next;
    }
    return 1;
}
void initPlayer(GameState* g) {
    // Initializes player stats and places them in the starting room (ID 0).
    if (g->player) {
        printf("Player exists.\n");
        return;
    }
    if (!g->rooms) {
        printf("Create rooms first\n");
        return;
    }
    g->player = malloc(sizeof(Player));
    g->player->bag = NULL;
    g->player->baseAttack = g->configBaseAttack;
    g->player->currentRoom = findRoomID(g->rooms,0);
    g->player->currentRoom->visited = VISITED;
    g->player->defeatedMonsters = NULL;
    g->player->maxHp = g->configMaxHp;
    g->player->hp = g->player->maxHp;
}
void addRoom(GameState* g) {
    // Handles the creation of a new room and links it to an existing one based on direction.
    if (g->rooms) {
        displayMap(g);
        displayRoomLegend(g);
    }
    Room* room = calloc(1,sizeof(Room));
    room->id = g->roomCount;
    room->visited = NOT_VISITED;
    if (g->rooms) {
        int attachedRoom = getInt("Attach to room ID: ");
        int direction = getInt("Direction (0=Up,1=Down,2=Left,3=Right): ");
        if (!getCoordinates(g,room,findRoomID(g->rooms,attachedRoom),direction)) {
            printf("Room exists there.\n");
            free(room);
            return;
        }
    }
    else {
        room->x = 0;
        room->y = 0;
    }
    int isMonster = getInt("Add monster? (1=Yes, 0=No): ");
    if (isMonster) {
        room->monster = malloc(sizeof(Monster));
        room->monster->name = getString("Monster name: ");
        room->monster->type = getInt("Type (0-4): ");
        room->monster->maxHp = getInt("HP: ");
        room->monster->hp = room->monster->maxHp;
        room->monster->attack = getInt("Attack: ");
    }
    int isItem = getInt("Add item? (1=Yes, 0=No): ");
    if (isItem) {
        room->item = malloc(sizeof(Item));
        room->item->name = getString("Item name: ");
        room->item->type = getInt("Type (0=Armor, 1=Sword): ");
        room->item->value = getInt("Value: ");
    }
    room->next = g->rooms;
    g->rooms = room;
    g->roomCount++;
    printf("Created room %d at (%d,%d)\n",room->id,room->x,room->y);
}
void playGame(GameState* g) {
    // Main game loop: handles user input and updates the game state until victory or quit.
    if (!g->player) {
        printf("Init player first\n");
        return;
    }
    if (g->player->defeatedMonsters == NULL)
        g->player->defeatedMonsters = calloc(1,sizeof(BST));
    if (g->player->bag == NULL)
        g->player->bag = calloc(1,sizeof(BST));
    while (1) {
        if (victoryCheck(g->rooms)) {
            printf("***************************************\n");
            printf("             VICTORY!                  \n");
            printf(" All rooms explored. All monsters defeated.\n");
            printf("***************************************\n");
            freeGame(g);
            exit(0);
        }
        generalDisplay(g);
        int choice = getInt("1.Move 2.Fight 3.Pickup 4.Bag 5.Defeated 6.Quit\n");
        switch (choice) {
            case 1: {
                moveFunction(g);
                break;
            }
            case 2: {
                fightFunction(g);
                break;
            }
            case 3: {
                pickupFunction(g);
                break;
            }
            case 4: {
                bagFunction(g);
                break;
            }
            case 5: {
                defeatedFunction(g);
                break;
            }
            case 6:{return;}
            default:break;
        }
    }
}
void freeGame(GameState* g) {
    // Recursively cleans up all allocated memory for the game, including BSTs and rooms.
    if (g->player->bag) {
        bstFree(g->player->bag->root,freeItem);
        free(g->player->bag);
    }
    if (g->player->defeatedMonsters) {
        bstFree(g->player->defeatedMonsters->root,freeMonster);
        free(g->player->defeatedMonsters);
    }
    if (g->rooms) {
        freeRooms(g->rooms);
    }
    free(g->player);
}
/***ROOM FUNCTIONS***/
int getCoordinates(GameState* g, Room* newRoom, Room* attachedRoom ,int direction) {
    //determining the new room's coordinates based on the direction from the attached room.
    if (!attachedRoom)
        return -1;
    if (direction == 0 && !findRoom(g->rooms,attachedRoom->x,(attachedRoom->y)-1)) {
        newRoom->x = attachedRoom->x;
        newRoom->y = (attachedRoom->y)-1;
        return 1;
    }
    if (direction == 1 && !findRoom(g->rooms,attachedRoom->x,(attachedRoom->y)+1)) {
        newRoom->x = attachedRoom->x;
        newRoom->y = (attachedRoom->y)+1;
        return 1;
    }
    if (direction == 2 && !findRoom(g->rooms,(attachedRoom->x)-1,attachedRoom->y)) {
        newRoom->x = (attachedRoom->x)-1;
        newRoom->y = attachedRoom->y;
        return 1;
    }
    if (direction == 3 && !findRoom(g->rooms,(attachedRoom->x+1),attachedRoom->y)) {
        newRoom->x = (attachedRoom->x)+1;
        newRoom->y = attachedRoom->y;
        return 1;
    }
    return 0;
}
// Helper functions to find a specific room based on its (x, y) coordinates or ID.
Room* findRoom(Room* room, int x, int y){
    if (room == NULL) {
        return NULL;
    }
    if (room->x == x && room->y == y) {
        return room;
    }
    return findRoom(room->next,x,y);
}
Room* findRoomID(Room* room, int id){
    if (room == NULL) {
        return NULL;
    }
    if (room->id == id) {
        return room;
    }
    return findRoomID(room->next,id);
}
void freeRooms(Room* room) {
    // free the memory of a room, its monster, and its item recursively.
    if (!room){
        return;
    }
    if (room->monster)
       freeMonster(room->monster);
    if (room->item)
        freeItem(room->item);
    freeRooms(room->next);
    free(room);
}
/***PLAY FUNCTIONS***/
static void moveFunction(GameState* g) {
    // Updates player position after checking for monsters and valid room existence
    if (!g->player->currentRoom)
        return;
    if (g->player->currentRoom->monster) {
        printf("Kill monster first\n");
        return;
    }
    int direction = getInt("Direction (0=Up,1=Down,2=Left,3=Right): ");
    int x = g->player->currentRoom->x;
    int y = g->player->currentRoom->y;

    switch (direction) {
        case 0: {
            x = g->player->currentRoom->x;
            y = g->player->currentRoom->y-1;
            break;
        }
        case 1: {
            x = g->player->currentRoom->x;
            y = g->player->currentRoom->y+1;
            break;
        }
        case 2: {
            x = g->player->currentRoom->x-1;
            y = g->player->currentRoom->y;
            break;
        }
        case 3: {
            x = g->player->currentRoom->x+1;
            y = g->player->currentRoom->y;
            break;
        }
        default: {}
    }
    if (!findRoom(g->rooms,x,y)) {
        printf("No room there.\n");
        return;
    }
    g->player->currentRoom = findRoom(g->rooms,x,y);
    g->player->currentRoom->visited = VISITED;
}
static void fightFunction(GameState* g) {
    // Turn-based combat logic: player and monster trade hits until one dies.
    if (!g->player->currentRoom->monster) {
        printf("No monster\n");
        return;
    }
    Monster* currentMonster = g->player->currentRoom->monster;
    Player* player = g->player;
    while (1) {
        currentMonster->hp -= g->player->baseAttack;
        printf("You deal %d damage. Monster HP: %d\n",player->baseAttack,max(currentMonster->hp,0));
        if (currentMonster->hp <= 0) {
            printf("Monster defeated!\n");
            bstInsert(&player->defeatedMonsters->root,currentMonster,compareMonsters);
            g->player->currentRoom->monster = NULL;
            return;
        }
        player->hp -= currentMonster->attack;
        printf("Monster deals %d damage. Your HP: %d\n",currentMonster->attack,max(player->hp,0));
        if (player->hp <= 0) {
            printf("--- YOU DIED ---\n");
            freeGame(g);
            exit(0);
        }
    }
}
static void pickupFunction(GameState* g) {
    // Transfers an item from the current room to the player's BST-based inventory.
    Item* item = g->player->currentRoom->item;
    if (g->player->currentRoom->monster) {
        printf("Kill monster first\n");
        return;
    }
    if (!item) {
        printf("No item here\n");
        return;
    }
    if (bstFind(g->player->bag->root,item,compareItems)) {
        printf("Duplicate item.\n");
        return;
    }
    bstInsert(&g->player->bag->root,item,compareItems);
    g->player->currentRoom->item = NULL;
    printf("Picked up %s\n",item->name);
}
static void bagFunction(GameState* g) {
    //prints a list of all the Items.
    printf("=== INVENTORY ===\n");
    if (!g->player->bag->root){
        printf("        Empty\n");
        return;
    }
    int choice = getInt("1.Preorder 2.Inorder 3.Postorder \n");
    switch (choice) {
        case 1: {
            bstPreorder(g->player->bag->root,printItem);
            break;
        }
        case 2: {
            bstInorder(g->player->bag->root,printItem);
            break;
        }
        case 3: {
            bstPostorder(g->player->bag->root,printItem);
            break;
        }
        default: {break;}
    }
}
static void defeatedFunction(GameState* g) {
    //prints a list of all the defeated monsters.
    printf("=== DEFEATED MONSTERS ===\n");
    if (!g->player->defeatedMonsters->root) {
        printf("        None\n");
        return;
    }
    int choice = getInt("1.Preorder 2.Inorder 3.Postorder \n");
    switch (choice) {
        case 1: {
            bstPreorder(g->player->defeatedMonsters->root,printMonster);
            break;
        }
        case 2: {
            bstInorder(g->player->defeatedMonsters->root,printMonster);
            break;
        }
        case 3: {
            bstPostorder(g->player->defeatedMonsters->root,printMonster);
            break;
        }
        default: {break;}
    }
}
/***ITEM FUNCTIONS***/
void printItem(void* data) {
    Item* item = data;
    int type = item->type;
    switch (type) {
        case 0: {
            printf("[ARMOR] %s - Value: %d\n",item->name,item->value);
            break;
        }
        case 1: {
            printf("[SWORD] %s - Value: %d\n",item->name,item->value);
            break;
        }
        default: {break;}
    }
}
int compareItems(void* a, void* b) {
    // Comparison logic for items: sorts by name, then value, then type.
    Item* item1 = a;
    Item* item2 = b;
    if (strcmp(item1->name,item2->name)==0) {
        if (item1->value == item2->value) {
            if (item1->type == item2->type)
                return 0;
            return item1->type - item2->type;
        }
        return (item1->value - item2->value);
    }
    return strcmp(item1->name,item2->name);
}
void freeItem(void* data) {
    Item* item = data;
    free(item->name);
    free(item);
}
/***MONSTERS FUNCTIONS***/
void printMonster(void* data) {
    Monster* monster = data;
    int type = monster->type;
    switch (type) {
        case 0: {
            printf("[%s] Type: Phantom, Attack: %d, HP: %d\n",
            monster->name,monster->attack,monster->maxHp);
            break;
        }
        case 1: {
            printf("[%s] Type: Spider, Attack: %d, HP: %d\n",
            monster->name,monster->attack,monster->maxHp);
            break;
        }
        case 2: {
            printf("[%s] Type: Demon, Attack: %d, HP: %d\n",
            monster->name,monster->attack,monster->maxHp);
            break;
        }
        case 3: {
            printf("[%s] Type: Golem, Attack: %d, HP: %d\n",
            monster->name,monster->attack,monster->maxHp);
            break;
        }
        case 4: {
            printf("[%s] Type: Cobra, Attack: %d, HP: %d\n",
            monster->name,monster->attack,monster->maxHp);
            break;
        }
        default: {break;}
    }
}
int compareMonsters(void* a, void* b) {
    // Comparison logic for monsters: sorts by name, attack, HP, and then type.
    Monster* monster1 = a;
    Monster* monster2 = b;
    if (strcmp(monster1->name,monster2->name)==0) {
        if (monster1->attack == monster2->attack) {
            if (monster1->maxHp == monster2->maxHp) {
                if (monster1->type == monster2->type)
                    return 0;
                return monster1->type - monster2->type;
            }
            return monster1->maxHp - monster2->maxHp;
        }
        return monster1->attack - monster2->attack;
    }
    return strcmp(monster1->name,monster2->name);
}
void freeMonster(void* data) {
    Monster* monster = data;
    free(monster->name);
    free(monster);
}