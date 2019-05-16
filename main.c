#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "process.h"
#include "queue.h"
#include "lifecycle.h"
#include "schedule.h"
#include "macro.h"

int main() {
	// 현재 시간으로 난수 초기화
	srand(time(NULL));

	// lifecycle.c의 gantt chart 변수 불러옴
	extern int gantt_chart[MAX_CYCLE];

	// procee 초기화
	init_process();

	// 각 scheduling 별로 정보를 모아 evaluate에서 쓰기 위한 manager
	Schedule* schedule_manager[NUM_SCHEDULING];

	// FCFS
	printf(" - FCFS\n\n");
	init(FCFS, 0, INFINITY, 0);
	Schedule* fcfs = life_cycle();
	if (fcfs->err)
		return -1;
	print_chart(gantt_chart, fcfs->total_time);
	schedule_manager[0] = fcfs;

	// Nonpreemptive SJF
	printf(" - Nonpreemptive SJF\n\n");
	init(SJF, 0, INFINITY, 0);
	Schedule* np_sjf = life_cycle();
	if (np_sjf->err)
		return -1;
	print_chart(gantt_chart, np_sjf->total_time);
	schedule_manager[1] = np_sjf;

	// Preemptive SJF
	printf(" - Preemptive SJF\n\n");
	init(SJF, 1, INFINITY, 0);
	Schedule* p_sjf = life_cycle();
	if (p_sjf->err)
		return -1;
	print_chart(gantt_chart, p_sjf->total_time);
	schedule_manager[2] = p_sjf;

	// NonPreemptive PRIORITY
	printf(" - NonPreemptive PRIORITY\n\n");
	init(PRIORITY, 0, INFINITY, 0);
	Schedule* np_pri = life_cycle();
	if (np_pri->err)
		return -1;
	print_chart(gantt_chart, np_pri->total_time);
	schedule_manager[3] = np_pri;

	// Preemptive PRIORITY
	printf(" - Preemptive PRIORITY\n\n");
	init(PRIORITY, 1, INFINITY, 0);
	Schedule* p_pri = life_cycle();
	if (p_pri->err)
		return -1;
	print_chart(gantt_chart, p_pri->total_time);
	schedule_manager[4] = p_pri;

	// NonPreemptive PRIORITY with aging
	printf(" - NonPreemptive PRIORITY with aging\n\n");
	init(PRIORITY, 0, INFINITY, 1);
	Schedule* np_pri_aging = life_cycle();
	if (np_pri_aging->err)
		return -1;
	print_chart(gantt_chart, np_pri_aging->total_time);
	schedule_manager[5] = np_pri_aging;

	// Preemptive PRIORITY with aging
	printf(" - Preemptive PRIORITY with aging\n\n");
	init(PRIORITY, 1, INFINITY, 1);
	Schedule* p_pri_aging = life_cycle();
	if (p_pri_aging->err)
		return -1;
	print_chart(gantt_chart, p_pri_aging->total_time);
	schedule_manager[6] = p_pri_aging;

	// Round Robin (time quantum = 3)
	printf(" - Round Robin(time quantum=3)\n\n");
	init(RR, 0, 3, 0);
	Schedule* rr3 = life_cycle();
	if (rr3->err)
		return -1;
	print_chart(gantt_chart, rr3->total_time);
	schedule_manager[7] = rr3;

	// Round Robin (time quantum = 5)
	printf(" - Round Robin(time quantum=5)\n\n");
	init(RR, 0, 5, 0);
	Schedule* rr5 = life_cycle();
	if (rr5->err)
		return -1;
	print_chart(gantt_chart, rr5->total_time);
	schedule_manager[8] = rr5;

	evaluate(schedule_manager);

	return 0;
}