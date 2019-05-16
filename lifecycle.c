#include <stdio.h>
#include <stdlib.h>

#include "process.h"
#include "queue.h"
#include "schedule.h"
#include "lifecycle.h"
#include "macro.h"

Process process[TOTAL_PID];	// 총 TOTAL_PID개의 process

Queue* new_queue;		// 생성된 process 중 아직 arrival time이 되지 않은 process들의 queue
Queue* ready_queue;		// admit된 메모리에 올라와있는 process들의 queue
Queue* waiting_queue;	// I/O 작업의 발생으로 인해 대기중인 process들의 queue

Schedule* schedule;	// schedule 방식에 대한 정보를 담고 있는 구조체

int running_pid;// 현재 running 하고 있는 process의 id

int cur_time;	// 현재 시간을 나타내는 전역변수

int gantt_chart[MAX_CYCLE]; // 현재 스케쥴 방식에서의 간트차트를 저장하는 변수


// process 초기화 함수, main 함수에서 한번만 실행하여 동일한 process들 간의 evaluate이 가능해야함
void init_process() {
	printf("Processes info.\n");
	printf("%6s %6s %6s %6s %6s\n", "pid", "ariv", "cpu", "io", "pri");
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		process[pid] = *create_process(pid);
		printf("%6d %6d %6d %6d %6d\n", process[pid].pid, process[pid].arrival_time, process[pid].cpu_burst_time, process[pid].io_burst_time, process[pid].priority);
	}
	printf("\n");
}

// 각 queue들을 초기화하고, process를 new_queue에 할당, schedule 방식 적용 및 기타 변수들 초기화
void init(int method, int preemptive, int time_quantum, int aging) {
	// new, ready, waiting queue 초기화
	new_queue = init_queue();
	ready_queue = init_queue();
	waiting_queue = init_queue();
	// 시간 1로 초기화
	cur_time = 1;
	// process들을 new_queue에 할당
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		int err = insert(new_queue, pid);
		if (err)
			printf("이미 new queue에 등록된 process입니다.\n");
	}
	// 현재 실행중인 process 초기화
	running_pid = NOT_EXIST;
	// 스케쥴 방식 초기화
	schedule = init_schedule(method, preemptive, time_quantum, aging);
	// 간트 차트 초기화
	for (int i = 0; i < MAX_CYCLE; i++)
		gantt_chart[i] = NOT_EXIST;
}

int admit() {
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		// 프로세스의 arrival time이 됐고, 프로세스가 new_queue에존재한다면
		if (cur_time == process[pid].arrival_time && new_queue->process[pid] == EXIST) {
			// new_queue에서 프로세스 pid를 제거하고,
			int err = extract(new_queue, pid);
			if (err) {// 프로세스가 new queue에 존재하지 않는다면 에러
				printf("process%d is not existed in new queue\n", pid);
				return ERR;
			}
			// ready_queue에 프로세스 pid를 삽입
			err = insert(ready_queue, pid);
			if (err) { // 프로세스가 ready queue에 이미 존재한다면 에러
				printf("process%d is already existed in ready queue\n", pid);
				return ERR;
			}
			// ready queue에 삽입하며 우선순위 부여
			err = schedule->give_priority(pid);
			if (err) { // 프로세스가 ready queue에 존재하지 않는다면 에러
				printf("process%d is not in ready queue\nNo need to schedule.\n", pid);
				return ERR;
			}
			//printf("%d is move new_queue from ready queue\n", pid);
		}
	}
	return !ERR;
}


int pop() {
	// ready queue에서 가장 우선순위가 높은 process의 pid를 확인
	int principal_process = highest_priority_process();
	if (principal_process == NOT_EXIST) {// 없다면 ready queue가 비어있음
		//printf("ready queue -> running queue : None\n");
		return !ERR;
	}
	// 현재 running 중인 process가 없거나 선점방식에서 우선순위가 높다면 바로 process 이동
	// 우선순위는 숫자가 작을수록 높음
	// priority가 같으면 선점방식이라도 효율성을 위해 선점하지 않음
	if (running_pid == NOT_EXIST || (schedule->preemptive && process[principal_process].priority < process[running_pid].priority)) {
		// 가장 우선순위가 높은 process를 ready queue에서 제거
		int err = extract(ready_queue, principal_process);
		if (err) { // 프로세스가 ready queue에 존재하지 않는다면 에러
			printf("process%d is not existed in ready queue\n", principal_process);
			return ERR;
		}
		// 선점방식에서 밀려난 process는 ready queue로 interrupt 됨
		if (running_pid != NOT_EXIST) {
			// interrupt 과정에서 에러 발생 시 ERR return
			//printf("Preemptive : running queue -> ready queue : %d\n", running_pid);
			err = interrupt();
			if (err) {
				return ERR;
			}
		}
		// 실행중인 process에 등록
		running_pid = principal_process;
		process[running_pid].insert_time = cur_time; // running 시작한 시간 갱신

		//if (principal_process != NOT_EXIST)
			//printf("CPU Scheduling : ready queue -> running queue : %d\n", principal_process);

		return !ERR;
	}

	// process를 cpu로 이동시키지 못하는 상황이라면 상황 출력
	else {
		//printf("Can't Preemptive : ready queue -/> running queue : %d -/> %d\n", principal_process, running_pid);
		return !ERR;
	}
}

// cpu가 process를 수행함
int running() {
	// gantt chart 업데이트
	gantt_chart[cur_time - 1] = running_pid;

	// running 중인 process가 없다면 상황 출력
	if (running_pid == NOT_EXIST) {
		//printf("None of process is now running.\n");
		return !ERR;
	}
	// CPU burst time 1 감소
	process[running_pid].cpu_remaining_time--;
	// turnaround time 1 증가
	process[running_pid].turnaround_time++;
	// response time (ready queue에서 처음 running 상태가 될 때까지의 시간) 갱신
	if (process[running_pid].response_time == NOT_EXIST)
		// 이 때, 그 떄까지의 waiting time이 곧 response time이 됨
		process[running_pid].response_time = process[running_pid].waiting_time;
	// 이때 SJF 스케쥴링 방식을 사용한다면 우선순위도 같이 한단계 높여줌(어차피 ready queue에 삽입될 때, priority가 업데이트 되므로 Preemptive 방식에서만 의미있음)
	if (schedule->method == SJF) {
		// 이 때, priority는 0보다 작아질 수 없음 
		if (process[running_pid].priority > 0)
			process[running_pid].priority--;
	}
	return !ERR;
}

int interrupt() {
	// ready queue에 해당 process 삽입
	int err = insert(ready_queue, running_pid);
	if (err) { // 프로세스가 ready queue에 이미 존재한다면 에러
		printf("process%d is already existed in ready queue\n", running_pid);
		return ERR;
	}
	err = schedule->give_priority(running_pid); // 우선순위 부여
	if (err) { // 프로세스가 ready queue에 존재하지 않는다면 에러
		printf("process%d is not existed in ready queue\nNo need to schedule.\n", running_pid);
		return ERR;
	}
	// running 중인 process를 비워줌
	running_pid = NOT_EXIST;
	return !ERR;
}

int io_occur() {
	// waiting queue에 해당 process 삽입
	int err = insert(waiting_queue, running_pid);
	if (err) { // 프로세스가 waiting queue에 이미 존재한다면 에러
		printf("process%d is already existed in waiting queue\n", running_pid);
		return ERR;
	}
	// running 중인 process를 비워줌
	running_pid = NOT_EXIST;
	return !ERR;
}

int terminate() {
	// 이미 terminate 된 process라면 에러
	if (process[running_pid].is_terminal) {
		printf("process%d alreday terminated.\n", running_pid);
		return ERR;
	}
	// terminal boolean을 true로 바꿈
	process[running_pid].is_terminal = 1;
	// running 중인 process를 비워줌
	running_pid = NOT_EXIST;
	return !ERR;
}

// running queue에 있는 process들의 다음 행선지 결정
int after_running() {
	// running 중인 process가 없다면 상황 출력 이후(error 상황 X)
	if (running_pid == NOT_EXIST) {
		//printf("None of process is now running.\n");
		return !ERR;
	}
	// cpu remain time이 0이라면 terminate 진행
	if (process[running_pid].cpu_remaining_time == 0) {
		//printf("running queue -> terminal : %d\n", running_pid);
		int err = terminate();
		if (err)
			return ERR;
	}
	// 주어진 time quantum을 모두 소비했다면 interrupt 진행
	else if ((cur_time - process[running_pid].insert_time + 1) == schedule->time_quantum) {
		//printf("Use all time quantum : running queue -> ready queue : %d\n", running_pid);
		int err = interrupt();
		if (err)
			return ERR;
	}
	// IO event가 발생했다면 io 발생 진행
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
		// 만약 waiting queue에 존재하는 process라면
		if (waiting_queue->process[pid] == EXIST) {
			//printf("%d is waiting in waiting queue\n", pid);
			// io burst time을 1만큼 소모하고, turnaround time이 1만큼 증가
			process[pid].io_remaining_time--;
			process[pid].turnaround_time++;
			// 주어진 io burst time을 모두 소모하였다면
			if (process[pid].io_remaining_time == 0) {
				//printf("IO terminate : waiting queue -> ready queue : %d\n", pid);
				// waiting queue에서 해당 precess 제거 후,
				int err = extract(waiting_queue, pid);
				if (err) { // 프로세스가 waiting queue에 존재하지 않는다면 에러
					printf("process%d is not existed in waiting queue\n", pid);
					return ERR;
				}
				// ready queue에 삽입 후,
				err = insert(ready_queue, pid);
				if (err) { // 프로세스가 ready queue에 이미 존재한다면 에러
					printf("process%d is already existed in ready queue\n", pid);
					return ERR;
				}
				// schedule에 따른 우선순위 부여
				err = schedule->give_priority(pid);
				if (err) { // 프로세스가 ready queue에 존재하지 않는다면 에러
					printf("process%d is not in ready queue\nNo need to schedule.\n", pid);
					return ERR;
				}
				// io remain time 초기화
				process[pid].io_remaining_time = process[pid].io_burst_time;
			}
		}
	}
	return !ERR;
}

int terminal() {
	// process가 모두 종료됐는지 확인하는 변수 if_terminal
	int if_terminal = 1;
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		// 하나의 process라도 종료되지 않았다면 if_terminal을 0으로 하고 break
		if (process[pid].is_terminal == 0) {
			if_terminal = 0;
			break;
		}
	}
	// 종료됐다면 TERMINAL, 종료되지 않았다면 지금까지 error가 없으므로 !ERR return
	return if_terminal ? TERMINAL : !ERR;
}

int one_cycle() {
	// admit 실행
	int err = admit();
	if (err)	// admit 에러
		return ERR;
	// pop 실행
	err = pop();
	if (err) // pop 에러
		return ERR;
	// ready queue에 있는 process에 대하여 waiting time과 turnaround time을 1씩 증가
	for (int pid = 0; pid < TOTAL_PID; pid++) {
		if (ready_queue->process[pid] == EXIST) {
			process[pid].waiting_time++;
			process[pid].turnaround_time++;
			// scheduling이 aging 기법을 사용한다면
			if (schedule->aging) {
				// 3초에 한번씩
				if (process[pid].insert_time < cur_time && ((cur_time - process[pid].insert_time) % 3 == 0)) {
					// priority를 한단계 올려줌, 이 때 priority는 0보다 작을 수 없음
					if (process[pid].priority > 0) {
						//printf("Aging : Process[%d] gains one more priority\n", pid);
						process[pid].priority--;
						// aging으로 priority가 몇 단계 올랐는지 저장
						process[pid].aging_priority++;
					}
				}
			}
		}
	}
	// running 실행
	err = running();
	if (err)	// running 에러
		return ERR;
	// waiting 실행
	err = waiting();
	if (err)	// waiting 에러
		return ERR;
	// running 중인 process에 대한 처리 진행
	err = after_running();
	if (err)
		return ERR;
	// terminal 여부 return
	return terminal();

}

Schedule* life_cycle() {
	while (1) {
		int status = one_cycle();
		float sum_turnaround_time = 0, sum_waiting_time = 0, sum_response_time = 0;
		switch (status)
		{
		// 종료 상황
		case TERMINAL:
			// 모든 process가 종료됐다면 다음 알고리즘에서 사용할 수 있게 process 전부 초기화
			// pid, arrival time, 각 burst time 및 own_priority는 그대로 유지
			// 또한, 평균 turnaround time과 waiting time 계산
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
			// 또한 총 소요시간을 저장해줌
			schedule->total_time = cur_time;
			//printf("해당 lifecycle이 종료되었습니다.\n\n");
			return schedule;
		// 에러 상황
		case ERR:
			printf("에러가 발생하였습니다. 코드를 확인해주세요.\n\n");
			schedule->err = ERR;
			return schedule;
		// life cycle 계속 진행
		default:
			//printf("%d번째 cycle이 모두 실행되었습니다.\n\n", cur_time);
			if (cur_time++ > MAX_CYCLE) {
				printf("최대 CYCLE 수를 초과하였습니다. 무한루프 여부를 확인해주세요.\n\n");
				schedule->err = ERR;
				return schedule;
			}
			break;
		}
	}
}