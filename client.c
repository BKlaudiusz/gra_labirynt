


#include "coomon.h"

void displaymap(struct dataPlayerToComunicate* pdata,WINDOW *screen);
WINDOW* initGameScreen();
int main() {
    const char nameSharedMemory[14] ="/player1_data";
    pid_t mypid = getpid();
    int pl = shm_open(nameSharedMemory, O_RDWR, 0666);
    if(pl == -1)
    {
        printf("Server don't run !\n");
        return 0;
    }
    struct dataPlayerToComunicate* pdata = (struct dataPlayerToComunicate*)mmap(NULL, sizeof(struct dataPlayerToComunicate), PROT_READ | PROT_WRITE, MAP_SHARED, pl, 0);


    int command = 0;
    /// nawiozanie kontaktu z serwerem
    sem_wait(&pdata->sem2);
        if(pdata->busy == 1)
        {
            command = -1;
        }else
        {
            pdata->hello = 1;
            pdata->pid = mypid;
        }
    sem_post(&pdata->sem);
    if(command == -1)
    {
        printf("Server is full!\n");
        munmap(pdata, sizeof(struct dataPlayerToComunicate));
        close(pl);
        return 0;
    }
    WINDOW *screen = initGameScreen();

    while (command != 'q')
    {
        sem_wait(&pdata->sem2);
        if(pdata->hello == 2)
        {
            /// serwer ustawil gracza
            displaymap(pdata,screen);
            if(command<=0)
            {
                pdata->move =0;
            }else
            {
                pdata->move = (int)command;
            }
        }
        sem_post(&pdata->sem);

        usleep(600000);
        command = wgetch(screen);
    }
/// zakonczenie kontaktu z serwerem
    sem_wait(&pdata->sem2);
    pdata->hello = 3;
    sem_post(&pdata->sem);

    /// odlaczenie sie od pamieci wspoldzielonej, czyszczenie zasobow
    printf("Goodbye!\n");
    munmap(pdata, sizeof(struct dataPlayerToComunicate));
    close(pl);
    endwin();
    return 0;
}



WINDOW* initGameScreen() {
    WINDOW *screen = initscr();
    curs_set(FALSE);
    keypad(screen, TRUE);
    nodelay(screen, TRUE);
    noecho();
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);
    init_pair(3, COLOR_WHITE, COLOR_WHITE);
    init_pair(4, COLOR_WHITE, COLOR_GREEN);
    init_pair(5, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(6, COLOR_RED, COLOR_GREEN);
    init_pair(7, COLOR_YELLOW, COLOR_GREEN);
    init_pair(8, COLOR_WHITE, COLOR_WHITE);
    init_pair(9, COLOR_BLACK, COLOR_WHITE);
    wbkgd(stdscr, COLOR_PAIR(8));
    return screen;
}
void displaymap(struct dataPlayerToComunicate* pdata,WINDOW *screen)
{
    wclear(screen);
    /// pobranie najnowyszch informacji od serwera
    for(int j  = pdata->x -2,jj=0;jj<5;j++,jj++)
    {
        for(int i  = pdata->y -2,ii=0;ii<5;i++,ii++)
        {
            if(pdata->map[jj][ii] == 'X')
            {
                attron(COLOR_PAIR(1));
                mvprintw(j,i,"%c",pdata->map[jj][ii]);
                attroff(COLOR_PAIR(1));
            }else if(pdata->map[jj][ii] == 'c' || pdata->map[jj][ii] == 'C' || pdata->map[jj][ii] == 'T' || pdata->map[jj][ii] == 'D')
            {
                attron(COLOR_PAIR(2));
                mvprintw(j,i,"%c",pdata->map[jj][ii]);
                attroff(COLOR_PAIR(2));
            }else if(pdata->map[jj][ii] == ' ')
            {
                attron(COLOR_PAIR(3));
                mvprintw(j,i,"%c",pdata->map[jj][ii]);
                attroff(COLOR_PAIR(3));
            }else if(pdata->map[jj][ii] == '#')
            {
                attron(COLOR_PAIR(4));
                mvprintw(j,i,"%c",pdata->map[jj][ii]);
                attroff(COLOR_PAIR(4));
            }
            else if(pdata->map[jj][ii] == '1' || pdata->map[jj][ii] == '2')
            {
                attron(COLOR_PAIR(5));
                mvprintw(j,i,"%c",pdata->map[jj][ii]);
                attroff(COLOR_PAIR(5));
            }
            else if(pdata->map[jj][ii] == 'A')
            {
                attron(COLOR_PAIR(6));
                mvprintw(j,i,"%c",pdata->map[jj][ii]);
                attroff(COLOR_PAIR(6));
            }
            //mvprintw(j,i,"%c",pdata->map[jj][ii]);
        }
    }

    attron(COLOR_PAIR(9));
    mvprintw(1,57," Server's PID: %d",pdata->pidServer);
    mvprintw(2,57,"Campsite X/Y: unknown");

    mvprintw(3,57,"Round number: %d",pdata->round);
    mvprintw(6,57,"Player:");
    mvprintw(7,57,"Number:      2");
    mvprintw(8,57,"Type:        HUMAN");
    mvprintw(9,57,"Position     %02d/%02d",pdata->x,pdata->y);
    mvprintw(10,57,"Deaths       %d",pdata->death);
    mvprintw(12,57,"Coins found:%d",pdata->coin);
    mvprintw(13,57,"Coins brought:%d",pdata->budget);

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

    attron(COLOR_PAIR(4));
    mvprintw(20,57,"*");
    attroff(COLOR_PAIR(4));
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