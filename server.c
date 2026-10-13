#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

#define BUFLEN 81

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

int main() {
	int sockMain, length, msgLength;
	struct sockaddr_in servAddr, clientAddr;
    char field[9];
	clearing_field(field);
	char move_buf[BUFLEN];
    int move = -1;
    char player_figure = 'O';
	char enemy_figure = 'X';
	int WIN_FLAG = 0, DRAW_FLAG = 0, table_Win = 0, table_Lose = 0;

	if(( sockMain = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
	{
		perror("Сервер не может открыть socket для UDP.\n");
		exit(1);
	}

	bzero((char*)&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = 0;

	if ( bind(sockMain, (struct sockaddr*)&servAddr, sizeof(servAddr)) )
	{
		perror("Связывание сервера неудачно.");
		exit(1);
	}

	length = sizeof(servAddr) ;
	if ( getsockname( sockMain, (struct sockaddr*)&servAddr, (socklen_t *)&length ) )
	{
		perror("Вызов getsockname неудачен.");
		exit(1);
	}

	printf("СЕРВЕР: номер порта -> %d\n", ntohs(servAddr.sin_port ));
	print_field(field);

	while (1) {
		length = sizeof(clientAddr);

		printf("Ход оппонента...\n");
		if ( (msgLength = recvfrom(sockMain, field, strlen(field), 0 , (struct sockaddr*)&clientAddr, (socklen_t *)&length)) < 0 )
		{ 
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

		print_field(field);
		printf("Выберите поле (1-9): \n");
        while (field[move-1] != '*') {       // проверка корректности хода
        	scanf("%s", move_buf);
			for (int i = 0; i < strlen(move_buf); i++) {
				move *= 10;
				move = move_buf[i] - '0';
			}
		
		}
		move_processing(field,player_figure,move);
		print_field(field);

		if(sendto (sockMain, field, msgLength, 0 , (struct sockaddr*)&clientAddr , sizeof(clientAddr)) < 0 )
		{
			perror( "Ошибка sendto.\n" );
			exit(1);
		}

		if ((WIN_FLAG = win_rule(field, WIN_FLAG, player_figure)) == 1) {
			WIN_FLAG = 0;
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
		}
	}
	return 0;
}
