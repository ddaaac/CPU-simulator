#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "schedule.h"
#include "process.h"
#include "queue.h"
#include "macro.h"

extern Process process[TOTAL_PID];
extern Queue* ready_queue;
extern int cur_time;

// FCFS 방식은 ready queue에 들어온 time이 작을수록 우선순위 높음
int schedule_FCFS(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	process[pid].priority = cur_time;
	return !ERR;
}
// SJF 방식은 남아있는 cpu burst time이 작을수록 우선순위 높음
int schedule_SJF(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	// aging으로 인해 증가한 우선순위까지 고려. 이 때, priority는 0보다 작아질 수 없음.
	int p = process[pid].cpu_remaining_time - process[pid].aging_priority;
	process[pid].priority = p < 0 ? 0 : process[pid].cpu_remaining_time;
	return !ERR;
}
// PRIORITY 방식은 process 고유의 priority를 사용
int schedule_PRIORITY(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	// aging으로 인해 증가한 우선순위까지 고려. 이 때, priority는 0보다 작아질 수 없음.
	int p = process[pid].own_priority - process[pid].aging_priority;
	process[pid].priority = p < 0 ? 0 : process[pid].own_priority;
	return !ERR;
}
// RR 방식은 FCFS와 마찬가지로 선입선출
int schedule_RR(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	process[pid].priority = cur_time;
	return !ERR;
}

// ready queue에서 우선순위가 가장 높은 process의 id를 return. 없으면 NOT_EXIST return
int highest_priority_process() {
	int min = INFINITY;
	int highest_priority_process = NOT_EXIST;
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		// ready queue에 존재하는 process 중 가장 priority 값이 작은(우선순위가 높은) process를 찾음
		// 이 때, 같은 priority라면 pid가 작을수록 우선권 부여
		if (ready_queue->process[pid] == EXIST && min > process[pid].priority) {
			min = process[pid].priority;
			highest_priority_process = pid;
		}
	}
	// -1이 return 된다면 ready_queue가 비어있음을 의미
	return highest_priority_process;
}

Schedule* init_schedule(int method, int preemptive, int time_quantum, int aging) {
	Schedule* s = malloc(sizeof(Schedule));
	s->method = method;
	s->preemptive = preemptive;
	s->aging = aging;
	s->time_quantum = time_quantum;
	// scheduling method에 따라 우선순위 부여 방식이 달라짐
	switch (method)
	{
	case FCFS:
		s->give_priority = schedule_FCFS;
		break;
	case SJF:
		s->give_priority = schedule_SJF;
		break;
	case PRIORITY:
		s->give_priority = schedule_PRIORITY;
		break;
	case RR:
		s->give_priority = schedule_RR;
	default:
		break;
	}
	s->avg_turnaround_time = 0;
	s->avg_waiting_time = 0;
	s->avg_response_time = 0;
	s->err = !ERR;
	s->total_time = 0;
	return s;
}

void print_chart(int chart[], int total_time) {
	int time_unit = 0;
	while (time_unit < total_time) {
		printf("\n");
		printf("%9s", "time|");
		for (int i = time_unit; i < time_unit+20 && i < total_time; i++) {
			// 시간은 1부터 시작하므로 1을 더한 값을 출력
			printf("%3d|", i + 1);
		}
		printf("\n");

		printf("%9s", "process|");
		for (int i = time_unit; i < time_unit+20 && i < total_time; i++) {
			if (chart[i] == NOT_EXIST)
				printf("%3s|", "IDL");
			else
				printf("%3d|", chart[i]);
		}
		time_unit += 20;
		printf("\n");
	}
	printf("\n\n\n");
}

void evaluate(Schedule* schedule_manager[]) {
	printf("Evaluation.\n\n");
	printf("%19s %8s %8s %8s %8s %8s %8s %8s %8s %8s\n", "Scheduling Method|",
		"FCFS", "NP_SJF", "P_SJF", "NP_PRI", "P_PRI", "ANP_PRI", "AP_PRI", "RR3", "RR5");
	printf("%19s ", "Average Waiting|");
	for (int i = 0; i < NUM_SCHEDULING; i++)
		printf("%8.2f ", schedule_manager[i]->avg_waiting_time);
	printf("\n");
	printf("%19s ", "Average Turnaround|");
	for (int i = 0; i < NUM_SCHEDULING; i++)
		printf("%8.2f ", schedule_manager[i]->avg_turnaround_time);
	printf("\n");
	printf("%19s ", "Average Response|");
	for (int i = 0; i < NUM_SCHEDULING; i++)
		printf("%8.2f ", schedule_manager[i]->avg_response_time);
	printf("\n");

}