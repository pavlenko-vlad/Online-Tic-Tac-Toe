#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BUFLEN 81

#define h_addr h_addr_list[0] 

void print_field(char* field) {
	int i = 0;
	printf("\n");
	for(; i < 2; i++) {
		printf(" %c |", field[i]);	
	}
	printf(" %c \n", field[i]);
	i++;

	printf("===========\n");
	for(; i < 5; i++) {
		printf(" %c |", field[i]);	
	}
	printf(" %c \n", field[i]);
	i++;

	printf("===========\n");
	for(; i < 8; i++) {
		printf(" %c |", field[i]);	
	}
	printf(" %c \n", field[i]);
}

char* move_processing(char* field, char player_figure, int move) {
	if (field[move-1] == '*')
		field[move-1] = player_figure;
	return field;
}

int win_rule(char* field, int WIN, char player_figure) {
	int rows = 0, cols = 0;
	for (int i = 0 ; i < 3; rows += 3) {
		if ((field[0 + rows] == field[1 + rows]) && (field[0 + rows] == field[2 + rows]) && (field[0 + rows] == player_figure)) {
			return 1;
		}
		i++;
	}

	for (int i = 0 ; i < 3; cols++) {
		if ((field[0 + cols] == field[3 + cols]) && (field[0 + cols] == field[6 + cols]) && (field[0 + cols] == player_figure)) {
			return 1;
		}
		i++;
	}

	if ((field[0] == field[4]) && (field[0] == field[8]) && (field[0] == player_figure)) {
		return 1;
	}
	if ((field[2] == field[4]) && (field[2] == field[6]) && (field[2] == player_figure)) {
		return 1;
	}
	return 0;
}

int lost_rule(char* field, int WIN, char enemy_figure) {
	int rows = 0, cols = 0;
	for (int i = 0 ; i < 3; rows += 3) {
		if ((field[0 + rows] == field[1 + rows]) && (field[0 + rows] == field[2 + rows]) && (field[0 + rows] == enemy_figure)) {
			return -1;
		}
		i++;
	}

	for (int i = 0 ; i < 3; cols++) {
		if ((field[0 + cols] == field[3 + cols]) && (field[0 + cols] == field[6 + cols]) && (field[0 + cols] == enemy_figure)) {
			return -1;
		}
		i++;
	}

	if ((field[0] == field[4]) && (field[0] == field[8]) && (field[0] == enemy_figure)) {
		return -1;
	}
	if ((field[2] == field[4]) && (field[2] == field[6]) && (field[2] == enemy_figure)) {
		return -1;
	}
	return 0;
}

int draw_rule(char *field, int DRAW_FLAG) {
	int close_field_counter = 0;
	for (int i = 0; i < 9; i++) {
		if (field[i] != '*')
			close_field_counter++;
	}
	if (close_field_counter == 9)
		return 1;
	return 0;
}

void clearing_field(char* field) {
	for (int i = 0; i < 9; i++) {
		field[i] = '*';
	}
}

int main(int argc, char **argv) {
	int sock;
	struct sockaddr_in servAddr, clientAddr ;
	struct hostent *hp, *gethostbyname();
    char field[9];
	clearing_field(field);
	char move_buf[BUFLEN];
    int move = -1;
    char player_figure = 'X';
	char enemy_figure = 'O';
	int WIN_FLAG = 0, DRAW_FLAG = 0, table_Win = 0, table_Lose = 0;


	if( (sock = socket( AF_INET, SOCK_DGRAM, 0)) < 0)
	{ 
		perror("He могу получить socket\n");
		exit(1);
	}

	bzero((char*)&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	hp = gethostbyname(argv [1]);
	if (hp == NULL) {
		fprintf(stderr, "Не удалось разрешить имя хоста %s\n", argv[1]);
		exit(1);
	}

	bcopy(hp -> h_addr, (char*)&servAddr.sin_addr, hp -> h_length);
	servAddr.sin_port = htons(atoi(argv[2]));

	bzero((char*)&clientAddr, sizeof(clientAddr));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clientAddr.sin_port = 0;

	if ( bind(sock, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) )
	{
		perror("Клиент не получил порт.");
		exit(1);
	}

	while (1) {
		print_field(field);
		printf("Выберите поле (1-9): \n");
		while (field[move-1] != '*') {       // проверка корректности хода
        	scanf("%s", move_buf);
			for (int i = 0; i < strlen(move_buf); i++) {
				move *= 10;
				move = move_buf[i] - '0';
			}
		}
		move_processing(field, player_figure, move);
		print_field(field);

		if( sendto (sock, field, strlen(field), 0 , (struct sockaddr*)&servAddr , sizeof(servAddr)) < 0 ) {
			perror( "Проблемы с sendto.\n" );
			exit(1);
		}

		if ((WIN_FLAG = win_rule(field, WIN_FLAG, player_figure)) == 1) {
			move = 0;
			table_Win++;
			printf("\nYou WIN!\n");
			printf("Wins: %d Defeats: %d\n", table_Win, table_Lose);
			clearing_field(field);
			print_field(field);
		}
		if ((DRAW_FLAG = draw_rule(field, DRAW_FLAG)) == 1) {
			move = 0;
			DRAW_FLAG = 0;
			printf("\nDRAW!\n");
			printf("Wins: %d Defeats: %d\n", table_Win, table_Lose);
			clearing_field(field);
			print_field(field);
		}

		printf("Ход оппонента...\n");
		if ( recv(sock, field, strlen(field), 0) < 0 ) {
			perror("Плохой socket клиента.");
			exit(1);
		}
		
		if ((WIN_FLAG = lost_rule(field, WIN_FLAG, enemy_figure)) == -1) {
			move = 0;
			table_Lose++;
			print_field(field);
			printf("\nYou LOST!\n");
			printf("Wins: %d Defeats: %d\n", table_Win, table_Lose);
			clearing_field(field);
		}
		if ((DRAW_FLAG = draw_rule(field, DRAW_FLAG)) == 1) {
			move = 0;
			DRAW_FLAG = 0;
			print_field(field);
			printf("\nDRAW!\n");
			printf("Wins: %d Defeats: %d\n", table_Win, table_Lose);
			clearing_field(field);
		}

	}
	close(sock);
	return 0;
}
