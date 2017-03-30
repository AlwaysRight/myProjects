/*
 * stlucia.c
 */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wait.h>
#include <sys/types.h>

#define MAXTOKEN 10
#define MAXLINE 1024
#define HEALTH 10
#define IN 1
#define OUTSIDE 0
#define TRUE 1
#define FALSE 0
#define ROLLSIZE 6

//exit code
#define ARGC_ERROR 1
#define WINSCOREERROR 2
#define ROLL_FILE_ERROR 3
#define ROLL_CONTENT_ERROR 4
#define START_SUB_ERROR 5
#define PLAYER_QUIT 6
#define BAD_MSG 7
#define BAD_REQUEST 8
#define GAME_OVER 0
#define GO_ON -1
#define running -2

struct player {
	int points;
	int health;
	pid_t pid;
	char name;
	int location;
	int fd1[2];
	int fd2[2];
	int isOver;
	int token;
	int rtn;
};
typedef struct player PlayerProcess;

char validRollChar[6] = { '1', '2', '3', 'H', 'A', 'P' };
int currentRollCharIndex = 0;
char * content = NULL;
char expectedMsg[MAXTOKEN] = "any";

PlayerProcess *players;
char roll[ROLLSIZE + 1];
int gameOver = 0;
int diePlayer = 0;
int maxPoints = 0;
int totalPlayer = 0;
int curPlayer = 0;
int pointsForTurn = 0;

char getNextChar(char * content);
void getNewRoll(char * returnRoll);
void sortRolls(char * rolls);
int getWeightForSort(char c);
void getReRoll(char * oldRoll, char * needroll);

int isValidRollChar(char c);
int loadRollsFromFile(char * file);
int getNumberOfCharInRoll(char * rolled, char c);
int readFromPlayer(PlayerProcess * p);

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

int isValidRollChar(char c) {
	for (int i = 0; i < 6; i++) {
		if (c == validRollChar[i]) {
			return 1;
		}
	}
	return 0;
}

int loadRollsFromFile(char * file) {
	FILE *fp;
	fp = fopen(file, "r");
	if (fp == NULL) {
		return ROLL_FILE_ERROR;
	}
	fseek(fp, 0, SEEK_END);
	int file_size;
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	content = (char *) malloc((file_size + 1) * sizeof(char));
	fread(content, file_size + 1, sizeof(char), fp);
	content[file_size] = '\0';
	fclose(fp);
	//check the content
	for (int i = 0; i < file_size; i++) {
		if (!isValidRollChar(content[i])) {
			if (content[i] != '\r' && content[i] != '\n') {
				return ROLL_CONTENT_ERROR;
			}
		}
	}
	return GO_ON;
}

char getNextChar(char * content) {
	size_t size = strlen(content);
	currentRollCharIndex = currentRollCharIndex % size;
	while (content[currentRollCharIndex] == '\r'
			|| content[currentRollCharIndex] == '\n') {
		currentRollCharIndex++;
		if (currentRollCharIndex >= size) {
			currentRollCharIndex = currentRollCharIndex % size;
		}
	}
	return content[currentRollCharIndex++];
}

void getNewRoll(char * returnRoll) {
	for (int i = 0; i < ROLLSIZE; i++) {

		returnRoll[i] = getNextChar(content);
	}
	returnRoll[ROLLSIZE] = '\0';
	sortRolls(returnRoll);
}

void getReRoll(char * oldRoll, char * needroll) {

	for (int i = 0; i < strlen(oldRoll); i++) {

		for (int j = 0; j < strlen(needroll); j++) {
			if (oldRoll[i] == needroll[j]) {
				oldRoll[i] = getNextChar(content);
				break;
			}
		}
	}
	sortRolls(oldRoll);
}

int getWeightForSort(char c) {
	if (c <= '3' && c >= '1') {
		return c - '0';
	}
	if (c == 'H') {
		return 4;
	}
	if (c == 'A') {
		return 5;
	}
	if (c == 'P') {
		return 6;
	}
	return -1;
}

void sortRolls(char * rolls) {
	int minIndex = 0;
	for (int i = 0; i < ROLLSIZE; i++) {
		minIndex = i;
		for (int j = i + 1; j < ROLLSIZE; j++) {
			if (getWeightForSort(rolls[j])
					< getWeightForSort(rolls[minIndex])) {
				minIndex = j;
			}
		}
		if (minIndex != i) {
			char temp = rolls[i];
			rolls[i] = rolls[minIndex];
			rolls[minIndex] = temp;
		}
	}
}

PlayerProcess * getClaimPlayer() {
	for (int i = 0; i < totalPlayer; i++) {
		if (players[i].isOver == 0 && players[i].location == IN) {
			return &players[i];
		}
	}
	return NULL;
}

void informMsgToOthers(PlayerProcess * p, char *message, int exceptSelf) {
	for (int i = 0; i < totalPlayer; i++) {
		if (!players[i].isOver && (!exceptSelf || p != &players[i])) {
			write(players[i].fd1[1], message, strlen(message));
		}
	}
}

void sendWinner(PlayerProcess * p) {
	char message[MAXLINE];
	memset(message, 0, MAXLINE);
	sprintf(message, "winner %c\n", p->name);
	informMsgToOthers(p, message, 0);
}
void sendClaim(PlayerProcess * p) {
	char message[MAXLINE];
	memset(message, 0, MAXLINE);
	sprintf(message, "claim %c\n", p->name);
	informMsgToOthers(p, message, 0);
}
void sendMyPointsToOthers(PlayerProcess * p, int points) {
	char message[MAXLINE];
	memset(message, 0, MAXLINE);
	sprintf(message, "points %c %d\n", p->name, points);
	informMsgToOthers(p, message, 1);
}

void sendMyRollToOthers(PlayerProcess * p) {
	char message[MAXLINE];
	memset(message, 0, MAXLINE);
	sprintf(message, "rolled %c %s\n", p->name, roll);
	informMsgToOthers(p, message, 1);
}

//Deal with players' turn
int sendTurn(PlayerProcess * p) {
	char message[MAXLINE];
	getNewRoll(roll);
	memset(message, 0, MAXLINE);
	sprintf(message, "turn %s\n", roll);
	//send a roll to player p
	write(p->fd1[1], message, strlen(message));
	strcpy(expectedMsg, "reroll|keepall");
	return readFromPlayer(p);

}

int sendReRoll(PlayerProcess * p, char * toReRoll) {
	char message[MAXLINE];
	getReRoll(roll, toReRoll);
	memset(message, 0, MAXLINE);
	sprintf(message, "rerolled %s\n", roll);
	write(p->fd1[1], message, strlen(message));
	strcpy(expectedMsg, "reroll|keepall");
	return readFromPlayer(p);
}

void healing(PlayerProcess * p) {
	if (p->location == OUTSIDE) {
		int healing = getNumberOfCharInRoll(roll, 'H');
		if (healing > 0) {
			if (p->health + healing >= HEALTH) {
				fprintf(stderr, "Player %c healed %d, health is now %d\n",
						p->name,
						HEALTH - p->health, HEALTH);
				p->health = HEALTH;
			} else {
				fprintf(stderr, "Player %c healed %d, health is now %d\n",
						p->name, healing, p->health + healing);
				p->health += healing;
			}
		}
	}
}
void myAttackToOthers(PlayerProcess * p, int damage) {
	for (int i = 0; i < totalPlayer; i++) {
		if (!players[i].isOver && (p != &players[i])) {
			int d = damage;
			if (damage >= players[i].health) {
				d = players[i].health;
				players[i].health = 0;
			} else {
				players[i].health -= damage;
			}
			fprintf(stderr, "Player %c took %d damage, health is now %d\n",
					players[i].name, d, players[i].health);
		}
	}
	char message[MAXLINE];
	memset(message, 0, MAXLINE);
	sprintf(message, "attacks %c %d %s\n", p->name, damage, "out");
	informMsgToOthers(p, message, 0);
}

void myAttackToClaim(PlayerProcess * p, PlayerProcess * claim, int damage) {
	int d = damage;
	if (damage >= claim->health) {
		d = claim->health;
		claim->health = 0;
		p->location = IN;
	} else {
		claim->health -= damage;
	}
	fprintf(stderr, "Player %c took %d damage, health is now %d\n", claim->name,
			d, claim->health);

	char message[MAXLINE];
	memset(message, 0, MAXLINE);
	sprintf(message, "attacks %c %d %s\n", p->name, damage, "in");
	informMsgToOthers(p, message, 0);
	if (claim->health < 1) {
		fprintf(stderr, "Player %c claimed StLucia\n", p->name);
		pointsForTurn += 1;
		sendClaim(p);
	}

}

int attack(PlayerProcess * p) {
	int a = 0;
	if ((a = getNumberOfCharInRoll(roll, 'A')) > 0) {
		PlayerProcess * claim = getClaimPlayer();
		if (claim == NULL) {
			p->location = IN;
			fprintf(stderr, "Player %c claimed StLucia\n", p->name);
			sendClaim(p);
			pointsForTurn += 1;
		} else if (p->location == IN) {
			myAttackToOthers(p, a);
		} else {
			myAttackToClaim(p, claim, a);
			if (claim->health >= 1) {
				char message[MAXLINE];
				memset(message, 0, MAXLINE);
				sprintf(message, "stay?\n");
				write(claim->fd1[1], message, strlen(message));
				strcpy(expectedMsg, "go|stay");
				return readFromPlayer(claim);
			}
		}
	}
	return GO_ON;
}

void myPoints(PlayerProcess * p) {
	int n = 0;
	if ((n = getNumberOfCharInRoll(roll, '1')) > 2) {
		pointsForTurn += n - 2;
	}
	if ((n = getNumberOfCharInRoll(roll, '2')) > 2) {
		pointsForTurn += 2 + n - 3;
	}
	if ((n = getNumberOfCharInRoll(roll, '3')) > 2) {
		pointsForTurn += 3 + n - 3;
	}
	if ((n = getNumberOfCharInRoll(roll, 'P')) > 0) {
		p->token += n;
		if (p->token >= 10) {
			p->token = p->token - 10;
			pointsForTurn += 1;
		}
	}
	p->points += pointsForTurn;
	if (pointsForTurn > 0) {
		fprintf(stderr, "Player %c scored %d for a total of %d\n", p->name,
				pointsForTurn, p->points);
		sendMyPointsToOthers(p, pointsForTurn);
	}
}

void checkEliminationToOthers(PlayerProcess * p) {
	for (int i = 0; i < totalPlayer; i++) {
		if (players[i].isOver == FALSE && players[i].health < 1) {
			char message[MAXLINE];
			memset(message, 0, MAXLINE);
			sprintf(message, "eliminated %c\n", players[i].name);
			informMsgToOthers(p, message, 0);
			players[i].isOver = TRUE;
		}
	}
}
int checkGameOver(PlayerProcess * p) {
	if (p->points >= maxPoints) {
		return TRUE;
	}
	for (int i = 0; i < totalPlayer; i++) {
		if (players[i].health >= 1 && p != &players[i]) {
			return FALSE;
		}
	}
	return TRUE;
}

int updatePlayer(PlayerProcess * p) {
	fprintf(stderr, "Player %c rolled %s\n", p->name, roll);
	pointsForTurn = 0;
	if (p->location == IN) {
		pointsForTurn += 2;
	}
	sendMyRollToOthers(p);
	//healing
	healing(p);
	//attack
	int exitCode;
	if ((exitCode = attack(p)) != GO_ON) {
		return exitCode;
	}
	//calculate points
	myPoints(p);

	checkEliminationToOthers(p);
	if (checkGameOver(p) == TRUE) {
		fprintf(stderr, "Player %c wins\n", p->name);
		sendWinner(p);
		return GAME_OVER;
	}

	strcpy(expectedMsg, "any");
	return GO_ON;
}

int checkExpectedMsg(char * msg_type) {
	if (strcmp(msg_type, "keepall") == 0 || strcmp(msg_type, "reroll") == 0) {
		if (strcmp(expectedMsg, "any") != 0
				&& strcmp(expectedMsg, "reroll|keepall") != 0) {
			return BAD_REQUEST;
		}
	} else if (strcmp(msg_type, "go") == 0 || strcmp(msg_type, "stay") == 0) {
		if (strcmp(expectedMsg, "any") != 0
				&& strcmp(expectedMsg, "go|stay") != 0) {
			return BAD_REQUEST;
		}
	} else {
		if (strcmp(expectedMsg, "any") != 0
				&& strcmp(expectedMsg, msg_type) != 0) {
			return BAD_REQUEST;
		}
	}
	return TRUE;
}

int readFromPlayer(PlayerProcess * p) {
	char response[MAXLINE];
	memset(response, 0, MAXLINE);
	int r = read(p->fd2[0], response, MAXLINE);
	if (r == -1) {
		return PLAYER_QUIT;
	}
	if (response[0] == '\n' && r == 1) {
		r = read(p->fd2[0], response, MAXLINE);
	}
	if (r == 0) {
		return BAD_MSG;
	}
	char msg_type[MAXTOKEN];
	char arg1[MAXTOKEN];
	sscanf(response, "%s %s", msg_type, arg1);

	if (checkExpectedMsg(msg_type) == BAD_REQUEST) {
		return BAD_REQUEST;
	}
	if (strcmp(msg_type, "reroll") == 0) {
		if (sscanf(response, "%s %s", msg_type, arg1) != 2) {
			return BAD_MSG;
		}
		if (strlen(arg1) > ROLLSIZE || strlen(arg1) < 1) {
			return BAD_MSG;
		}
		for (int i = 0; i < strlen(arg1); i++) {
			if (!isValidRollChar(arg1[i])) {
				return BAD_MSG;
			}
		}
		return sendReRoll(p, arg1);
	} else if (strcmp(msg_type, "keepall") == 0) {
		return updatePlayer(p);
	} else if (strcmp(msg_type, "stay") == 0) {
	} else if (strcmp(msg_type, "go") == 0) {
		p->location = OUTSIDE;
		players[curPlayer].location = IN; // current player take place of StLucia
		fprintf(stderr, "Player %c claimed StLucia\n", players[curPlayer].name);
		sendClaim(&players[curPlayer]);
		pointsForTurn += 1;
	} else if (strcmp(msg_type, "!") == 0) {
	} else {
		return BAD_MSG;
	}
	return GO_ON;
}

int nextLivePlayer(int start) {
	int find = start + 1;
	while (TRUE) {
		find = find % totalPlayer;
		if (find == start) {
			return -1;
		}
		if (players[find].isOver == 0) {
			return find;
		}
		find++;
	}
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

int checkArg(int argc, char *argv[]) {
	if (argc < 5) {
		fprintf(stderr,
				"Usage: stlucia rollfile winscore prog1 prog2 [prog3 [prog4]]\n");
		return ARGC_ERROR;
	}
	maxPoints = strToInt(argv[2]);
	if (maxPoints <= 0) {
		fprintf(stderr, "Invalid score\n");
		return WINSCOREERROR;
	}

	char *filename = argv[1];
	int loadCode = loadRollsFromFile(filename);
	if (loadCode != GO_ON) {
		if (loadCode == ROLL_FILE_ERROR) {
			fprintf(stderr, "Unable to access rollfile\n");
		} else {
			fprintf(stderr, "Error reading rolls\n");
		}
		free(content);
		return loadCode;
	}
	return GO_ON;
}

int initPlayers(int argc, char *argv[]) {
	for (int i = 0; i < totalPlayer; i++) {
		players[i].location = OUTSIDE;
		players[i].points = 0;
		players[i].health = HEALTH;
		players[i].name = 'A' + i;
		players[i].isOver = FALSE;
		players[i].token = 0;
		players[i].rtn = running;
		char playerNum[MAXTOKEN];
		sprintf(playerNum, "%d", totalPlayer);
		char playerName[2];
		sprintf(playerName, "%c", players[i].name);

		if (pipe(players[i].fd1)) {
			fprintf(stderr, "create pipe failed!\n");
			return START_SUB_ERROR;
		}
		if (pipe(players[i].fd2)) {
			fprintf(stderr, "create pipe failed!\n");
			return START_SUB_ERROR;
		}
		players[i].pid = fork();
		if (players[i].pid < 0) {
			fprintf(stderr, "fork failed!\n");
			return START_SUB_ERROR;
		}
		if (players[i].pid == 0) {
			close(players[i].fd1[1]);
			close(players[i].fd2[0]);

			close(0);
			dup2(players[i].fd1[0], 0);

			close(1);
			dup2(players[i].fd2[1], 1);
			fclose(stderr);
			execlp(argv[3 + i], argv[3 + i], playerNum, playerName, NULL);
			return START_SUB_ERROR;
		} else {
			close(players[i].fd1[0]);
			close(players[i].fd2[1]);
		}
	}
	return GO_ON;
}

//hander SIGINT
void interruptHandler(int sig) {
	//TODO
	//stop =1;
	for (int i = 0; i < totalPlayer; i++) {
		if (players[i].rtn == running) {
			write(players[i].fd1[1], "shutdown\n", strlen("shutdown\n"));
			sleep(2000);
		}
	}
	for (int i = 0; i < totalPlayer; i++) {
		if (players[i].rtn == running) {
			kill(players[i].pid, SIGINT);
			fprintf(stderr, "Player %c terminated due to signal %d\n",
					players[i].name, SIGINT);
		}
	}
	free(content);
	free(players);
}

int main(int argc, char *argv[]) {
	signal(SIGINT, interruptHandler);
	setvbuf(stdout, NULL, _IONBF, 0);
	int code = checkArg(argc, argv);
	if (code != GO_ON) {
		return code;
	}

	totalPlayer = argc - 3;
	players = (PlayerProcess *) malloc(sizeof(PlayerProcess) * totalPlayer);
	int exitCode;
	if ((exitCode = initPlayers(argc, argv)) == START_SUB_ERROR) {
		fprintf(stderr, "Unable to start subprocess\n");
		return exitCode;
	}
	for (int i = 0; i < totalPlayer; i++) {
		strcpy(expectedMsg, "!");
		exitCode = readFromPlayer(&players[i]);
		if (exitCode != GO_ON) {
			// read and check the !
			fprintf(stderr, "Unable to start subprocess\n");
			return START_SUB_ERROR;
		}
	}

	strcpy(expectedMsg, "any");

	while (TRUE) {
		exitCode = sendTurn(&players[curPlayer]);
		if (exitCode != GO_ON) {
			break;
		}
		curPlayer = nextLivePlayer(curPlayer);
	}
	if (exitCode == PLAYER_QUIT) {
		fprintf(stderr, "Player quit\n");
		return PLAYER_QUIT;
	} else if (exitCode == BAD_MSG) {
		fprintf(stderr, "Invalid message received from player\n");
		return BAD_MSG;
	} else if (exitCode == BAD_REQUEST) {
		fprintf(stderr, "Invalid request by player\n");
		return BAD_REQUEST;
	}

	//Check for exit
	for (int i = 0; i < totalPlayer; i++) {
		waitpid(players[i].pid, &players[i].rtn, 0);
		if (players[i].rtn != GAME_OVER) {
			fprintf(stderr, "Player %c exited with status %d\n",
					players[i].name, players[i].rtn);
		}
	}
	free(content);
	free(players);
	return 0;
}
