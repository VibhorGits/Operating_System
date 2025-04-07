#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <string>

// Struct representing a task
struct Task {
    int id;
    std::string command;
    int priority; // 1 = High (RR), 2 = Low (FCFS)
};

// Scheduler core functions
void add_task(int id, const std::string& command, int priority);
void jschedule_command(const std::string& filename, int priority);
void jschedulexecute_command();
void multi_level_schedule();

// Queue utilities
void round_robin_schedule();
void fcfs_schedule();

// Task utilities
void execute_task_parallel(const Task& task);

// Enhancements
void print_scheduled_tasks();
void save_queues_to_file(const std::string& filename);
void cancel_task(int id);
void modify_task(int id, const std::string& new_command);

#endif // SCHEDULER_H
