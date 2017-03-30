/*
 CITS2002 Project 1 2016
 Name(s):		student-name1 (, student-name2)
 Student number(s):	student-number1 (, student-number2)
 Date:		date-of-submission
 */
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAXEVENTNUM 100 // max number of events in a process#define MAXPROCESSNUM 50 // max number of process in a scenario file#define MAXTYPELENGHT 10 // max length of the event type string#define MAX_LINE 1024 // max length of a line in a file
#define TRUE 1
#define FALSE 0
#define BLOCK_SWITCH 1
#define TIME_SWITCH 0

//The type of the eventenum event_type{admit, read, write, verify, exit};
//define an event structure//including the time, event type and disk sector(for read and write event)struct event{int time;int end_time;enum event_type type;int disk_sector; // will be -1 if the type is not read or write
int block_time; // record the read and write event block time};typedef struct event Event;
//define a process structure//including the pid ,events array, and the number of events in the process,
//which event will be executed next
struct process {
	int pid;
	Event events[MAXEVENTNUM];
	int event_num;
	int next_excute_event;
	int execute_time;
};
typedef struct process Process;

Process process[MAXPROCESSNUM]; //all initially process
Process ready[MAXPROCESSNUM]; // store ready process
Process block[MAXPROCESSNUM]; // store block process

int current_num = 0; // number of process in the file
int ready_num = 0; // current number of ready process
int block_num = 0; // current number of block process

//characteristics of the system
int timequantum;
int diskrpm;
int disksectors;

int current_time; // represent the current absolutely time

int turnaround_time = 0;// total process turn around time
int writing_time = 0;
int reading_time = 0;

int debug = TRUE; // if debug is 1, print debug log informations

int ready_index = 0; // current execute index in the ready queue
int current_timequantum_time = 0; // the time in a quantum

int current_sector;
//------following deal with load data from file-------

/**
 * Find the process in the array by the pid
 * return NULL if the process not exist
 */
Process* find_by_pid(int pid) {

	for (int i = 0; i < current_num; i++) {
		if (process[i].pid == pid) {
			return &process[i];
		}
	}
	return NULL;

}

/**
 * Add an event, first check the pid exist or not
 */
void add_event(int pid, Event event) {

	Process* p = find_by_pid(pid);
	if (p == NULL) {
		Process np;
		np.pid = pid;
		np.events[0] = event;
		np.event_num = 1;
		np.next_excute_event = 0;
		np.execute_time = 0;
		process[current_num] = np;
		(current_num)++;
	} else {
		p->events[p->event_num] = event;
		p->event_num++;
	}
}

/**
 * Load the process data from scenario file
 */
void load_from_file(char* filename) {
	char buf[MAX_LINE];
	FILE *fp;
	int len;
	if ((fp = fopen(filename, "r")) == NULL) {
		perror("fail to load scenario file");
		return;
	}

	//get timequantum, diskrpm and  distsectors
	for (int i = 0; i < 3; i++) {
		fscanf(fp, "%s", buf);
		if (strcmp(buf, "diskrpm") == 0) {
			fscanf(fp, "%d", &diskrpm);
		}
		if (strcmp(buf, "timequantum") == 0) {
			fscanf(fp, "%d", &timequantum);
		} else if(strcmp(buf, "disksectors") == 0){
			fscanf(fp, "%d", &disksectors);
		}
	}

	while (fgets(buf, MAX_LINE, fp) != NULL) {
		len = strlen(buf);
		if (len < 3) {
			continue;
		}

		char type[MAXTYPELENGHT];
		int pid;
		int time;
		int disk_sector = -1;
		sscanf(buf, "%d%d%s%d", &time, &pid, type, &disk_sector);
		Event event;
		event.disk_sector = disk_sector;
		event.time = time;
		if (strcmp(type, "admit") == 0) {
			event.type = admit;
		} else if (strcmp(type, "read") == 0) {
			event.type = read;
		} else if (strcmp(type, "write") == 0) {
			event.type = write;
		} else {
			event.type = exit;
		}
		add_event(pid, event);
	}
}

//following deal with execute process's events

/**
 * Add to ready queue
 */
void add_to_ready(Process p) {
	if (ready_num == 0) {
		ready[ready_num++] = p;
		return;
	}
	//To add the process to the queue's end position
	// we should add it before the current process in the array
	for (int i = ready_num; i > ready_index; i--) {
		ready[i] = ready[i - 1];
	}
	ready[ready_index] = p;
	ready_index++;
	ready_num++;

}

/**
 * remove the process at index from ready queue
 */
void rm_from_ready(int index) {
	for (int i = index; i < ready_num - 1; i++) {
		ready[i] = ready[i + 1];
	}
	ready_num--;
}

/**
 * Add the process p to block queue
 */
void add_to_block(Process p) {
	block[block_num++] = p;
}

/**
 * remove process at the index from block queue
 */
void rm_from_block(int index) {
	for (int i = index; i < block_num - 1; i++) {
		block[i] = block[i + 1];
	}
	block_num--;
}

/**
 * remove the process from initial all process array
 */
void rm_from_unadmit() {
	for (int i = 0; i < current_num - 1; i++) {
		process[i] = process[i + 1];
	}
	current_num--;
}

/**
 * decide which sector at the time
 */
int get_cur_sector(int time){
	double current_sector = ((time * ((long double) diskrpm / 60 / 1000000)) - (int)(time * ((long double) diskrpm / 60 / 1000000))) * disksectors;
	//printf("%f %d\n", current_sector, (int)(current_sector));
	return (int)(current_sector);
}
/**
 *Judge whether the process can get the required sector at the current_time
 *It can get it when turning to the sector's header at the microseconds,
 *but not at the header at one microseconds before microseconds.
 *
 */
int is_at_head(Process *p) {
	int require_sector = p->events[p->next_excute_event].disk_sector;
	if (get_cur_sector(current_time) == require_sector
			&& get_cur_sector(current_time - 1) != require_sector) {
		return TRUE;
	}
	return FALSE;
}

/**
 * Search the block , judge whether some process can get required sources
 *
 */
void deal_block() {

	if (block_num < 1) {
		return;
	}

	//find the block process can reach the sector required at the currrent_time
	int index;
	for (index = 0; index < block_num; index++) {
		if (is_at_head(&block[index])) {
			if (block[index].events[block[index].next_excute_event].type
					== write) {

				block[index].events[block[index].next_excute_event].type =
						verify;
				if (debug) {
					printf("@%-12i%i\twrite   -> verify\t%i\n", current_time,
							block[index].pid,
							block[index].events[block[index].next_excute_event].disk_sector);
				}

			} else {
				if (debug) {
					printf("@%-12i%i\tblocked -> ready\t%i\n", current_time,
							block[index].pid,
							block[index].events[block[index].next_excute_event].disk_sector);
				}
				if (block[index].events[block[index].next_excute_event].type
						== verify) {
					writing_time +=
							current_time
									- block[index].events[block[index].next_excute_event].block_time;
				} else {
					reading_time +=
							current_time
									- block[index].events[block[index].next_excute_event].block_time;
				}
				block[index].next_excute_event++;
				add_to_ready(block[index]);
				rm_from_block(index);
				index--;
			}
		}
	}

}

/**
 * Switch to next process
 * If the flag is BLOCK_SWITCH, next ready_index = ready_index % ready_num;
 * If the flag is TIME_SWITCH, next ready_index =  (ready_index + 1) % ready_num;
 */
void switch_next_process(int flag) {
	current_timequantum_time = 0;
	if (ready_num == 0) {
		return;
	}
	if (flag == TIME_SWITCH) {
		ready_index = (ready_index + 1) % ready_num;
	} else {
		ready_index = ready_index % ready_num;
	}
	if (debug) {
		printf("@%-12i%i\tready   -> running\n", current_time,
				ready[ready_index].pid);
	}
}

/**
 * deal the ready queue
 */
void deal_ready() {
	if (ready_num == 0) {
		current_time++;
		return;
	}
	if (current_timequantum_time == 0) {
		if (debug) {
			printf("@%-12i%i\tready   -> running\n", current_time,
					ready[ready_index].pid);
		}
	}
	Event *event =
			&ready[ready_index].events[ready[ready_index].next_excute_event];
	if (ready[ready_index].execute_time == event->time) {
		//meet block event
		if (event->type == read) {
			event->block_time = current_time;
			if (debug) {
				printf("@%-12i%i\trunning -> read\t\t%i\n", current_time,
						ready[ready_index].pid, event->disk_sector);
			}
			add_to_block(ready[ready_index]);
			rm_from_ready(ready_index);
		} else if (event->type == write) {
			event->block_time = current_time;
			if (debug) {
				printf("@%-12i%i\trunning -> write\t%i\n", current_time,
						ready[ready_index].pid, event->disk_sector);
			}
			add_to_block(ready[ready_index]);
			rm_from_ready(ready_index);
		} else {
			if (debug) {
				printf("@%-12i%i\trunning -> exit\n", current_time,
						ready[ready_index].pid);
			}
			turnaround_time += current_time - ready[ready_index].events[0].time;
			rm_from_ready(ready_index);
		}
		switch_next_process(BLOCK_SWITCH);
	} else if (current_timequantum_time != 0
			&& current_timequantum_time % timequantum == 0) {
		// a time quantum ends
		current_timequantum_time = 0;
		if (ready_num > 1) {
			if (debug) {
				printf("@%-12i%i\trunning -> ready\n", current_time,
						ready[ready_index].pid);
			}
			switch_next_process(TIME_SWITCH);
		} else {
			if (debug) {
				printf("@%-12i%i\trunning -> running\n", current_time,
						ready[ready_index].pid);
			}
		}
	}
	if (ready_num > 0) {
		ready[ready_index].execute_time++;
		current_time++;
		current_timequantum_time++;
	}else{
		current_time++;
	}

}

/**
 * find the process can admit at current time
 * remove it from this array to ready list
 */
void search_unadmit() {
	if (current_num == 0) {
		return;
	}
	if (process[0].events[0].time == current_time) {
		if (debug) {
			printf("@%-12i%i\tadmit   -> ready\n", process[0].events[0].time,
					process[0].pid);
		}
		process[0].next_excute_event++;
		add_to_ready(process[0]);
		process[0].execute_time = 0;
		rm_from_unadmit();
	}
}

/**
 * execute all the process
 */
void excute() {

	current_time = process[0].events[0].time;
	while (1) {
		search_unadmit();
		deal_block();
		deal_ready();
		if (ready_num == 0 && block_num == 0 && current_num == 0) {
			// all the process have exited
			break;
		}

	}
}

int main(int argc, char ** argv) {
	if (argc < 2) {
		printf("usage: your program - filename\n");
		return -1;
	}

	char * filename = argv[1];
	load_from_file(filename);
	printf(
			"# form of events understood by osevents-view:\n#\n#   @usecs  processID  oldState	 ->  newState  [sector]\n#\n");
	printf("@timequantum\t%d\n@diskrpm\t%d\n@disksectors\t%d\n", timequantum,
			diskrpm, disksectors);
	excute();
	//printf("%d \n", get_cur_sector(17611250));
	printf("%d\t%d\t%d\n", turnaround_time, reading_time, writing_time);
	return 0;

}
