#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net.h"
#include "game.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Использование: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    if (net_init() != 0) {
        fprintf(stderr, "Ошибка инициализации сокетов.\n");
        return 1;
    }

    socket_t sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == SOCK_INVALID) {
        fprintf(stderr, "Не удалось создать сокет: %d\n", sock_last_error());
        net_cleanup();
        return 1;
    }

    struct sockaddr_in serv_addr, from_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons((unsigned short)atoi(argv[2]));

    struct hostent *hp = gethostbyname(argv[1]);
    if (hp == NULL) {
        fprintf(stderr, "Не удалось разрешить хост: %s\n", argv[1]);
        SOCK_CLOSE(sock);
        net_cleanup();
        return 1;
    }
    memcpy(&serv_addr.sin_addr, hp->h_addr, (size_t)hp->h_length);

    printf("=== Крестики-нолики: КЛИЕНТ (X) ===\n");
    printf("Сервер: %s:%s\n\n", argv[1], argv[2]);

    char field[FIELD_SIZE];
    char buf[MSG_SIZE];
    field_clear(field);

    int wins = 0, losses = 0;
    socklen_t from_len = sizeof(from_addr);

    for (;;) {
        /* --- Наш ход --- */
        field_print(field);
        int move = read_move(field);
        if (move < 0) break;
        field[move - 1] = FIG_X;
        field_print(field);

        if (field_has_winner(field, FIG_X)) {
            wins++;
            printf("Вы победили! Счёт: %d побед, %d поражений\n", wins, losses);
            msg_pack(buf, STATUS_WIN, field);
            if (sendto(sock, buf, sizeof(buf), 0,
                       (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCK_ERR) {
                fprintf(stderr, "sendto: %d\n", sock_last_error());
                break;
            }
            field_clear(field);
            continue;              /* новый раунд — наш ход */
        }
        if (field_is_full(field)) {
            printf("Ничья. Счёт: %d побед, %d поражений\n", wins, losses);
            msg_pack(buf, STATUS_DRAW, field);
            sendto(sock, buf, sizeof(buf), 0,
                   (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            field_clear(field);
            continue;
        }

        msg_pack(buf, STATUS_MOVE, field);
        if (sendto(sock, buf, sizeof(buf), 0,
                   (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCK_ERR) {
            fprintf(stderr, "sendto: %d\n", sock_last_error());
            break;
        }

        /* --- Ждём ответ сервера --- */
        int bytes = recvfrom(sock, buf, sizeof(buf), 0,
                             (struct sockaddr *)&from_addr, &from_len);
        if (bytes <= 0) {
            fprintf(stderr, "recvfrom: %d\n", sock_last_error());
            break;
        }

        uint8_t status = msg_status(buf);
        msg_field(buf, field);
        field_print(field);

        if (status == STATUS_WIN) {
            losses++;
            printf("Вы проиграли! Счёт: %d побед, %d поражений\n", wins, losses);
            field_clear(field);
        } else if (status == STATUS_DRAW) {
            printf("Ничья. Счёт: %d побед, %d поражений\n", wins, losses);
            field_clear(field);
        }
        /* STATUS_MOVE — просто продолжаем цикл */
    }

    SOCK_CLOSE(sock);
    net_cleanup();
    return 0;
}