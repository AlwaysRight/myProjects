/*
 * player.h
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXTOKEN 10
#define MAXLINE 1024
#define HEALTH 10
#define IN 1
#define OUTSIDE 0
#define TRUE 1
#define FALSE 0
#define ROLLLENGTH 6

//exit code
enum exitCode {
	GAME_OVER,
	ARGC_ERROR,
	WINSCOREERROR,
	PLAYER_ID_ERROR,
	ROLL_CONTENT_ERROR,
	BAD_MSG,
	GO_ON
};

struct player {
	int health;
	int location;
	char name[2];
	int rerolled_time;
	int isOver;
};
typedef struct player Player;

Player *players;

char expectedMsg[MAXTOKEN] = "any";
char validRollChar[6] = { '1', '2', '3', 'H', 'A', 'P' };
int total = 0;

void initPlayers(int total);
int needReRoll(char *rolled, Player *p, char * rerolledChars);
int willStay(Player * p);
int getNumberOfCharInRoll(char * rolled, char c);
void updateStateAfterRoll(char *rolled, Player * p);
int dealTurn(char *rolled, Player * p);
void dealAttack(char * name, int damage, char * type, Player *p);
int dealMessage(char *message, Player * p);
int play(Player *p);
int isValidRollChar(char c);
int dealElimanated(char *name);
int dealRolled(char * message);

void initPlayers(int total) {
	players = (Player *) malloc(sizeof(Player) * total);
	for (int i = 0; i < total; i++) {
		players[i].health = HEALTH;
		players[i].location = OUTSIDE;
		char c = 'A' + i;
		sprintf(players[i].name, "%c", c);
		players[i].rerolled_time = 0;
		players[i].isOver = FALSE;

	}
}

int isValidRollChar(char c) {
	for (int i = 0; i < 6; i++) {
		if (c == validRollChar[i]) {
			return 1;
		}
	}
	return 0;
}
/**
 * Get how many char c in a rolled
 */
int getNumberOfCharInRoll(char * rolled, char c) {
	int num = 0;
	for (int i = 0; rolled[i] != '\0'; i++) {
		if (rolled[i] == c) {
			num++;
		}
	}
	return num;
}
/**
 * Use the rolled to update the state of the player
 */
void updateStateAfterRoll(char *rolled, Player * p) {
	if (p->location == OUTSIDE) {
		int healing = getNumberOfCharInRoll(rolled, 'H');
		if (p->health + healing >= HEALTH) {
			//fprintf(stderr, "%s healing : %d\n", p->name, HEALTH - p->health);
			p->health = HEALTH;
		} else {
			//fprintf(stderr, "%s healing : %d\n", p->name, healing);
			p->health += healing;
		}
	}
	//fprintf(stderr, "Current points: %d health: %d\n", p->score, p->health);
}

/**
 * Deal with a player's turn. it may choose to rerolled the dices.
 */
int dealTurn(char *rolled, Player * p) {
	//check rolled chars first
	if (strlen(rolled) != ROLLLENGTH) {
		return FALSE;
	}
	for (int i = 0; i < ROLLLENGTH; i++) {
		if (!isValidRollChar(rolled[i])) {
			return FALSE;
		}
	}
	char rerolledChars[MAXTOKEN];
	int needFlag = needReRoll(rolled, p, rerolledChars);
	if (p->rerolled_time >= 2 || needFlag == FALSE) {
		printf("keepall\n");
		updateStateAfterRoll(rolled, p);
		p->rerolled_time = 0;
		strcpy(expectedMsg, "any");
		return TRUE;
	}
	printf("reroll %s\n", rerolledChars);
	p->rerolled_time++;
	strcpy(expectedMsg, "rerolled");
	return TRUE;
}

/**
 * deal with an attack message
 */
void dealAttack(char * name, int damage, char * type, Player *p) {
	for (int i = 0; i < total; i++) {
		if (players[i].isOver == FALSE && strcmp(name, players[i].name) != 0) {
			if (strcmp(type, "in") == 0 && players[i].location == IN) {
				players[i].health -= damage;
				if (p == &players[i] && p->health >= 1) {
					strcpy(expectedMsg, "stay?");
				}

			} else if (strcmp(type, "out")
					== 0&& players[i].location == OUTSIDE) {
				players[i].health -= damage;
			}
		}
	}

}

int dealEliminated(char *name) {
	for (int i = 0; i < total; i++) {
		if (strcmp(name, players[i].name) == 0 && !players[i].isOver) {
			players[i].isOver = TRUE;
			return GO_ON;
		}
	}
	return BAD_MSG;
}

int dealRolled(char * message) {
	char msgType[MAXTOKEN];
	char playerName[MAXTOKEN];
	char rolled[MAXTOKEN];
	if (sscanf(message, "%s %s %s", msgType, playerName, rolled) != 3) {
		return BAD_MSG;
	}
//	if ((playerName[0] - 'A') >= total || strlen(rolled) != ROLLLENGTH) {
//		return BAD_MSG;
//	}
//	for (int i = 0; i < ROLLLENGTH; i++) {
//		if (!isValidRollChar(rolled[i])) {
//			return BAD_MSG;
//		}
//	}
	Player * p = &players[playerName[0] - 'A'];
	if (p->isOver == TRUE) {
		return BAD_MSG;
	}
	if (p->location == OUTSIDE) {
		int healing = getNumberOfCharInRoll(rolled, 'H');
		p->health += healing;
		if (p->health > HEALTH) {
			p->health = HEALTH;
		}
	}
	return GO_ON;
}
/**
 * Deal a single message
 */
int dealMessage(char *message, Player * p) {
	char msg_type[MAXTOKEN];
	char arg1[MAXTOKEN];
	char arg2[MAXTOKEN];
	int number;
	sscanf(message, "%s %s", msg_type, arg1);
	if (strcmp(expectedMsg, "any") != 0 && strcmp(expectedMsg, msg_type) != 0) {
		return BAD_MSG;
	}
	if (strcmp(msg_type, "turn") == 0 || strcmp(msg_type, "rerolled") == 0) {
		if (dealTurn(arg1, p) == FALSE) {
			return BAD_MSG;
		}
	} else if (strcmp(msg_type, "attacks") == 0) {
		if (sscanf(message, "%s %s %d %s", msg_type, arg1, &number, arg2)
				!= 4) {
			return BAD_MSG;
		}
		dealAttack(arg1, number, arg2, p);
	} else if (strcmp(msg_type, "eliminated") == 0) {
		if (sscanf(message, "%s %s %s", msg_type, arg1, arg2) != 2) {
			return BAD_MSG;
		}
		if (strlen(arg1) != 1 || ((arg1[0] - 'A') >= total)) {
			return BAD_MSG;
		}
		if (strcmp(arg1, p->name) == 0) {
			return GAME_OVER;
		}
		return dealEliminated(arg1);
	} else if (strcmp(msg_type, "claim") == 0) {
		if (sscanf(message, "%s %s %s", msg_type, arg1, arg2) != 2) {
			return BAD_MSG;
		}
		for (int i = 0; i < total; i++) {
			if (strcmp(arg1, players[i].name) == 0) {
				players[i].location = IN;
			} else {
				players[i].location = OUTSIDE;
			}
		}
	} else if (strcmp(msg_type, "stay?") == 0) {
		if (willStay(p) == TRUE) {
			printf("stay\n");
		} else {
			printf("go\n");
			p->location = OUTSIDE;
		}
		strcpy(expectedMsg, "any");
	} else if (strcmp(msg_type, "winner") == 0) {
		if ((sscanf(message, "%s %s", msg_type, arg1) != 2) || strlen(arg1) != 1
				|| ((arg1[0] - 'A') >= total)) {
			return BAD_MSG;
		}
		return GAME_OVER;
	} else if (strcmp(msg_type, "shutdown") == 0) {
		return ROLL_CONTENT_ERROR;
	} else if (strcmp(msg_type, "points") == 0) {
		if (sscanf(message, "%s %s %d", msg_type, arg1, &number) != 3
				|| number <= 0) {
			return BAD_MSG;
		}
	} else if (strcmp(msg_type, "rolled") == 0) {
		return dealRolled(message);
	} else {
		return BAD_MSG;
	}
	return GO_ON;
}

int play(Player *p) {
	char buff[MAXLINE];
	int exitCode;
	while (NULL != fgets(buff, MAXLINE, stdin)) {
		fprintf(stderr, "From StLucia:%s", buff);

		if ((exitCode = dealMessage(buff, p)) != GO_ON) {
			return exitCode;
		}
	}
	return ROLL_CONTENT_ERROR;
}

/**
 * Convert a string to an integer
 * if the string is not a valid number, return 0
 */
int strToInt(char * str) {
	int num = 0;
	int x = 1;
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] < '0' || str[i] > '9') {
			return FALSE;
		}
		num = num * x + (str[i] - '0');
		x = x * 10;
	}
	return num;
}
