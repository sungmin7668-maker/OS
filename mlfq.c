#include <stdio.h>
#include <stdlib.h>

// Structure to represent a process
typedef struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_burst_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int queue_level;
    struct Process* next;
} Process;

// Structure to represent a queue
typedef struct Queue {
    Process *head, *tail;
    int time_quantum;
} Queue;

// Function to create a new process
Process* createProcess(int pid, int arrival, int burst) {
    Process* p = (Process*)malloc(sizeof(Process));
    p->pid = pid;
    p->arrival_time = arrival;
    p->burst_time = burst;
    p->remaining_burst_time = burst;
    p->completion_time = 0;
    p->turnaround_time = 0;
    p->waiting_time = 0;
    p->queue_level = 1; // Start in the highest priority queue
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

// Main MLFQ simulation function
void mlfq_scheduling(Process* processes[], int n) {
    // Define the three queues with their time quanta
    Queue* q1 = createQueue(4);
    Queue* q2 = createQueue(8);
    Queue* q3 = createQueue(-1); // FCFS (no specific quantum, runs until completion or preemption)

    int current_time = 0;
    int completed_processes = 0;
    int i;

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

        // Determine the execution time
        int exec_time;
        if (current_queue->time_quantum == -1) {
            // FCFS queue, run until completion
            exec_time = current_process->remaining_burst_time;
        } else {
            // Round Robin queue, run for min of quantum or remaining time
            exec_time = (current_process->remaining_burst_time < current_queue->time_quantum) ?
                        current_process->remaining_burst_time : current_queue->time_quantum;
        }

        // Simulate execution
        current_process->remaining_burst_time -= exec_time;
        current_time += exec_time;

        // Check if process completed
        if (current_process->remaining_burst_time == 0) {
            current_process->completion_time = current_time;
            current_process->turnaround_time = current_process->completion_time - current_process->arrival_time;
            current_process->waiting_time = current_process->turnaround_time - current_process->burst_time;
            completed_processes++;
            free(current_process); // Free the memory after completion
        } else {
            // Process not completed, demote to the next queue
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

    // Print results (this basic simulation prints results as processes finish)
    // A better approach would store completed processes in an array for structured output.
    printf("All processes completed.\n");
}

int main() {
    // Example processes: PID, Arrival Time, Burst Time
    Process* processes[] = {
        createProcess(1, 0, 10),
        createProcess(2, 1, 5),
        createProcess(3, 2, 8)
    };
    int n = sizeof(processes) / sizeof(processes[0]);

    mlfq_scheduling(processes, n);

    // Note: A real implementation for comprehensive results (Avg Turnaround/Waiting Time)
    // needs a list of completed processes for final calculation.

    return 0;
}
