#include "scheduler.h"
#include <iostream>
#include <queue>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <thread>
#include <vector>
#include "commands.h"

static std::queue<Task> rr_queue;
static std::queue<Task> fcfs_queue;
static int task_id_counter = 1;

// -------------------------
// Task Management
// -------------------------

/**
 * @brief Adds a task to the appropriate queue based on priority.
 *
 * @param id       Task ID.
 * @param command  Command string to execute.
 * @param priority Priority level (1 = RR, 2 = FCFS).
 */
void add_task(int id, const std::string &command, int priority)
{
    Task task = {id, command, priority};
    if (priority == 1)
    {
        rr_queue.push(task);
    }
    else
    {
        fcfs_queue.push(task);
    }
}

/**
 * @brief Reads a script file and schedules each line as a separate task.
 *
 * @param filename Path to the script file.
 * @param priority Priority for the scheduled tasks.
 */
void jschedule_command(const std::string &filename, int priority)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    // Store the filename only, not the file content
    add_task(task_id_counter++, filename, priority);

    file.close();
    std::cout << "[Script file " << filename << " scheduled as a single task]\n";
}

/**
 * @brief Executes a task in a detached thread.
 *
 * @param task The Task to be executed.
 */
void execute_task_parallel(const Task &task)
{
    std::thread([task]()
                { execute_task(task); })
        .detach();
}

// -------------------------
// Scheduling Algorithms
// -------------------------

/**
 * @brief Executes tasks in the Round Robin queue (time-sliced).
 */
void round_robin_schedule()
{
    std::cout << "== Round Robin Queue (High Priority) ==\n";
    while (!rr_queue.empty())
    {
        Task t = rr_queue.front();
        rr_queue.pop();

        execute_task(t);
    }
}

/**
 * @brief Executes tasks in the FCFS queue (First Come First Serve).
 */
void fcfs_schedule()
{
    std::cout << "== FCFS Queue (Low Priority) ==\n";
    std::vector<std::thread> threads;
    while (!fcfs_queue.empty())
    {
        Task t = fcfs_queue.front();
        fcfs_queue.pop();
        threads.emplace_back([t]()
                             { execute_task(t); });
    }
    for (auto &th : threads)
        th.join();
}

/**
 * @brief Executes both Round Robin and FCFS queues.
 */
void multi_level_schedule()
{
    std::cout << "== Multi-Level Queue Scheduler ==\n";
    round_robin_schedule(); // Serve high priority first
    fcfs_schedule();        // Then serve low priority
}

// -------------------------
// Task Management Utilities
// -------------------------

/**
 * @brief Displays the currently scheduled tasks in both queues.
 */
void print_scheduled_tasks()
{
    std::cout << "Scheduled Tasks (RR - High Priority):\n";
    std::queue<Task> temp = rr_queue;
    while (!temp.empty())
    {
        Task t = temp.front();
        temp.pop();
        std::cout << "[" << t.id << "] " << t.command << "\n";
    }

    std::cout << "\nScheduled Tasks (FCFS - Low Priority):\n";
    temp = fcfs_queue;
    while (!temp.empty())
    {
        Task t = temp.front();
        temp.pop();
        std::cout << "[" << t.id << "] " << t.command << "\n";
    }
}

/**
 * @brief Saves the contents of both queues to a file.
 *
 * @param filename Path to the output file.
 */
void save_queues_to_file(const std::string &filename)
{
    std::ofstream out(filename);
    std::queue<Task> temp = rr_queue;
    while (!temp.empty())
    {
        Task t = temp.front();
        temp.pop();
        out << t.id << "," << t.priority << "," << t.command << "\n";
    }
    temp = fcfs_queue;
    while (!temp.empty())
    {
        Task t = temp.front();
        temp.pop();
        out << t.id << "," << t.priority << "," << t.command << "\n";
    }
    out.close();
}

/**
 * @brief Removes a task from both queues by ID.
 *
 * @param id Task ID to remove.
 */
void cancel_task(int id)
{
    std::queue<Task> temp;
    while (!rr_queue.empty())
    {
        Task t = rr_queue.front();
        rr_queue.pop();
        if (t.id != id)
            temp.push(t);
    }
    rr_queue = temp;

    std::queue<Task> temp_fcfs;
    while (!fcfs_queue.empty())
    {
        Task t = fcfs_queue.front();
        fcfs_queue.pop();
        if (t.id != id)
            temp_fcfs.push(t);
    }
    fcfs_queue = temp_fcfs;

    std::cout << "Task " << id << " cancelled.\n";
}

/**
 * @brief Modifies the command of a scheduled task.
 *
 * @param id           Task ID to modify.
 * @param new_command  New command string.
 */
void modify_task(int id, const std::string &new_command)
{
    std::queue<Task> temp;
    while (!rr_queue.empty())
    {
        Task t = rr_queue.front();
        rr_queue.pop();
        if (t.id == id)
            t.command = new_command;
        temp.push(t);
    }
    rr_queue = temp;

    std::queue<Task> temp_fcfs;
    while (!fcfs_queue.empty())
    {
        Task t = fcfs_queue.front();
        fcfs_queue.pop();
        if (t.id == id)
            t.command = new_command;
        temp_fcfs.push(t);
    }
    fcfs_queue = temp_fcfs;

    std::cout << "Task " << id << " modified.\n";
}

// -------------------------
// Scheduler Entry Point
// -------------------------

/**
 * @brief Starts the multi-level scheduler to execute all queued tasks.
 */
void jschedulexecute_command()
{
    multi_level_schedule();
}
