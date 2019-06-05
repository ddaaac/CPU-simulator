#ifndef schedule_h
#define schedule_h

// Schedule 구조체 내부에 flexible integer array로 간트차트 구현 시 다른 메모리 침범하는 현상 발생
typedef struct _Schedule {
	int (*give_priority)(int);
	int preemptive;
	int aging;
	int time_quantum;
	int method;
	float avg_turnaround_time;
	float avg_waiting_time;
	float avg_response_time;
	int err;
	int total_time;
}Schedule;

Schedule* init_schedule(int, int, int, int);
int highest_priority_process();
void print_chart(int[], int);
void evaluate(Schedule* []);
void evaluate_additional(Schedule* []);

#endif