#include <stdlib.h>

#include "queue.h"
#include "macro.h"
#include "lifecycle.h"
#include "process.h"

extern int cur_time;
extern Process process[TOTAL_PID];

// queue �ʱ�ȭ, �ش� queue�� process ������ NOT_EXIST�� ����
Queue* init_queue() {
	Queue* q = malloc(sizeof(Queue) + sizeof(int) * TOTAL_PID);
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		q->length = TOTAL_PID;
		q->process[pid] = NOT_EXIST;
	}
	return q;
}

// queue�� process [pid]�� ����, �̹� �����ϴ� pid��� ���� return
int insert(Queue* q, int pid) {
	if (q->process[pid] == EXIST)
		return ERR;
	// process[pid]�� EXIST�� �ٲٰ�, queue ���ο� �ش� process�� �����ϴ� ������ ����
	q->process[pid] = EXIST;
	// process�� �ֱ� ť ���� �ð� ����
	process[pid].insert_time = cur_time;
	return !ERR;
}

// queue���� process [pid]�� ����, �������� �ʴ� pid��� ���� return
int extract(Queue * q, int pid) {
	if (q->process[pid] == NOT_EXIST)
		return ERR;
	// process[pid]�� NOT_EXIST�� �ٲٰ�, queue ���ο� �ش� process�� �������� �ʴ� ������ ����
	q->process[pid] = NOT_EXIST;
	// process�� �ֱ� ť ���� �ð��� ������
	process[pid].insert_time = NOT_EXIST;
	return !ERR;
}