#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>
#include "scheduler.h"     
#include <sstream>     

void print_help_menu();

void create_file(const char* filename);
void edit_file(const char* filename);
void modify_file(const char* filename);
void rename_file(const char* oldname, const char* newname);
void save_file(const char* filename);
void change_directory(const char* path);

bool is_shell_command(const char* input);
void execute_shell_command(const char* input);

bool is_jam_script(const char* input);
void execute_jam_script(const char* filename);
void execute_task(const Task& task);

#endif // COMMANDS_H
