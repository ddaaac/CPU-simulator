#ifndef schedule_h
#define schedule_h

// Schedule ����ü ���ο� flexible integer array�� ��Ʈ��Ʈ ���� �� �ٸ� �޸� ħ���ϴ� ���� �߻�
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