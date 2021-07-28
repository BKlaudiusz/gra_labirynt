//
// Created by root on 1/25/21.
//

#ifndef PROJEKTGRA_COOMON_H
#define PROJEKTGRA_COOMON_H

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>
#include <locale.h>
/// struktura do komunikacji z zdalnym graczem
struct dataPlayerToComunicate
{
    sem_t sem;
    sem_t sem2;
    int move;
    int hello;
    int pid;
    int pidServer;
    int round;
    char map[5][5];
    int spawn_x;
    int spawn_y;
    int coin;
    int x;
    int y;
    int death;
    int budget;
    int busy;
    int alive;
};
#endif //PROJEKTGRA_COOMON_H
