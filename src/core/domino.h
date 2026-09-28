#ifndef DOMINO_H
#define DOMINO_H

#include <stdbool.h>
#include <stdint.h>

#define TILE_COUNT 28
#define HAND_SIZE 7
#define WIN_SCORE 6

typedef struct {
    uint8_t a, b;
} Tile;

/* Ordem do enum = ordem da vez (anti-horário): Sul -> Leste -> Norte -> Oeste. */
typedef enum { SEAT_SOUTH, SEAT_EAST, SEAT_NORTH, SEAT_WEST, SEAT_COUNT } Seat;

typedef enum { TEAM_US, TEAM_THEM } Team; /* Sul+Norte, Leste+Oeste */

typedef enum { END_LEFT, END_RIGHT } End;

typedef enum { PHASE_PLAYING, PHASE_HAND_OVER, PHASE_MATCH_OVER } Phase;

typedef enum {
    RESULT_NONE,
    RESULT_SIMPLES,
    RESULT_CARROCA,
    RESULT_LA_E_LO,
    RESULT_CRUZADA,
    RESULT_TRANCADO
} HandResult;

/* Peça na linha, com os valores na ordem da linha (esquerda -> direita). */
typedef struct {
    uint8_t left, right;
} PlacedTile;

typedef struct {
    PlacedTile line[TILE_COUNT];
    int count;
    int anchor; /* índice em line[] da primeira peça jogada na mão */
} Board;

typedef struct {
    Tile tiles[HAND_SIZE];
    int count;
} Hand;

typedef struct {
    Hand hands[SEAT_COUNT];
    Board board;
    Seat turn;
    Seat opener;       /* quem abriu a mão atual */
    Seat next_opener;  /* onde começa a procura por carroça para abrir a próxima mão */
    bool first_hand;   /* primeira mão da partida: abre com [6|6] */
    int passes;        /* passes seguidos */
    int score[2];
    Phase phase;
    HandResult result;
    Seat result_seat;  /* quem bateu */
    int result_points;
    int result_team;   /* -1 quando ninguém pontua */
    uint64_t rng;
    bool lacks[SEAT_COUNT][7]; /* valores em que cada assento já passou nesta mão */
} Game;

bool tile_is_double(Tile t);
int tile_sum(Tile t);
Team team_of(Seat s);
Seat next_seat(Seat s);

/* Zera o placar e começa a primeira mão (regra do [6|6]). */
void game_new_match(Game *g, uint64_t seed);
/* Mesma coisa, mas continua a sequência do RNG atual. */
void game_restart_match(Game *g);
/* Distribui e começa a próxima mão (depois de PHASE_HAND_OVER). */
void game_next_hand(Game *g);
/* Quem abre uma mão que não é a primeira: o primeiro com carroça na ordem
   lead, parceiro do lead, seguinte ao lead, parceiro do seguinte. */
Seat game_choose_opener(const Game *g, Seat lead);

/* Pontas em que a peça encaixa agora. Mesa vazia: só END_LEFT, e só [6|6] na
   primeira mão ou carroça nas outras. */
void game_fits(const Game *g, Tile t, bool *left, bool *right);
bool game_has_move(const Game *g, Seat s);
/* Joga hands[turn].tiles[index] na ponta pedida. Retorna false se não for válida. */
bool game_play(Game *g, int index, End end);
/* Passa a vez do jogador atual. */
void game_pass(Game *g);

/* O que uma CPU pode ver: a própria mão, as pontas, as quantidades e os passes. */
typedef struct {
    Hand own;
    Seat seat;
    int counts[SEAT_COUNT];
    bool lacks[SEAT_COUNT][7];
    bool empty;
    uint8_t left, right;
    bool first_hand;
} CpuView;

CpuView cpu_view_of(const Game *g, Seat s);
/* Decide só pelo CpuView. Retorna false se não há jogada. */
bool cpu_decide(const CpuView *v, int *index, End *end);
/* CPU: escolhe a jogada do assento da vez (monta o CpuView e chama cpu_decide). */
bool cpu_choose(const Game *g, int *index, End *end);

#endif
