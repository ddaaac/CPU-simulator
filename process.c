#include <time.h>
#include <stdlib.h>

#include "process.h"
#include "macro.h"

// pid, arrival_time, cpu_burst_time, io_burst_time, own_priority는 process 고유의 값으로 변경되면 안됨
Process* create_process(int pid) {
	Process* process = malloc(sizeof(Process));
	process->pid = pid;
	process->arrival_time = rand() % MAX_ARRIVAL_TIME + 1;
	process->cpu_burst_time = rand() % MAX_CPU_BURST_TIME + 1;
	process->cpu_remaining_time = process->cpu_burst_time;
	process->io_burst_time = rand() % MAX_IO_BURST_TIME;
	process->io_remaining_time = process->io_burst_time;
	process->own_priority = rand() % TOTAL_PID;
	process->priority = process->own_priority;
	process->aging_priority = 0;
	process->waiting_time = 0;
	process->turnaround_time = 0;
	process->response_time = NOT_EXIST;
	process->insert_time = NOT_EXIST;
	process->is_terminal = 0;
	return  process;
}

// 1/10 확률로 1을 return
int generate_io() {
	return 0 == rand() % 10;
}