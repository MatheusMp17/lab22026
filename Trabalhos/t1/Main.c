// programa para testar o terminal em modo "cru"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
typedef struct timespec crono;
typedef enum { JOGO_MENU, JOGO_DIURNO, JOGO_NOTURNO, JOGO_TUTORIAL, JOGO_VITORIA, JOGO_DERROTA } EstadoJogo;

typedef struct {
    EstadoJogo estado_atual;
    int pontos; 
    int qtdTiros;
    int qtdInimigos;
    int inimigos_na_tela;
    char nome_jogador[3];   
    int escudos;
    int arma;
    time_t tempo_inicio;
    int tempo_de_jogo;
    int fases_passadas;
    int partidas_jogadas;
    bool existe_historico;
    int recorde;
} ControleJogo;

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
int main() {
  
  return 0;
}
void inicializa_estado(controleJogo *controle, char nome[]){
	controle.pontos = 0;
	controle.escudos = 3;
	controle.estado_atual = ESTADO_MENU;
	controle.qtdTiros = 30;
	controle.qtdInimigos = 20;
	controle.nome_jogador = nome;
	controle.tempo_inicio = 0;
	controle.tempo_de_jogo = 0;
	controle.fases_passadas = 0;
	controle.partidas_jogadas = 0;
	controle.existe_historico = false;
	controle.recorde = 0;

}
