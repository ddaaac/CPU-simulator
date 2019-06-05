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
	float priority;	// HRRN ����� �켱���� �ο� ������ float������ ����
	int aging_priority;
	int waiting_time;
	int turnaround_time;
	int response_time;
	int insert_time; // Ư�� ť�� insert �Ǵ� �ð��� ��Ÿ�� -> ������ priority�� ���ؼ��� ���Լ����ϱ� ���� ���
	int is_terminal;
	int lottery;
}Process;

Process* create_process(int);
int generate_io();

#endif
