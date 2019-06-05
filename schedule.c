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
extern Schedule* schedule;

// FCFS ����� ready queue�� ���� time�� �������� �켱���� ����
int schedule_FCFS(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	process[pid].priority = cur_time;
	return !ERR;
}
// SJF ����� �����ִ� cpu burst time�� �������� �켱���� ����
int schedule_SJF(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	// aging���� ���� ������ �켱�������� ���. �� ��, priority�� 0���� �۾��� �� ����.
	int p = process[pid].cpu_remaining_time - process[pid].aging_priority;
	process[pid].priority = p < 0 ? 0 : p;
	return !ERR;
}
// PRIORITY ����� process ������ priority�� ���
int schedule_PRIORITY(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	// aging���� ���� ������ �켱�������� ���. �� ��, priority�� 0���� �۾��� �� ����.
	int p = process[pid].own_priority - process[pid].aging_priority;
	process[pid].priority = p < 0 ? 0 : p;
	return !ERR;
}
// RR ����� FCFS�� ���������� ���Լ���
int schedule_RR(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	process[pid].priority = cur_time;
	return !ERR;
}

// LIFO ����� ���Ѵ뿡�� ���� �ð��� ���� ������ ���� �ð��� �ݺ���ϰ� �켱���� �ο�
int schedule_LIFO(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	process[pid].priority = INFINITY - cur_time;
	return !ERR;
}

// HRRN ����� ��Ű�ǵ���� ������ �����ؼ� �켱���� �ο�
// �ٸ�, waiting time�� Ŀ���� �켱������ �������� �ϹǷ�, ���Ѵ뿡�� ���� �켱������ �ο�
int schedule_HRRN(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	process[pid].priority = INFINITY - 
		(float)(process[pid].waiting_time + process[pid].cpu_burst_time) / process[pid].cpu_burst_time;
	return !ERR;
}

// SIF (shortest IO fist) ����� io remain �ð��� ª������ �� ���� �켱������ �ο�
int schedule_SIF(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	process[pid].priority = process[pid].io_remaining_time;
	return !ERR;
}

// LOTT ����� priority�� �ο��ϴ� ���� �ǹ̰� ����
int schedule_LOTT(int pid) {
	if (ready_queue->process[pid] == NOT_EXIST) {
		return ERR;
	}
	return !ERR;
}

// ready queue���� �켱������ ���� ���� process�� id�� return. ������ NOT_EXIST return
int highest_priority_process() {
	int highest_priority_process = NOT_EXIST;
	// LOTTERY ����� ���� �ִ� ready queue�� �ִ� ���μ��� �� ������ ������ ���� ��÷�Ͽ� ����
	if (schedule->method == LOTT) {
		int lottery[TOTAL_PID];
		int total_lottery = 0;
		for (int pid = 0; pid < TOTAL_PID; pid++) {
			lottery[pid] = 0;
			if (ready_queue->process[pid] == EXIST) {
				total_lottery += process[pid].lottery;
				lottery[pid] = process[pid].lottery;
			}
		}
		// total lottery�� 0�̸� NOT_EXIST return
		if (total_lottery == 0) {
			return highest_priority_process;
		}
		// ��÷
		int target = rand() % total_lottery + 1;
		//printf("%d\n", target);
		// ��÷�� ��ȣ ��ġ ã��
		int cnt_lottery = 0;
		for (int pid = 0; pid < TOTAL_PID; pid++) {
			if (ready_queue->process[pid] == EXIST) {
				cnt_lottery += process[pid].lottery;
				// �ش� pid�� ���� ���μ������� ��÷��
				if (cnt_lottery >= total_lottery) {
					highest_priority_process = pid;
					//printf("%d\n", highest_priority_process);
				}
			}
		}
	}
	// LOTT ��� �̿��� ����̶��
	else {
		int min = INFINITY;
		for (int pid = 0; pid < TOTAL_PID; pid++) {
			// ready queue�� �����ϴ� process �� ���� priority ���� ����(�켱������ ����) process�� ã��
			// �� ��, ���� priority��� pid�� �������� �켱�� �ο�
			if (ready_queue->process[pid] == EXIST && min > process[pid].priority) {
				min = process[pid].priority;
				highest_priority_process = pid;
			}
		}
	}
	// -1�� return �ȴٸ� ready_queue�� ��������� �ǹ�
	return highest_priority_process;
}

Schedule* init_schedule(int method, int preemptive, int time_quantum, int aging) {
	Schedule* s = malloc(sizeof(Schedule));
	s->method = method;
	s->preemptive = preemptive;
	s->aging = aging;
	s->time_quantum = time_quantum;
	// scheduling method�� ���� �켱���� �ο� ����� �޶���
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
	case LIFO:
		s->give_priority = schedule_LIFO;
	case HRRN:
		s->give_priority = schedule_HRRN;
	case SIF:
		s->give_priority = schedule_SIF;
	case LOTT:
		s->give_priority = schedule_LOTT;
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
			// �ð��� 1���� �����ϹǷ� 1�� ���� ���� ���
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
	printf("\n\n");

}

void evaluate_additional(Schedule* schedule_manager[]) {
	printf("Evaluation Additional.\n\n");
	printf("%19s %8s %8s %8s %8s\n", "Scheduling Method|",
		"LIFO", "HRRN", "SIF", "LOTT");
	printf("%19s ", "Average Waiting|");
	for (int i = 0; i < 4; i++)
		printf("%8.2f ", schedule_manager[i]->avg_waiting_time);
	printf("\n");
	printf("%19s ", "Average Turnaround|");
	for (int i = 0; i < 4; i++)
		printf("%8.2f ", schedule_manager[i]->avg_turnaround_time);
	printf("\n");
	printf("%19s ", "Average Response|");
	for (int i = 0; i < 4; i++)
		printf("%8.2f ", schedule_manager[i]->avg_response_time);
	printf("\n\n");

}