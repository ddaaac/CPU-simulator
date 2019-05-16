#include <stdio.h>
#include <stdlib.h>

#include "process.h"
#include "queue.h"
#include "schedule.h"
#include "lifecycle.h"
#include "macro.h"

Process process[TOTAL_PID];	// �� TOTAL_PID���� process

Queue* new_queue;		// ������ process �� ���� arrival time�� ���� ���� process���� queue
Queue* ready_queue;		// admit�� �޸𸮿� �ö���ִ� process���� queue
Queue* waiting_queue;	// I/O �۾��� �߻����� ���� ������� process���� queue

Schedule* schedule;	// schedule ��Ŀ� ���� ������ ��� �ִ� ����ü

int running_pid;// ���� running �ϰ� �ִ� process�� id

int cur_time;	// ���� �ð��� ��Ÿ���� ��������

int gantt_chart[MAX_CYCLE]; // ���� ������ ��Ŀ����� ��Ʈ��Ʈ�� �����ϴ� ����


// process �ʱ�ȭ �Լ�, main �Լ����� �ѹ��� �����Ͽ� ������ process�� ���� evaluate�� �����ؾ���
void init_process() {
	printf("Processes info.\n");
	printf("%6s %6s %6s %6s %6s\n", "pid", "ariv", "cpu", "io", "pri");
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		process[pid] = *create_process(pid);
		printf("%6d %6d %6d %6d %6d\n", process[pid].pid, process[pid].arrival_time, process[pid].cpu_burst_time, process[pid].io_burst_time, process[pid].priority);
	}
	printf("\n");
}

// �� queue���� �ʱ�ȭ�ϰ�, process�� new_queue�� �Ҵ�, schedule ��� ���� �� ��Ÿ ������ �ʱ�ȭ
void init(int method, int preemptive, int time_quantum, int aging) {
	// new, ready, waiting queue �ʱ�ȭ
	new_queue = init_queue();
	ready_queue = init_queue();
	waiting_queue = init_queue();
	// �ð� 1�� �ʱ�ȭ
	cur_time = 1;
	// process���� new_queue�� �Ҵ�
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		int err = insert(new_queue, pid);
		if (err)
			printf("�̹� new queue�� ��ϵ� process�Դϴ�.\n");
	}
	// ���� �������� process �ʱ�ȭ
	running_pid = NOT_EXIST;
	// ������ ��� �ʱ�ȭ
	schedule = init_schedule(method, preemptive, time_quantum, aging);
	// ��Ʈ ��Ʈ �ʱ�ȭ
	for (int i = 0; i < MAX_CYCLE; i++)
		gantt_chart[i] = NOT_EXIST;
}

int admit() {
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		// ���μ����� arrival time�� �ư�, ���μ����� new_queue�������Ѵٸ�
		if (cur_time == process[pid].arrival_time && new_queue->process[pid] == EXIST) {
			// new_queue���� ���μ��� pid�� �����ϰ�,
			int err = extract(new_queue, pid);
			if (err) {// ���μ����� new queue�� �������� �ʴ´ٸ� ����
				printf("process%d is not existed in new queue\n", pid);
				return ERR;
			}
			// ready_queue�� ���μ��� pid�� ����
			err = insert(ready_queue, pid);
			if (err) { // ���μ����� ready queue�� �̹� �����Ѵٸ� ����
				printf("process%d is already existed in ready queue\n", pid);
				return ERR;
			}
			// ready queue�� �����ϸ� �켱���� �ο�
			err = schedule->give_priority(pid);
			if (err) { // ���μ����� ready queue�� �������� �ʴ´ٸ� ����
				printf("process%d is not in ready queue\nNo need to schedule.\n", pid);
				return ERR;
			}
			//printf("%d is move new_queue from ready queue\n", pid);
		}
	}
	return !ERR;
}


int pop() {
	// ready queue���� ���� �켱������ ���� process�� pid�� Ȯ��
	int principal_process = highest_priority_process();
	if (principal_process == NOT_EXIST) {// ���ٸ� ready queue�� �������
		//printf("ready queue -> running queue : None\n");
		return !ERR;
	}
	// ���� running ���� process�� ���ų� ������Ŀ��� �켱������ ���ٸ� �ٷ� process �̵�
	// �켱������ ���ڰ� �������� ����
	// priority�� ������ ��������̶� ȿ������ ���� �������� ����
	if (running_pid == NOT_EXIST || (schedule->preemptive && process[principal_process].priority < process[running_pid].priority)) {
		// ���� �켱������ ���� process�� ready queue���� ����
		int err = extract(ready_queue, principal_process);
		if (err) { // ���μ����� ready queue�� �������� �ʴ´ٸ� ����
			printf("process%d is not existed in ready queue\n", principal_process);
			return ERR;
		}
		// ������Ŀ��� �з��� process�� ready queue�� interrupt ��
		if (running_pid != NOT_EXIST) {
			// interrupt �������� ���� �߻� �� ERR return
			//printf("Preemptive : running queue -> ready queue : %d\n", running_pid);
			err = interrupt();
			if (err) {
				return ERR;
			}
		}
		// �������� process�� ���
		running_pid = principal_process;
		process[running_pid].insert_time = cur_time; // running ������ �ð� ����

		//if (principal_process != NOT_EXIST)
			//printf("CPU Scheduling : ready queue -> running queue : %d\n", principal_process);

		return !ERR;
	}

	// process�� cpu�� �̵���Ű�� ���ϴ� ��Ȳ�̶�� ��Ȳ ���
	else {
		//printf("Can't Preemptive : ready queue -/> running queue : %d -/> %d\n", principal_process, running_pid);
		return !ERR;
	}
}

// cpu�� process�� ������
int running() {
	// gantt chart ������Ʈ
	gantt_chart[cur_time - 1] = running_pid;

	// running ���� process�� ���ٸ� ��Ȳ ���
	if (running_pid == NOT_EXIST) {
		//printf("None of process is now running.\n");
		return !ERR;
	}
	// CPU burst time 1 ����
	process[running_pid].cpu_remaining_time--;
	// turnaround time 1 ����
	process[running_pid].turnaround_time++;
	// response time (ready queue���� ó�� running ���°� �� �������� �ð�) ����
	if (process[running_pid].response_time == NOT_EXIST)
		// �� ��, �� �������� waiting time�� �� response time�� ��
		process[running_pid].response_time = process[running_pid].waiting_time;
	// �̶� SJF �����층 ����� ����Ѵٸ� �켱������ ���� �Ѵܰ� ������(������ ready queue�� ���Ե� ��, priority�� ������Ʈ �ǹǷ� Preemptive ��Ŀ����� �ǹ�����)
	if (schedule->method == SJF) {
		// �� ��, priority�� 0���� �۾��� �� ���� 
		if (process[running_pid].priority > 0)
			process[running_pid].priority--;
	}
	return !ERR;
}

int interrupt() {
	// ready queue�� �ش� process ����
	int err = insert(ready_queue, running_pid);
	if (err) { // ���μ����� ready queue�� �̹� �����Ѵٸ� ����
		printf("process%d is already existed in ready queue\n", running_pid);
		return ERR;
	}
	err = schedule->give_priority(running_pid); // �켱���� �ο�
	if (err) { // ���μ����� ready queue�� �������� �ʴ´ٸ� ����
		printf("process%d is not existed in ready queue\nNo need to schedule.\n", running_pid);
		return ERR;
	}
	// running ���� process�� �����
	running_pid = NOT_EXIST;
	return !ERR;
}

int io_occur() {
	// waiting queue�� �ش� process ����
	int err = insert(waiting_queue, running_pid);
	if (err) { // ���μ����� waiting queue�� �̹� �����Ѵٸ� ����
		printf("process%d is already existed in waiting queue\n", running_pid);
		return ERR;
	}
	// running ���� process�� �����
	running_pid = NOT_EXIST;
	return !ERR;
}

int terminate() {
	// �̹� terminate �� process��� ����
	if (process[running_pid].is_terminal) {
		printf("process%d alreday terminated.\n", running_pid);
		return ERR;
	}
	// terminal boolean�� true�� �ٲ�
	process[running_pid].is_terminal = 1;
	// running ���� process�� �����
	running_pid = NOT_EXIST;
	return !ERR;
}

// running queue�� �ִ� process���� ���� �༱�� ����
int after_running() {
	// running ���� process�� ���ٸ� ��Ȳ ��� ����(error ��Ȳ X)
	if (running_pid == NOT_EXIST) {
		//printf("None of process is now running.\n");
		return !ERR;
	}
	// cpu remain time�� 0�̶�� terminate ����
	if (process[running_pid].cpu_remaining_time == 0) {
		//printf("running queue -> terminal : %d\n", running_pid);
		int err = terminate();
		if (err)
			return ERR;
	}
	// �־��� time quantum�� ��� �Һ��ߴٸ� interrupt ����
	else if ((cur_time - process[running_pid].insert_time + 1) == schedule->time_quantum) {
		//printf("Use all time quantum : running queue -> ready queue : %d\n", running_pid);
		int err = interrupt();
		if (err)
			return ERR;
	}
	// IO event�� �߻��ߴٸ� io �߻� ����
	else if (process[running_pid].io_burst_time > 0 && generate_io()) {
		printf("IO occur : Process[%d] after time %d\n", running_pid, cur_time);
		int err = io_occur();
		if (err)
			return ERR;
	}
	return !ERR;
}

int waiting() {
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		// ���� waiting queue�� �����ϴ� process���
		if (waiting_queue->process[pid] == EXIST) {
			//printf("%d is waiting in waiting queue\n", pid);
			// io burst time�� 1��ŭ �Ҹ��ϰ�, turnaround time�� 1��ŭ ����
			process[pid].io_remaining_time--;
			process[pid].turnaround_time++;
			// �־��� io burst time�� ��� �Ҹ��Ͽ��ٸ�
			if (process[pid].io_remaining_time == 0) {
				//printf("IO terminate : waiting queue -> ready queue : %d\n", pid);
				// waiting queue���� �ش� precess ���� ��,
				int err = extract(waiting_queue, pid);
				if (err) { // ���μ����� waiting queue�� �������� �ʴ´ٸ� ����
					printf("process%d is not existed in waiting queue\n", pid);
					return ERR;
				}
				// ready queue�� ���� ��,
				err = insert(ready_queue, pid);
				if (err) { // ���μ����� ready queue�� �̹� �����Ѵٸ� ����
					printf("process%d is already existed in ready queue\n", pid);
					return ERR;
				}
				// schedule�� ���� �켱���� �ο�
				err = schedule->give_priority(pid);
				if (err) { // ���μ����� ready queue�� �������� �ʴ´ٸ� ����
					printf("process%d is not in ready queue\nNo need to schedule.\n", pid);
					return ERR;
				}
				// io remain time �ʱ�ȭ
				process[pid].io_remaining_time = process[pid].io_burst_time;
			}
		}
	}
	return !ERR;
}

int terminal() {
	// process�� ��� ����ƴ��� Ȯ���ϴ� ���� if_terminal
	int if_terminal = 1;
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		// �ϳ��� process�� ������� �ʾҴٸ� if_terminal�� 0���� �ϰ� break
		if (process[pid].is_terminal == 0) {
			if_terminal = 0;
			break;
		}
	}
	// ����ƴٸ� TERMINAL, ������� �ʾҴٸ� ���ݱ��� error�� �����Ƿ� !ERR return
	return if_terminal ? TERMINAL : !ERR;
}

int one_cycle() {
	// admit ����
	int err = admit();
	if (err)	// admit ����
		return ERR;
	// pop ����
	err = pop();
	if (err) // pop ����
		return ERR;
	// ready queue�� �ִ� process�� ���Ͽ� waiting time�� turnaround time�� 1�� ����
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		if (ready_queue->process[pid] == EXIST) {
			process[pid].waiting_time++;
			process[pid].turnaround_time++;
			// scheduling�� aging ����� ����Ѵٸ�
			if (schedule->aging) {
				// 3�ʿ� �ѹ���
				if (process[pid].insert_time < cur_time && ((cur_time - process[pid].insert_time) % 3 == 0)) {
					// priority�� �Ѵܰ� �÷���, �� �� priority�� 0���� ���� �� ����
					if (process[pid].priority > 0) {
						//printf("Aging : Process[%d] gains one more priority\n", pid);
						process[pid].priority--;
						// aging���� priority�� �� �ܰ� �ö����� ����
						process[pid].aging_priority++;
					}
				}
			}
		}
	}
	// running ����
	err = running();
	if (err)	// running ����
		return ERR;
	// waiting ����
	err = waiting();
	if (err)	// waiting ����
		return ERR;
	// running ���� process�� ���� ó�� ����
	err = after_running();
	if (err)
		return ERR;
	// terminal ���� return
	return terminal();

}

Schedule* life_cycle() {
	while (1) {
		int status = one_cycle();
		float sum_turnaround_time = 0, sum_waiting_time = 0, sum_response_time = 0;
		switch (status)
		{
		// ���� ��Ȳ
		case TERMINAL:
			// ��� process�� ����ƴٸ� ���� �˰��򿡼� ����� �� �ְ� process ���� �ʱ�ȭ
			// pid, arrival time, �� burst time �� own_priority�� �״�� ����
			// ����, ��� turnaround time�� waiting time ���
			for (int pid = 0; pid < TOTAL_PID; pid++) {
				sum_turnaround_time += process[pid].turnaround_time;
				sum_waiting_time += process[pid].waiting_time;
				sum_response_time += process[pid].response_time;
				process[pid].cpu_remaining_time = process[pid].cpu_burst_time;
				process[pid].io_remaining_time = process[pid].io_burst_time;
				process[pid].priority = process[pid].own_priority;
				process[pid].aging_priority = 0;
				process[pid].waiting_time = 0;
				process[pid].turnaround_time = 0;
				process[pid].response_time = NOT_EXIST;
				process[pid].insert_time = NOT_EXIST;
				process[pid].is_terminal = 0;
			}
			schedule->avg_turnaround_time = sum_turnaround_time / TOTAL_PID;
			schedule->avg_waiting_time = sum_waiting_time / TOTAL_PID;
			schedule->avg_response_time = sum_response_time / TOTAL_PID;
			// ���� �� �ҿ�ð��� ��������
			schedule->total_time = cur_time;
			//printf("�ش� lifecycle�� ����Ǿ����ϴ�.\n\n");
			return schedule;
		// ���� ��Ȳ
		case ERR:
			printf("������ �߻��Ͽ����ϴ�. �ڵ带 Ȯ�����ּ���.\n\n");
			schedule->err = ERR;
			return schedule;
		// life cycle ��� ����
		default:
			//printf("%d��° cycle�� ��� ����Ǿ����ϴ�.\n\n", cur_time);
			if (cur_time++ > MAX_CYCLE) {
				printf("�ִ� CYCLE ���� �ʰ��Ͽ����ϴ�. ���ѷ��� ���θ� Ȯ�����ּ���.\n\n");
				schedule->err = ERR;
				return schedule;
			}
			break;
		}
	}
}