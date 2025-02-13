#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>


#define check_error(cond, msg)\
do{\
    if(!(cond)){\
        perror(msg);\
        fprintf(stderr, "%s: %d\n", __FILE__, __LINE__);\
        exit(EXIT_FAILURE);\
        }\
}while(0)


    char mapa[5][5];
    char protivnickaMapa[5][5];

void unesiBrodove(){
    printf("Unesite pozicije 5 brodova: \n");
    int k, l;
    for(int i = 0; i < 5; i++){
        scanf("%d %d", &k, &l);
        mapa[k][l] = '1';
    }
}

void inicijalizujMapu(){
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            mapa[i][j] = '0';
        }
    }
}

void inicijalizujProtivnickuMapu(){
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            protivnickaMapa[i][j] = '*';
        }
    }
}

void stampajSvojuMapu(){
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            printf("%c ", mapa[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void stampajProtivnickuMapu(){
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
            printf("%c ", protivnickaMapa[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main() {
    // Prvi igrac pravi fifo fajlove
    check_error(mkfifo("fifo1", 0600) != -1 || errno == EEXIST, "mkfifo fifo1 failed");
    check_error(mkfifo("fifo2", 0600) != -1 || errno == EEXIST, "mkfifo fifo2 failed");
    int write_fd = open("fifo1", O_WRONLY);
    check_error(write_fd != -1, "open fifo1 failed");
    int read_fd = open("fifo2", O_RDONLY);
    check_error(read_fd != -1, "open fifo2 failed");

    // Inicijalizujemo mape
    inicijalizujMapu();
    unesiBrodove();
    stampajSvojuMapu();
    inicijalizujProtivnickuMapu();
    stampajProtivnickuMapu();

    int brojPreostalih = 5;
    char potez[5], odgovor[5];
    char pogodak = 'n'; // Oznacava da li smo pogodjeni ili ne u prethodnom potezu / U prvom koraku n jer nema smisla da bude 0 ili 1
    int a, b, x, y;

    while (brojPreostalih > 0) {
        // Unos poteza
        printf("Unesite potez (format: x y):\n");
        fflush(stdin);  // Da ne ostane nista na standardnom ulazu pre fgets(pokusao sam ovime da resim problem)
        //fgets(potez, sizeof(potez), stdin);
        scanf("%d %d", &a, &b);
        sprintf(potez, "%d %d%c", a, b, pogodak); 

        write(write_fd, potez, strlen(potez) + 1);

        // Citamo protivnikov odgovor
        read(read_fd, odgovor, sizeof(odgovor));
        printf("Protivnik kaze: %s\n", odgovor);

        if (odgovor[3] == 'K') { // Protivnik izgubio
            printf("Cestitamo, pobedili ste!\n");
            break;
        }

        //Osvezavamo nasu mapu
        x = odgovor[0] - '0';
        y = odgovor[2] - '0';
        if (mapa[x][y] == '1') {
            mapa[x][y] = '!';
            pogodak = '1';
            brojPreostalih--;
        } else {
            mapa[x][y] = 'X';
            pogodak = '0';
        }

        // Osvezavamo protivnicku mapu
        if(odgovor[3] == '0'){
            protivnickaMapa[a][b] = '0';
        } else if(odgovor[3] == '1'){
            protivnickaMapa[a][b] = '1';
        }

        stampajSvojuMapu();
        stampajProtivnickuMapu();

        if (brojPreostalih == 0) {
            snprintf(odgovor, sizeof(odgovor), "%d %dK", x, y);
            write(write_fd, odgovor, strlen(odgovor) + 1);
            printf("Izgubili ste!\n");
            break;
        }
    }

    close(write_fd);
    close(read_fd);
    unlink("fifo1");
    unlink("fifo2");

    return 0;
}