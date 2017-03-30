/*
 * hass.c
 */

#include <stdio.h>
#include <string.h>
#include "player.h"

int needReRoll(char *rolled, Player *p, char * rerolledChars);
int willStay(Player * p);

/**
 * according to the rolled chars and our player's strategy, decide which chars to rerolled
 *
 * if nothing need rerolled, return FALSE. otherwise return TRUE
 *
 * and the rerolledChars represent the chars to rerolled
 */
int needReRoll(char *rolled, Player *p, char * rerolledChars) {
	int index = 0;
	for (int i = 0; rolled[i] != '\0'; i++) {

		if (p->location == IN) {
			if (rolled[i] == 'P') {
				continue;
			}
		} else {
			if (rolled[i] == 'A') {
				for (int i = 0; i < total; i++) {
					if (players[i].isOver == FALSE && players[i].location == IN) {
						if (players[i].health
								<= getNumberOfCharInRoll(rolled, 'A')) {
							continue;
						}
					}
				}
			}
		}
		rerolledChars[index++] = rolled[i];
	}
	if (index == 0) {
		return FALSE;
	}
	rerolledChars[index] = '\0';
	return TRUE;
}

/**
 * When the hub ask the player whether to stay, the player
 * answer stay or go according its status;
 * Eait will stay if its health >= 5
 */
int willStay(Player * p) {
	return TRUE;
}

int main(int argc, char *argv[]) {
	setvbuf(stdout, NULL, _IONBF, 0);
	//check arguments
	if (argc != 3) {
		fprintf(stderr, "Usage: player number_of_players my_id\n");
		return ARGC_ERROR;
	}
	total = strToInt(argv[1]);
	if (total < 2 || total > 26) {
		fprintf(stderr, "Invalid player count\n");
		return WINSCOREERROR;
	}

	if (strlen(argv[2]) != 1 || argv[2][0] > 'A' + (total - 1)
			|| argv[2][0] < 'A') {
		fprintf(stderr, "Invalid player ID\n");
		return PLAYER_ID_ERROR;
	}
	printf("!");
	initPlayers(total);
	int exitCode = play(&players[argv[2][0] - 'A']);
	if (exitCode == ROLL_CONTENT_ERROR) {
		fprintf(stderr, "Unexpectedly lost contact with StLucia\n");
	} else if (exitCode == BAD_MSG) {
		fprintf(stderr, "Bad message from StLucia\n");
	}
	free(players);
	return exitCode;
}
