#include <stdlib.h>

#include "queue.h"
#include "macro.h"
#include "lifecycle.h"
#include "process.h"

extern int cur_time;
extern Process process[TOTAL_PID];

// queue 초기화, 해당 queue의 process 정보를 NOT_EXIST로 저장
Queue* init_queue() {
	Queue* q = malloc(sizeof(Queue) + sizeof(int) * TOTAL_PID);
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		q->length = TOTAL_PID;
		q->process[pid] = NOT_EXIST;
	}
	return q;
}

// queue에 process [pid]를 삽입, 이미 존재하는 pid라면 에러 return
int insert(Queue* q, int pid) {
	if (q->process[pid] == EXIST)
		return ERR;
	// process[pid]를 EXIST로 바꾸고, queue 내부에 해당 process가 존재하는 것으로 간주
	q->process[pid] = EXIST;
	// process의 최근 큐 삽입 시간 갱신
	process[pid].insert_time = cur_time;
	return !ERR;
}

// queue에서 process [pid]를 제거, 존재하지 않는 pid라면 에러 return
int extract(Queue * q, int pid) {
	if (q->process[pid] == NOT_EXIST)
		return ERR;
	// process[pid]를 NOT_EXIST로 바꾸고, queue 내부에 해당 process가 존재하지 않는 것으로 간주
	q->process[pid] = NOT_EXIST;
	// process의 최근 큐 삽입 시간을 지워줌
	process[pid].insert_time = NOT_EXIST;
	return !ERR;
}