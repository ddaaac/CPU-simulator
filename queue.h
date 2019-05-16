#ifndef queue_h
#define queue_h

typedef struct _Queue {
	int length; // gcc 컴파일러는 flexible array를 구조체의 단독멤버로 사용하지 못하게해서 추가한 변수
	int process[];
}Queue;

Queue* init_queue();
int insert(Queue*, int);
int extract(Queue*, int);

#endif
