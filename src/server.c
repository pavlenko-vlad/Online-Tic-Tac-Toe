#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net.h"
#include "game.h"

#define DEFAULT_PORT 45678

int main(int argc, char **argv) {
    int port = (argc >= 2) ? atoi(argv[1]) : DEFAULT_PORT;

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

    struct sockaddr_in serv_addr, client_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons((unsigned short)port);

    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
        fprintf(stderr, "bind не удался: %d\n", sock_last_error());
        SOCK_CLOSE(sock);
        net_cleanup();
        return 1;
    }

    printf("=== Крестики-нолики: СЕРВЕР (O) ===\n");
    printf("Слушаем порт %d (UDP)\n\n", port);

    char field[FIELD_SIZE];
    char buf[MSG_SIZE];
    field_clear(field);

    int wins = 0, losses = 0;
    socklen_t client_len = sizeof(client_addr);

    for (;;) {
        int bytes = recvfrom(sock, buf, sizeof(buf), 0,
                             (struct sockaddr *)&client_addr, &client_len);
        if (bytes <= 0) {
            fprintf(stderr, "recvfrom: %d\n", sock_last_error());
            break;
        }

        uint8_t status = msg_status(buf);
        msg_field(buf, field);
        field_print(field);

        /* Клиент завершил раунд */
        if (status == STATUS_WIN) {
            losses++;
            printf("Вы проиграли! Счёт: %d побед, %d поражений\n", wins, losses);
            field_clear(field);
            continue;
        }
        if (status == STATUS_DRAW) {
            printf("Ничья. Счёт: %d побед, %d поражений\n", wins, losses);
            field_clear(field);
            continue;
        }

        /* Обычный ход клиента — теперь наш ход */
        int move = read_move(field);
        if (move < 0) break;
        field[move - 1] = FIG_O;
        field_print(field);

        if (field_has_winner(field, FIG_O)) {
            wins++;
            printf("Вы победили! Счёт: %d побед, %d поражений\n", wins, losses);
            msg_pack(buf, STATUS_WIN, field);
            field_clear(field);
        } else if (field_is_full(field)) {
            printf("Ничья. Счёт: %d побед, %d поражений\n", wins, losses);
            msg_pack(buf, STATUS_DRAW, field);
            field_clear(field);
        } else {
            msg_pack(buf, STATUS_MOVE, field);
        }

        if (sendto(sock, buf, sizeof(buf), 0,
                   (struct sockaddr *)&client_addr, client_len) == SOCK_ERR) {
            fprintf(stderr, "sendto: %d\n", sock_last_error());
            break;
        }
    }

    SOCK_CLOSE(sock);
    net_cleanup();
    return 0;
}