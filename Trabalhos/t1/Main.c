// programa para testar o terminal em modo "cru"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
typedef struct timespec crono;

// implementação de um cronômetro
typedef struct timespec crono;

// inicializa um cronômetro com a hora atual
// void crono_inicia(crono *c)
// {
//     clock_gettime(CLOCK_MONOTONIC, c);
// }

// // retorna o tempo passado desde que o cronômetro *c foi iniciado, em segundos
// double crono_parcial(crono *c)
// {
//     crono agora;
//     clock_gettime(CLOCK_MONOTONIC, &agora);

//     double segundos = agora.tv_sec - c->tv_sec;
//     double nanosegundos = agora.tv_nsec - c->tv_nsec;
//     return segundos + 1e-9 * nanosegundos;
// }

int f(int x)
{
    if (x <= 0) return 0;
    return f(x - 1);
}
// configura o terminal para o modo "cru", para permitir a leitura
//   de cada caractere digitado sem esperar pelo "enter".
void configura_terminal()
{
    if (system("stty raw opost -echo min 0 time 1") != 0) {
        perror("erro na execução de system(\"stty\")");
        fprintf(stderr, "você tem o programa stty instalado?\n");
        exit(1);
    };
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0) {
        perror("erro na execução de setvbuf()");
        exit(1);
    }
}

// configura o terminal para o modo normal, com bufferização por linha.
void normaliza_terminal()
{
    system("stty sane");
}

// lê um caractere do teclado.
// retorna o código do caractere lido ou 0 casa nada tenha sido digitado.
// só funciona corretamente se o terminal estiver em modo "cru".
char lechar()
{
    fflush(stdout);
    char c;
    if (fread(&c, 1, 1, stdin) == 1) return c;
    return 0;
}

// int main()
// {
//     configura_terminal();
//     for (;;) {
//         int c = lechar();
//         printf("%d\n", c);
//         if (c == 'q') break;
//     }
//     normaliza_terminal();
// }
int main() {
    configura_terminal();
    char u = 0;
    int n = 0;
    do {
        char c = lechar();
        if (c != 0) {
            u = c;
            n = 0;
        }
        n++;
        printf("leituras: %d  ult ch: %d   \r", n, c);
    } while (u != 'q');
    normaliza_terminal();
        crono c1;
    //crono_inicia(&c);
    f(1000);
    //double t1 = crono_parcial(&c);
    f(2000);
    // double t2 = crono_parcial(&c);
    // printf("f(1000) demorou %f segundos.\n", t1);
    // printf("f(2000) demorou %f segundos.\n", t2 - t1);
    system("aplay -q x.3.wav 2.3.wav");
    return 0;
}