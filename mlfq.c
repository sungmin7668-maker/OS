// think about .... : 최소단위가 1초인걸 좀 더 구현해보자 (MEMO 참고)

#include <stdio.h>
#include <stdlib.h>

// Structure to represent a process
typedef struct Process {
    int pid;
    int arrival_time;
    int run_time;
    int remaining_run_time; 
    int start_time;
    int completion_time; //실행 끝
    int turnaround_time;
    int response_time;
    int waiting_time;
    int queue_level;
    int IO_start_time;
    int IO_run_time;
    struct Process* next;
} Process;

// Structure to represent a queue
typedef struct Queue {
    Process *head, *tail;
    int time_quantum;
} Queue;

typedef struct IO{
    int pid;
    int remaining_run_time;
    Queue* return_queue;
} IO;

// Function to create a new process  from file
Process* createProcess(int pid, int arrival, int run, int IO_start_time, int IO_run_time) {
    Process* p = (Process*)malloc(sizeof(Process));
    p->pid = pid;
    p->arrival_time = arrival;
    p->run_time = run;
    p->remaining_run_time = run;
    p->start_time = 0;
    p->completion_time = 0;
    p->turnaround_time = 0;
    p->response_time = 0;
    p->waiting_time = 0;
    p->queue_level = 1; // Start in the highest priority queue
    p->IO_start_time=IO_start_time;
    p->IO_run_time=IO_run_time;
    p->next = NULL;
    return p;
}

// Function to create a new queue
Queue* createQueue(int quantum) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->head = q->tail = NULL;
    q->time_quantum = quantum;
    return q;
}

IO* createIO(int pid, int IO_run_time, Queue* return_queue){
    IO* io=(IO*)malloc(sizeof(IO));
    io->pid=pid;
    io->remaining_run_time=IO_run_time;
    io->return_queue=return_queue;
    return io;
}

// Function to add a process to a queue
void enqueue(Queue* q, Process* p) {
    if (q->tail == NULL) {
        q->head = q->tail = p;
        return;
    }
    q->tail->next = p;
    q->tail = p;
    p->next = NULL; // Ensure the new tail points to NULL
}

// Function to remove a process from the front of a queue
Process* dequeue(Queue* q) {
    if (q->head == NULL) return NULL;
    Process* p = q->head;
    q->head = q->head->next;
    if (q->head == NULL) q->tail = NULL;
    p->next = NULL;
    return p;
}

int compare_process(const void *a, const void *b){ // const : 값들을 변경하지 못하도록
    Process* arg1=*(Process* const*)a;
    Process* arg2=*(Process* const*)b;
    if (arg1->arrival_time < arg2->arrival_time) return -1;
    else if (arg1->arrival_time > arg2->arrival_time) return 1;
    else {
        if(arg1->pid < arg2->pid) return -1;
        else return 1;
    }
}

// Main MLFQ simulation function
int mlfq_scheduling(Process* processes[], int n) {
    // Define the three queues with their time quanta
    Queue* q1 = createQueue(10);
    Queue* q2 = createQueue(10);
    Queue* q3 = createQueue(20); // FCFS (no specific quantum, runs until completion or preemption)

    IO** io=(IO**)malloc(n*sizeof(IO*)); // 'IO* io[]=(IO**)~~~' is wrong?
    int io_top = 0;
    int io_tail = 0;

    int current_time = 0;
    int completed_processes = 0;
    int i;

    // process sort along arrival_time
    qsort(processes,n,sizeof(Process*),compare_process);

    // Initially add all processes to the highest priority queue (Q1) at their arrival time
    for (i = 0; i < n; i++) {
        enqueue(q1, processes[i]);
    }
    
    while (completed_processes < n) {
        Process* current_process = NULL;
        Queue* current_queue = NULL;

        // Find the highest priority non-empty queue
        if (q1->head != NULL) {
            current_queue = q1;
        } else if (q2->head != NULL) {
            current_queue = q2;
        } else if (q3->head != NULL) {
            current_queue = q3;
        } else {
            // No ready process, increment time
            current_time++;
            continue;
        }

        current_process = dequeue(current_queue);

        // process's start time to calculate response time
        if(current_process->run_time==current_process->remaining_run_time){
            current_process->start_time=current_time;
        }
        // Determine the execution time
        int exec_time;
        
        // Round Robin queue, run for min of quantum or remaining time
        exec_time = (current_process->remaining_run_time < current_queue->time_quantum) ?
                    current_process->remaining_run_time : current_queue->time_quantum;
    
        if(current_process->IO_start_time!=0){
            exec_time=current_process->IO_start_time;
            
            // same return queue
            if(current_queue->time_quantum > exec_time)
                io[io_tail]=createIO(current_process->pid, current_process->IO_run_time, current_queue);
            // lower
            else{
                if(current_queue==q1)
                    io[io_tail]=createIO(current_process->pid, current_process->IO_run_time,q2);
                else
                    io[io_tail]=createIO(current_process->pid, current_process->IO_run_time,q3);
            }
            //io_tail=io_tail+1;
        }

        // Simulate execution
        for(int exec=1;exec<=exec_time;exec++){

            //IO 작동 (1초마다 작동)
            for(int i=io_top;i<io_tail;i++){

                io[i]->remaining_run_time-=1;

                // IO end, return to queue
                if(io[i]->remaining_run_time==0){
                    for(int j=0;j<n;j++){
                        if(processes[j]->pid==io[i]->pid){
                            processes[j]->IO_start_time=0;
                            enqueue(io[i]->return_queue,processes[j]);
                            break;
                        }
                    }
                    free(io[i]);
                    io_top++;
                }
            }

            current_process->remaining_run_time -= 1;
            current_time += 1;
            //boost
            if((current_time%50)==0){
            while(q2->tail!=NULL)
                enqueue(q1,dequeue(q2));
            while(q3->tail!=NULL)
                enqueue(q1,dequeue(q3));
            }
        }

        if(current_process->IO_start_time!=0){ // current process 가 exec 만큼 실행되고 나서 현재 IO가 실행되도록
            io_tail=io_tail+1;
        }

        // Check if process completed
        if (current_process->remaining_run_time == 0) {
            current_process->completion_time = current_time;
            current_process->turnaround_time = current_process->completion_time - current_process->arrival_time;
            current_process->waiting_time = current_process->turnaround_time - current_process->run_time;
            current_process->response_time = current_process->start_time - current_process->arrival_time;
            completed_processes++;
            free(current_process); // Free the memory after completion
        } else {
            // Process not completed, demote to the next queue

            if(current_process->IO_start_time!=0) continue;
            else{
                if (current_queue == q1) {
                    current_process->queue_level = 2;
                    enqueue(q2, current_process);
                } else if (current_queue == q2) {
                    current_process->queue_level = 3;
                    enqueue(q3, current_process);
                } else {
                    // Stays in Q3 (FCFS)
                    enqueue(q3, current_process);
                }
            }
            
        }
    }
    free(q1);free(q2);free(q3);
    free(io);
    // Print results (this basic simulation prints results as processes finish)
    // A better approach would store completed processes in an array for structured output.
    printf("All processes completed.\n");

    return current_time;
}

int main() {
    FILE* fp=fopen("trace1.txt","r");
    if(fp==NULL){
        perror("파일 열기 실패");
        return 1;
    }
    
    int PID,arrival_time,run_time,IO_start_time,IO_run_time;
    int n=0;
    Process* processes[100]; //프로세스 최대 100개
    while(fscanf(fp,"%d %d %d %d %d",&PID,&arrival_time,&run_time,&IO_start_time,&IO_run_time)==5){
        processes[n]=createProcess(PID, arrival_time, run_time, IO_start_time, IO_run_time);
        n=n+1;
    }
    fclose(fp);

    int final_completion_time = mlfq_scheduling(processes, n);

    // print 
    printf("PID | Turnaround time | response time\n");
    for (int p=0;p<n;p++){
        printf("%-3d | %-15d | %-13d\n",processes[p]->pid,processes[p]->turnaround_time,processes[p]->response_time);
    }

    printf("final_completion_time : %d\n",final_completion_time);

    // Note: A real implementation for comprehensive results (Avg Turnaround/Waiting Time)
    // needs a list of completed processes for final calculation.

    return 0;
}
