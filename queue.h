#ifndef queue_h
#define queue_h

typedef struct _Queue {
	int length; // gcc �����Ϸ��� flexible array�� ����ü�� �ܵ������ ������� ���ϰ��ؼ� �߰��� ����
	int process[];
}Queue;

Queue* init_queue();
int insert(Queue*, int);
int extract(Queue*, int);

#endif
