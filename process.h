#ifndef process_h
#define process_h

typedef struct _Process {
	int pid;
	int arrival_time;
	int cpu_burst_time;
	int cpu_remaining_time;
	int io_burst_time;
	int io_remaining_time;
	int own_priority;
	float priority;	// HRRN 방식의 우선순위 부여 때문에 float형으로 변경
	int aging_priority;
	int waiting_time;
	int turnaround_time;
	int response_time;
	int insert_time; // 특정 큐에 insert 되는 시간을 나타냄 -> 동일한 priority에 대해서는 선입선출하기 위해 사용
	int is_terminal;
	int lottery;
}Process;

Process* create_process(int);
int generate_io();

#endif
