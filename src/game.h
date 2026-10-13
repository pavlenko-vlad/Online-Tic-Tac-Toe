#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define FIELD_SIZE 9
#define EMPTY      '*'
#define FIG_X      'X'
#define FIG_O      'O'

#define MSG_SIZE      (1 + FIELD_SIZE)
#define STATUS_MOVE   0   
#define STATUS_WIN    1   
#define STATUS_DRAW   2   

static inline void field_clear(char *field) {
    memset(field, EMPTY, FIELD_SIZE);
}

static inline void field_print(const char *field) {
    printf("\n");
    printf(" %c | %c | %c \n", field[0], field[1], field[2]);
    printf("---+---+---\n");
    printf(" %c | %c | %c \n", field[3], field[4], field[5]);
    printf("---+---+---\n");
    printf(" %c | %c | %c \n\n", field[6], field[7], field[8]);
}

static inline int field_has_winner(const char *field, char figure) {
    for (int r = 0; r < 3; ++r)
        if (field[r*3] == figure && field[r*3+1] == figure && field[r*3+2] == figure)
            return 1;
    for (int c = 0; c < 3; ++c)
        if (field[c] == figure && field[c+3] == figure && field[c+6] == figure)
            return 1;
    if (field[0] == figure && field[4] == figure && field[8] == figure) return 1;
    if (field[2] == figure && field[4] == figure && field[6] == figure) return 1;
    return 0;
}

static inline int field_is_full(const char *field) {
    for (int i = 0; i < FIELD_SIZE; ++i)
        if (field[i] == EMPTY) return 0;
    return 1;
}

static inline void msg_pack(char *buf, uint8_t status, const char *field) {
    buf[0] = (char)status;
    memcpy(buf + 1, field, FIELD_SIZE);
}

static inline uint8_t msg_status(const char *buf) {
    return (uint8_t)buf[0];
}

static inline void msg_field(const char *buf, char *field_out) {
    memcpy(field_out, buf + 1, FIELD_SIZE);
}

static inline int read_move(const char *field) {
    int move = 0;
    for (;;) {
        printf("Ваш ход (1-9): ");
        fflush(stdout);

        if (scanf("%d", &move) != 1) {
            if (feof(stdin)) {
                printf("\nЗавершение.\n");
                return -1;
            }
            while (getchar() != '\n');
            printf("Некорректный ввод.\n");
            continue;
        }
        if (move < 1 || move > 9) {
            printf("Число должно быть от 1 до 9.\n");
            continue;
        }
        if (field[move - 1] != EMPTY) {
            printf("Клетка %d уже занята.\n", move);
            continue;
        }
        return move;
    }
}

#endif