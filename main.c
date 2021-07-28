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
#include "coomon.h"
///struktura dropow
struct dropedCoin
{
    int x;
    int y;
    int value;
};

/// stuktura do obslugi graczy
struct data_playerToServer
{
    int pid;
    int alive;
    int spawn_x;
    int spawn_y;
    int coin;
    int x;
    int y;
    int death;
    int budget;
    int freeze;
    char name[9];
};
/// mapy do gry
char map[][55]=
        {
            {"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXX                                XXXXXXXX"},
            {"XXXXXX                   XXXXX  XXXXXXXXXXXXXXXXXXXXXXX"},
            {"XXXXXXX                XXXXXXX  XXXXXXXXXXXXXXXXXXXXXXX"},
            {"XXXXXXXXXXXX           XXXXXXX  XXXXXXXXXXXXXXXXXXXXXXX"},
            {"XXXX                     XXXXX  XXXXXXXXXXXX   XXXXXXXX"},
            {"XXXXXXX         XXXXX    XXXXX  XXXXXXXXXX     XXXXXXXX"},
            {"XXXXXXX        XXXXXXXXX   XXX              XXXXXXXXXXX"},
            {"######### # ###########    XXXXXXXXXXXX    XXXXXXXXXXXX"},
            {"XXX                 XXX    XXXXXXXXXXXX  XXXXXXXXXXXXXX"},
            {"XXX                 XXX                              XX"},
            {"XXX                        XXXXXXXXXXXX  XXXXXXXXXXXXXX"},
            {"XXX                     ###XXXXXXXXXXXX  XXXXXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXX XXXXXXX    XXXXXXXXXXXX  XXXXXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXX  XXXXXX      A   XXXXXX  XXXXXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXX   XXXXX          XXXXX   XXXXXXXXXXXXXX"},
            {"XXXXXXXX                                  XXXXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXXXXX    XXXXXX  XXX   XXX      XXXXXXXXXX"},
            {"XXXXXXXXXXXXXXXXXX    XXXXXXXXXXX   XXX      XXXXXXXXXX"},
            {"XXXXXXXXXXXXXXXXXX    XXXXXXX  XXX  XXX    XXXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXXXXX      XXXXXXXXX   XXXX   XXXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXXXX       ##########  XXXX    XXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXX      XXX XXXXXX      ####   XXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXX      XXXXXXXXXX            XXXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXX     XXXXXXXXXXXX           XXXXXXXXXXXX"},
            {"XXXX######################XXXXXXX   XXXXXXXXXXXXXXXXXXX"},
            {"XX######################            XXXXXXXXXXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXXXXXXX#XXXXXXXX    XXXXXXXXXXXXXXXXXXXXXX"},
            {"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"},
        };
char mapToGame[][55]=
{
        {"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXX                           T  c XXXXXXXX"},
        {"XXXXXX             T     XXXXX  XXXXXXXXXXXXXXXXXXXXXXX"},
        {"XXXXXXX        T       XXXXXXX  XXXXXXXXXXXXXXXXXXXXXXX"},
        {"XXXXXXXXXXXX           XXXXXXX  XXXXXXXXXXXXXXXXXXXXXXX"},
        {"XXXX         c           XXXXX  XXXXXXXXXXXX T XXXXXXXX"},
        {"XXXXXXX         XXXXX    XXXXX  XXXXXXXXXX     XXXXXXXX"},
        {"XXXXXXX        XXXXXXXXX   XXX              XXXXXXXXXXX"},
        {"######### # ###########    XXXXXXXXXXXX    XXXXXXXXXXXX"},
        {"XXX                 XXX    XXXXXXXXXXXX  XXXXXXXXXXXXXX"},
        {"XXX                 XXX                           CCcXX"},
        {"XXX                        XXXXXXXXXXXX  XXXXXXXXXXXXXX"},
        {"XXX  c                  ###XXXXXXXXXXXX  XXXXXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXX XXXXXXX    XXXXXXXXXXXX  XXXXXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXX  XXXXXX      A   XXXXXX  XXXXXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXX   XXXXX          XXXXX   XXXXXXXXXXXXXX"},
        {"XXXXXXXX                                  XXXXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXXXXX    XXXXXX  XXX   XXX      XXXXXXXXXX"},
        {"XXXXXXXXXXXXXXXXXX    XXXXXXXXXXX   XXX      XXXXXXXXXX"},
        {"XXXXXXXXXXXXXXXXXX    XXXXXXX  XXX  XXX    XXXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXXXXX      XXXXXXXXXc  XXXX   XXXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXXXX       ##########  XXXX    XXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXX  C   XXX XXXXXX      ####   XXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXX      XXXXXXXXXX        C   XXXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXX     XXXXXXXXXXXX           XXXXXXXXXXXX"},
        {"XXXX######################XXXXXXX   XXXXXXXXXXXXXXXXXXX"},
        {"XX######################   C        XXXXXXXXXXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXXXXXXX#XXXXXXXX    XXXXXXXXXXXXXXXXXXXXXX"},
        {"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"},
};

int gameRound;
struct  dropedCoin dropedCoinArray[50]={0};
int EndGame =0 ;
struct dataPlayerToComunicate* pdata_shared[2];
struct data_playerToServer dataPlayer[2];
char *mem_names[2] = {"/player1_data", "/player2_data"};
pthread_mutex_t mutex;
int pl[2];
WINDOW * screen;

void setdisplay();
void* beast(void * arg);
int findNearbyPlayer(int x,int y );
void displaymap(int a);
void initPlayers();
int main() {

    pthread_mutex_init(&mutex,NULL);
    int amountDrop=0;
    /// inicjalizacja pamieci wspoldzielonej pol gracza oraz pokazanie mapy
    pid_t pid = getpid();
    gameRound = 0;
    srand(time(NULL));

    for (int i = 0; i < 1; ++i)
    {
        pl[i] = shm_open(mem_names[i], O_CREAT | O_RDWR, 0600);
        ftruncate(pl[i], sizeof(struct dataPlayerToComunicate));
        pdata_shared[i] = (struct dataPlayerToComunicate*)mmap(NULL, sizeof(struct dataPlayerToComunicate), PROT_READ | PROT_WRITE, MAP_SHARED, pl[i], 0);
        pdata_shared[i]->move = 0;
        pdata_shared[i]->busy = 0;
        pdata_shared[i]->pid = 0;
        pdata_shared[i]->alive = 0;
        pdata_shared[i]->coin = 0;
        pdata_shared[i]->budget = 0;
        pdata_shared[i]->pidServer = getppid();
        sem_init(&pdata_shared[i]->sem, 1, 1);
        sem_init(&pdata_shared[i]->sem2, 1, 1);
    }

    initPlayers();
    setdisplay();
    displaymap('a');


    /// uruchomienie besti w watku

    pthread_t beastThread;
    pthread_create(&beastThread,NULL,beast,NULL);

    int command;
    while (command!='b')
    {
        command = wgetch(screen);
        pthread_mutex_lock(&mutex);
        /// obsluga gracza na serwerze oraz dodawanie elementow
        if(command=='b')
            break;

        int nowyx,nowyy;
        if(command>=0) {
            nowyx = dataPlayer[0].x;
            nowyy = dataPlayer[0].y;
            if (command == 'a') {
                nowyy--;
            } else if (command == 's') {
                nowyx++;
            } else if (command == 'w') {
                nowyx--;
            } else if (command == 'd') {
                nowyy++;
            }else
            {
                while (1) {
                    int r = rand() % 29;
                    int z = rand() % 55;
                    if (mapToGame[r][z] == ' ') {
                        if(command=='c')
                            mapToGame[r][z] = 'c';
                        if(command=='t')
                            mapToGame[r][z] = 't';
                        if(command=='T')
                            mapToGame[r][z] = 'T';
                        break;
                    }
                }
            }
            int death = 0;
            if(dataPlayer[0].alive == 1) {
                if (dataPlayer[0].freeze == 0) {
                    if (mapToGame[nowyx][nowyy] != 'X') {
                        if (map[dataPlayer[0].x][dataPlayer[0].y] == 'A') {
                            mapToGame[dataPlayer[0].x][dataPlayer[0].y] = 'A';
                        }
                        if (mapToGame[nowyx][nowyy] == 'c') {
                            mapToGame[dataPlayer[0].x][dataPlayer[0].y] = ' ';
                            dataPlayer[0].coin++;
                        } else if (mapToGame[nowyx][nowyy] == 'T') {
                            mapToGame[dataPlayer[0].x][dataPlayer[0].y] = ' ';
                            dataPlayer[0].coin += 10;
                        } else if (mapToGame[nowyx][nowyy] == 'C') {
                            mapToGame[dataPlayer[0].x][dataPlayer[0].y] = ' ';
                            dataPlayer[0].coin += 50;
                        } else if (mapToGame[nowyx][nowyy] == 'D') {
                            for (int i = 0; i < amountDrop; i++) {
                                if (nowyx == dropedCoinArray[i].x && nowyy == dropedCoinArray[i].y) {
                                    if (dropedCoinArray[i].value != -1) {
                                        dataPlayer[0].coin += dropedCoinArray[i].value;
                                        dropedCoinArray[i].value = -1;
                                    }
                                }
                            }
                            mapToGame[dataPlayer[0].x][dataPlayer[0].y] = ' ';
                        } else if (mapToGame[nowyx][nowyy] == 'A') {
                            dataPlayer[0].budget += dataPlayer[0].coin;
                            dataPlayer[0].coin = 0;
                            mapToGame[dataPlayer[0].x][dataPlayer[0].y] = ' ';
                        } else if (mapToGame[nowyx][nowyy] == '#') {
                            mapToGame[dataPlayer[0].x][dataPlayer[0].y] = map[dataPlayer[0].x][dataPlayer[0].y];
                            dataPlayer[0].freeze = 4;
                        } else if (mapToGame[nowyx][nowyy] == '2') {
                            /// drop z zebranych monet graczy
                            int valueDrop = dataPlayer[0].coin + dataPlayer[1].coin;
                            if (valueDrop >= 0) {
                                dropedCoinArray[amountDrop].value = valueDrop;
                                dropedCoinArray[amountDrop].x = nowyx;
                                dropedCoinArray[amountDrop].y = nowyy;
                                mapToGame[nowyx][nowyy] = 'D';
                                amountDrop++;
                            }
                            /// flaga do gracze ida na spawn
                            death = 1;
                        } else
                        {
                            mapToGame[dataPlayer[0].x][dataPlayer[0].y] = map[dataPlayer[0].x][dataPlayer[0].y];
                        }


                        if (death == 0) {
                            /// gracz poprostu sie przesuwa
                            dataPlayer[0].x = nowyx;
                            dataPlayer[0].y = nowyy;
                            mapToGame[nowyx][nowyy] = '1';
                        } else {
                            ///gracze ida na spawn
                            mapToGame[dataPlayer[0].x][dataPlayer[0].y] = map[dataPlayer[0].x][dataPlayer[0].y];
                            mapToGame[dataPlayer[1].x][dataPlayer[1].y] = map[dataPlayer[1].x][dataPlayer[1].y];
                            for (int i = 0; i <= 1; i++) {
                                dataPlayer[i].coin = 0;
                                dataPlayer[i].x = dataPlayer[i].spawn_x;
                                dataPlayer[i].y = dataPlayer[i].spawn_y;
                                dataPlayer[i].death++;
                            }
                            mapToGame[nowyx][nowyy] = 'D';
                            mapToGame[dataPlayer[0].x][dataPlayer[0].y] = '1';
                            mapToGame[dataPlayer[1].x][dataPlayer[1].y] = '2';
                        }
                    }
                } else {
                    dataPlayer[0].freeze--;
                }
            }else
            {

                dataPlayer[0].alive = 1;
                dataPlayer[0].coin = 0;
                mapToGame[dataPlayer[0].x][dataPlayer[0].y] = 'D';
                int valueDrop = dataPlayer[0].coin;
                if (valueDrop >= 0) {
                    dropedCoinArray[amountDrop].value = valueDrop;
                    dropedCoinArray[amountDrop].x = dataPlayer[0].x;
                    dropedCoinArray[amountDrop].y = dataPlayer[0].y;
                    mapToGame[nowyx][nowyy] = 'D';
                    amountDrop++;
                }
                dataPlayer[0].x = dataPlayer[0].spawn_x;
                dataPlayer[0].y = dataPlayer[0].spawn_y;
                dataPlayer[0].death++;
                mapToGame[nowyx][nowyy] = '1';
            }
        }
        mapToGame[14][29] = 'A';

        /// obsluga gracza zdalnego jezeli gra
        sem_wait(&pdata_shared[0]->sem);
        if(pdata_shared[0]->hello == 1 )
        {
            /// inicjalizajca gracza
            strncpy(dataPlayer[1].name, "PLAYER1", 7);

            while (1) {
                int r = rand() % 29;
                int z = rand() % 55;
                if (mapToGame[r][z] == ' ') {
                    dataPlayer[1].spawn_x = r;
                    dataPlayer[1].spawn_y = z;
                    dataPlayer[1].x = r;
                    dataPlayer[1].y = z;
                    break;
                }
            }
            pdata_shared[0]->pidServer = pid;
            pdata_shared[0]->alive = 0;
            pdata_shared[0]->busy = 1;
            pdata_shared[0]->round = gameRound;
            dataPlayer[1].pid = pdata_shared[0]->pid;
            pdata_shared[0]->spawn_y = dataPlayer[1].spawn_y;
            pdata_shared[0]->spawn_x = dataPlayer[1].spawn_x;
            pdata_shared[0]->x = dataPlayer[1].spawn_x;
            pdata_shared[0]->y = dataPlayer[1].spawn_y;

            mapToGame[dataPlayer[1].x][dataPlayer[1].y] = '2';
            /// przekazywanie mapy
            for(int i  = dataPlayer[1].x -2,ii=0;ii<5;i++,ii++)
            {
                for(int j  = dataPlayer[1].y -2,jj=0;jj<5;j++,jj++)
                {
                    pdata_shared[0]->map[ii][jj] = mapToGame[i][j];
                }
            }
            /// flaga ze serwer juz wszystko ustawil
            pdata_shared[0]->hello = 2;
        }else if(pdata_shared[0]->hello == 2)
        {
            /// rozgrywka

            int commandoutsideplayer = pdata_shared[0]->move;
            nowyx = dataPlayer[1].x;
            nowyy = dataPlayer[1].y;
            if (commandoutsideplayer == 'a') {
                nowyy--;
            } else if (commandoutsideplayer == 's') {
                nowyx++;
            } else if (commandoutsideplayer == 'w') {
                nowyx--;
            } else if (commandoutsideplayer == 'd') {
                nowyy++;
            }
            if(commandoutsideplayer==0)
            {
                pdata_shared[0]->alive++;
                sem_post(&pdata_shared[0]->sem);

            }else
            {
                pdata_shared[0]->alive=0;
            }
            if(pdata_shared[0]->alive>50)
            {
                pdata_shared[0]->hello = 3;
            }
            int death = 0;
            if (dataPlayer[1].freeze == 0) {
                if (mapToGame[nowyx][nowyy] != 'X') {
                    if (map[dataPlayer[1].x][dataPlayer[1].y] == 'A') {
                        mapToGame[dataPlayer[1].x][dataPlayer[1].y] = 'A';
                    }
                    if (mapToGame[nowyx][nowyy] == 'c') {
                        mapToGame[dataPlayer[1].x][dataPlayer[1].y] = ' ';
                        dataPlayer[1].coin++;
                    } else if (mapToGame[nowyx][nowyy] == 'T') {
                        mapToGame[dataPlayer[1].x][dataPlayer[1].y] = ' ';
                        dataPlayer[1].coin += 10;
                    } else if (mapToGame[nowyx][nowyy] == 'C') {
                        mapToGame[dataPlayer[1].x][dataPlayer[1].y] = ' ';
                        dataPlayer[1].coin += 50;
                    } else if (mapToGame[nowyx][nowyy] == 'D') {
                        for(int i = 0;i<amountDrop;i++)
                        {
                            if(nowyx== dropedCoinArray[i].x  && nowyy== dropedCoinArray[i].y)
                            {
                                if(dropedCoinArray[i].value != -1)
                                {
                                    dataPlayer[1].coin += dropedCoinArray[i].value ;
                                    dropedCoinArray[i].value = -1;
                                }
                            }
                        }
                        mapToGame[dataPlayer[1].x][dataPlayer[1].y] = ' ';
                    } else if (mapToGame[nowyx][nowyy] == 'A') {
                        dataPlayer[1].budget += dataPlayer[1].coin;
                        dataPlayer[1].coin = 0;
                        mapToGame[dataPlayer[1].x][dataPlayer[1].y] = ' ';
                    } else if (mapToGame[nowyx][nowyy] == '#') {
                        mapToGame[dataPlayer[1].x][dataPlayer[1].y] = map[dataPlayer[1].x][dataPlayer[1].y];
                        dataPlayer[1].freeze = 4;
                    }
                    else if (mapToGame[nowyx][nowyy] == '1') {
                        /// drop z zebranych monet graczy
                        int valueDrop = dataPlayer[0].coin + dataPlayer[1].coin;
                        if(valueDrop>=0)
                        {
                            dropedCoinArray[amountDrop].value = valueDrop;
                            dropedCoinArray[amountDrop].x = nowyx;
                            dropedCoinArray[amountDrop].y = nowyy;
                            mapToGame[nowyx][nowyy] = 'D';
                            amountDrop++;
                        }
                        /// flaga do gracze ida na spawn
                        death = 1;
                    }else
                        mapToGame[dataPlayer[1].x][dataPlayer[1].y] = map[dataPlayer[1].x][dataPlayer[1].y];

                    if(death==0)
                    {
                        /// gracz poprostu sie przesuwa
                        dataPlayer[1].x = nowyx;
                        dataPlayer[1].y = nowyy;
                        mapToGame[nowyx][nowyy] = '2';
                    }
                    else
                    {
                        ///gracze ida na spawn
                        mapToGame[dataPlayer[0].x][dataPlayer[0].y] =  map[dataPlayer[0].x][dataPlayer[0].y];
                        mapToGame[dataPlayer[1].x][dataPlayer[1].y] =  map[dataPlayer[1].x][dataPlayer[1].y];
                        for(int i =0;i<=1;i++)
                        {
                            dataPlayer[i].coin = 0;
                            dataPlayer[i].x = dataPlayer[i].spawn_x;
                            dataPlayer[i].y = dataPlayer[i].spawn_y;
                            dataPlayer[i].death++;
                        }
                        mapToGame[nowyx][nowyy] = 'D';
                        mapToGame[dataPlayer[0].x][dataPlayer[0].y] = '1';
                        mapToGame[dataPlayer[1].x][dataPlayer[1].y] = '2';
                    }
                }
            } else {
                dataPlayer[1].freeze--;
            }
            for(int i  = dataPlayer[1].x -2,ii=0;ii<5;i++,ii++)
            {
                for(int j  = dataPlayer[1].y -2,jj=0;jj<5;j++,jj++)
                {
                    pdata_shared[0]->map[ii][jj] = mapToGame[i][j];
                }
            }
            pdata_shared[0]->coin = dataPlayer[1].coin;
            pdata_shared[0]->budget = dataPlayer[1].budget;
            pdata_shared[0]->death = dataPlayer[1].death;
            pdata_shared[0]->x = nowyx;
            pdata_shared[0]->y = nowyy;
            pdata_shared[0]->round = gameRound;
        }else if(pdata_shared[0]->hello == 3)
        {
            pdata_shared[0]->busy = 0;
            mapToGame[dataPlayer[1].x][dataPlayer[1].y] = map[dataPlayer[1].x][dataPlayer[1].y];
            strncpy(dataPlayer[1].name, "--------", 7);
            dataPlayer[1].pid =0;
            dataPlayer[1].coin =0;
            dataPlayer[1].death =0;
            dataPlayer[1].budget =0;
            dataPlayer[1].x =0;
            dataPlayer[1].y =0;
        }
        sem_post(&pdata_shared[0]->sem2);


        /// wysietlanie mapy oraz dany na serwerze oraz czas rundy
        displaymap(command);
        gameRound++;

        /// wewnetrzy mutex do besti i gry
        pthread_mutex_unlock(&mutex);
        usleep(600000);
    }
    EndGame = 1;


    /// zamykanie semaforow i sprzatanie po sobie -> pamiec wspoldzielona
    sem_close(&pdata_shared[0]->sem);
    sem_close(&pdata_shared[0]->sem2);
    munmap(pdata_shared[0], sizeof(struct dataPlayerToComunicate));
    close(pl[0]);
    endwin();
    return 0;
}
void initPlayers()
{
    srand(time(NULL));
    for(int i =0;i<=1;i++) {

        while (1) {
            int r = rand() % 29;
            int z = rand() % 55;
            if (mapToGame[r][z] == ' ') {
                dataPlayer[i].spawn_x = r;
                dataPlayer[i].spawn_y = z;
                break;
            }
        }
        dataPlayer[i].alive = 0;
        dataPlayer[i].pid = 0;
        dataPlayer[i].x = dataPlayer[0].spawn_x;
        dataPlayer[i].y = dataPlayer[0].spawn_y;
        dataPlayer[i].budget = 0;
        dataPlayer[i].coin = 0;
        dataPlayer[i].alive = 1;

    }
    dataPlayer[0].pid = getpid();
    mapToGame[dataPlayer[0].x][dataPlayer[0].y] = '1';
    strncpy(dataPlayer[0].name, "server", 6);
    strncpy(dataPlayer[1].name, "------", 6);
    dataPlayer[1].x =0;
    dataPlayer[1].y =0;
}
void setdisplay()
{
    screen = initscr();
    curs_set(FALSE);
    keypad(screen, TRUE);
    nodelay(screen, TRUE);
    noecho();
    start_color();
    init_pair(1,COLOR_BLACK,COLOR_BLACK);
    init_pair(2,COLOR_BLACK,COLOR_YELLOW);
    init_pair(3,COLOR_WHITE,COLOR_WHITE);
    init_pair(4,COLOR_WHITE,COLOR_GREEN);
    init_pair(5,COLOR_WHITE,COLOR_MAGENTA);
    init_pair(6,COLOR_RED,COLOR_GREEN);
    init_pair(7,COLOR_YELLOW,COLOR_GREEN);
    init_pair(8,COLOR_WHITE,COLOR_WHITE);
    init_pair(9,COLOR_BLACK,COLOR_WHITE);
    init_pair(10,COLOR_WHITE,COLOR_RED);
    wbkgd(stdscr,COLOR_PAIR(8));

}
void displaymap(int a)
{
    //wclear(screen);
    for(int j =0;j<29;j++)
    {
        for(int i = 0; i<55;i++)
        {
            if(mapToGame[j][i] == 'X')
            {
                attron(COLOR_PAIR(1));
                mvprintw(j,i,"%c",mapToGame[j][i]);
                attroff(COLOR_PAIR(1));
            }else if(mapToGame[j][i] == 'c' || mapToGame[j][i] == 'C' || mapToGame[j][i] == 'T' || mapToGame[j][i] == 'D')
            {
                attron(COLOR_PAIR(2));
                mvprintw(j,i,"%c",mapToGame[j][i]);
                attroff(COLOR_PAIR(2));
            }else if(mapToGame[j][i] == ' ')
            {
                attron(COLOR_PAIR(3));
                mvprintw(j,i,"%c",mapToGame[j][i]);
                attroff(COLOR_PAIR(3));
            }else if(mapToGame[j][i] == '#')
            {
                attron(COLOR_PAIR(4));
                mvprintw(j,i,"%c",mapToGame[j][i]);
                attroff(COLOR_PAIR(4));
            }
            else if(mapToGame[j][i] == '1' || mapToGame[j][i] == '2')
            {
                attron(COLOR_PAIR(5));
                mvprintw(j,i,"%c",mapToGame[j][i]);
                attroff(COLOR_PAIR(5));
            }
            else if(mapToGame[j][i] == 'A')
            {
                attron(COLOR_PAIR(6));
                mvprintw(j,i,"%c",mapToGame[j][i]);
                attroff(COLOR_PAIR(6));
            }
            else if(mapToGame[j][i] == '*')
            {
                attron(COLOR_PAIR(10));
                mvprintw(j,i,"%c",mapToGame[j][i]);
                attroff(COLOR_PAIR(10));
            }
        }
    }
        attron(COLOR_PAIR(9));

        mvprintw(1,57,"Server's PID: %d",getpid());
        mvprintw(2,57,"Campsite X/Y: 23/11");
        mvprintw(3,57,"Round number: %d",gameRound);
        mvprintw(6,57,"PID         %u          %u",dataPlayer[0].pid,dataPlayer[1].pid);
        mvprintw(7,57,"Type         %s       %s",dataPlayer[0].name,dataPlayer[1].name);
        mvprintw(8,57,"Position        %02d/%02d      %02d/%02d",dataPlayer[0].x,dataPlayer[0].y,dataPlayer[1].x,dataPlayer[1].y);
        mvprintw(9,57, "Budget            %02d          %02d",dataPlayer[0].budget,dataPlayer[1].budget);
        mvprintw(10,57,"Coins             %02d          %02d",dataPlayer[0].coin,dataPlayer[1].coin);
        mvprintw(11,57,"Death             %02d          %02d",dataPlayer[0].death,dataPlayer[1].death);
        attroff(COLOR_PAIR(9));

    mvprintw(16,57,"Legend:");
    attroff(COLOR_PAIR(9));


    attron(COLOR_PAIR(5));
    mvprintw(17,57,"1234");
    attroff(COLOR_PAIR(5));
    attron(COLOR_PAIR(9));
    mvprintw(17,64,"   - players");
    attroff(COLOR_PAIR(9));

    attron(COLOR_PAIR(1));
    mvprintw(18,57,"X");
    attroff(COLOR_PAIR(1));
    attron(COLOR_PAIR(9));
    mvprintw(18,64,"   - Wall");
    attroff(COLOR_PAIR(9));


    attron(COLOR_PAIR(4));
    mvprintw(19,57,"#");
    attroff(COLOR_PAIR(4));
    attron(COLOR_PAIR(9));
    mvprintw(19,64,"    - bushes (slow down)");
    attroff(COLOR_PAIR(9));

    attron(COLOR_PAIR(10));
    mvprintw(20,57,"*");
    attroff(COLOR_PAIR(10));
    attron(COLOR_PAIR(9));
    mvprintw(20,64,"    - enemy");
    attroff(COLOR_PAIR(9));

    attron(COLOR_PAIR(2));
    mvprintw(21,57,"c");
    attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(9));
    mvprintw(21,64,"    - one coin");
    attroff(COLOR_PAIR(9));

    attron(COLOR_PAIR(2));
    mvprintw(22,57,"C");
    attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(9));
    mvprintw(22,64,"    - treasure (10 coins)");
    attroff(COLOR_PAIR(9));


    attron(COLOR_PAIR(2));
    mvprintw(22,100,"D");
    attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(9));
    mvprintw(22,102,"  – dropped treasure ");
    attroff(COLOR_PAIR(9));


    attron(COLOR_PAIR(2));
    mvprintw(23,57,"T");
    attroff(COLOR_PAIR(2));
    attron(COLOR_PAIR(9));
    mvprintw(23,64,"    - large treasure (50 coins)");
    attroff(COLOR_PAIR(9));

    attron(COLOR_PAIR(6));
    mvprintw(24,57,"A");
    attroff(COLOR_PAIR(6));
    attron(COLOR_PAIR(9));
    mvprintw(24,64,"    - campsite");
    attroff(COLOR_PAIR(9));

    refresh();
}
int findNearbyPlayer(int x,int y )
{
    int lenght[2];
    for(int i =0;i<=1;i++) {
        if(dataPlayer[i].x == 0  || dataPlayer[i].y == 0)
            lenght[i] = -1;
        else
        lenght[i] = abs(dataPlayer[i].x-x) + abs(dataPlayer[i].y - y);
    }
    if(lenght[1]<0 && lenght[0]<10)
        return 1;
    if(lenght[1]<10 && lenght[1]>0)
        return 2;
    if(lenght[1]<10 && lenght[1]>0 && lenght[0]<10) {
        if (lenght[0] < lenght[1]) {
            return 1;
        } else {
            return 2;
        }
    }
    return 0;
}
void* beast(void * arg)
{
    srand(time(NULL));
    int x=10,y=36;
    mapToGame[x][y] = '*';
    char valueFromMap=' ';

    while(EndGame==0)
    {
        int playerIndex = findNearbyPlayer(x,y);
        int ruch =0;

        pthread_mutex_lock(&mutex);
        /// czy gracze sa w zasiegu

        attron(COLOR_PAIR(9));
        mvprintw(30,27,"%d",playerIndex);
        attroff(COLOR_PAIR(9));
        refresh();
        if(playerIndex!=0) {
            playerIndex -=1 ;
            if (dataPlayer[playerIndex].x != x) {
                if ((dataPlayer[playerIndex].x - x) <= 0){
                        ruch = 2;

                        if (mapToGame[x + 1][y] == 'X') {
                            if (dataPlayer[playerIndex].y - y > 0) {
                                ruch = 3;
                            } else {
                                ruch = 4;
                            }
                        }
                    }else
                    {
                        ruch = 1;
                    }
                }else
                {
                    if (dataPlayer[playerIndex].y - y > 0) {
                        ruch = 3;
                    } else {
                        ruch = 4;
                    }
                }
            }

        char isPlayer= ' ';
        switch (ruch) {
            case 1:
                if (mapToGame[x + 1][y] != 'X') {
                    valueFromMap = mapToGame[x][y];
                    mapToGame[x][y] = map[x][y];
                    x++;
                    isPlayer = mapToGame[x][y];
                    mapToGame[x][y] = '*';
                }
            case 2:
                if (mapToGame[x - 1][y] != 'X') {
                    valueFromMap = mapToGame[x][y];
                    mapToGame[x][y] = map[x][y];
                    x--;
                    isPlayer = mapToGame[x][y];
                    mapToGame[x][y] = '*';
                }
            case 3:
                    y+=1;
                    isPlayer = mapToGame[x][y];
                    mapToGame[x][y] = '*';
            case 4:
                if (mapToGame[x][y - 1] != 'X') {
                    valueFromMap = mapToGame[x][y];
                    mapToGame[x][y] = map[x][y];
                    y--;
                    isPlayer = mapToGame[x][y];
                    mapToGame[x][y] = '*';
                }
            default:
                mapToGame[x][y] = '*';
        }
        if(isPlayer<='2' && isPlayer>='0')
        {
            int index = 0;
            if(isPlayer == '2')
            {
                index = 1;
            }
            dataPlayer[index].alive = 0;
        }

        displaymap(valueFromMap);
        pthread_mutex_unlock(&mutex);

        usleep(800000);
    }
    return NULL;
}

