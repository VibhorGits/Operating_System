/**
 * @file shell.c
 * @brief JAM Shell REPL with Modular Task Flow and Extended Capabilities.
 * 
 * This shell provides support for command execution, scripting, history,
 * file operations, aliasing, scheduling, and more advanced shell features
 * like redirection and piping.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <fcntl.h>
#include <signal.h>
#include <sstream>             

#include "commands.h"
#include "history.h"
#include "scheduler.h"

#define MAX_INPUT 1024

namespace fs = std::filesystem;
using namespace std;

/// Alias storage
unordered_map<string, string> aliases;

// -------------------- Task 1: Initialization --------------------

/**
 * @brief Displays the JAM Shell welcome banner.
 */
void show_banner()
{
    cout << "-----------------------------------------------------------\n";
    cout << "                  Welcome to the JAM Shell                 \n";
    cout << "-----------------------------------------------------------\n";
    cout << "       Type \033[1mhelp\033[0m to view available commands.\n";
    cout << "-----------------------------------------------------------\n";
}

// -------------------- Task 2: Utilities --------------------

/**
 * @brief Retrieves the current system time as a formatted string.
 * @return Current time string (no newline).
 */
string current_time()
{
    time_t now = time(nullptr);
    string time_str = ctime(&now);
    time_str.pop_back();
    return time_str;
}

/**
 * @brief Interprets return codes from subprocesses to human-readable messages.
 * @param code Exit code from subprocess.
 * @return String message for the code.
 */
string interpret_error(int code)
{
    switch (code)
    {
    case 0:
        return "";
    case 139:
    case 132:
        return "Error: Invalid command syntax.\n";
    case 512:
        return "Error: Improper command usage.\n";
    default:
        return "Error Code [" + to_string(code) + "] occurred.\n";
    }
}

// -------------------- Task 3: File Search --------------------

/**
 * @brief Searches a file for lines containing a given pattern.
 * @param filepath Path to the file.
 * @param pattern String to search for.
 * @return Vector of matched lines with line numbers and highlights.
 */
vector<pair<int, string>> grep_in_file(const string &filepath, const string &pattern)
{
    ifstream file(filepath);
    string line;
    int lineno = 0;
    vector<pair<int, string>> results;

    while (getline(file, line))
    {
        lineno++;
        size_t pos = line.find(pattern);
        if (pos != string::npos)
        {
            string highlighted = line;
            highlighted.insert(pos + pattern.length(), "\033[0m");
            highlighted.insert(pos, "\033[31m");
            results.emplace_back(lineno, highlighted);
        }
    }
    return results;
}

/**
 * @brief Recursively searches a directory for pattern matches in files.
 * @param root Root directory.
 * @param term Search term.
 * @return Map of file paths to matched lines and highlights.
 */
unordered_map<string, vector<pair<int, string>>> search_directory(const string &root, const string &term)
{
    unordered_map<string, vector<pair<int, string>>> findings;
    for (const auto &entry : fs::recursive_directory_iterator(root))
    {
        if (entry.is_regular_file())
        {
            auto matches = grep_in_file(entry.path().string(), term);
            if (!matches.empty())
            {
                findings[entry.path().string()] = matches;
            }
        }
    }
    return findings;
}

/**
 * @brief Displays search results with highlighting and line numbers.
 * @param data Map of file paths to matched line information.
 */
void display_search_results(const unordered_map<string, vector<pair<int, string>>> &data)
{
    for (const auto &[file, entries] : data)
    {
        cout << "\n\033[95m" << file << "\033[0m\n";
        for (const auto &[line, text] : entries)
        {
            cout << "\033[34m" << line << "\033[0m: ..." << text << endl;
        }
    }
}

// -------------------- Task 4: File Location --------------------

/**
 * @brief Finds paths containing the search term in their names.
 * @param root Directory to search from.
 * @param term Search term.
 * @return Vector of matched file/directory paths.
 */
vector<string> find_paths_containing(const string &root, const string &term)
{
    vector<string> found;
    for (const auto &entry : fs::recursive_directory_iterator(root))
    {
        string path_str = entry.path().string();
        size_t pos = path_str.find(term);
        if (pos != string::npos)
        {
            path_str.insert(pos + term.length(), "\033[0m");
            path_str.insert(pos, "\033[31m");
            found.push_back(path_str);
        }
    }
    return found;
}

/**
 * @brief Outputs the found paths.
 * @param paths Vector of colored, matched paths.
 */
void show_found_paths(const vector<string> &paths)
{
    for (const auto &p : paths)
    {
        cout << p << endl;
    }
}

// -------------------- Task 5: Redirection and Piping --------------------

/**
 * @brief Handles input/output redirection and pipes, then executes commands.
 * @param args Command arguments.
 * @param background Whether the command runs in the background.
 */
void handle_redirection_and_execute(vector<char *> &args, bool background)
{
    int in_fd = 0, out_fd = 1;
    vector<char *> command;
    bool piping = false;
    int pipefd[2];

    for (size_t i = 0; i < args.size(); ++i)
    {
        if (strcmp(args[i], "|") == 0)
        {
            piping = true;
            args[i] = nullptr;
            pipe(pipefd);
            pid_t pid = fork();
            if (pid == 0)
            {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                execvp(args[0], args.data());
                perror("execvp");
                exit(1);
            }
            else
            {
                wait(nullptr);
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[1]);
                vector<char *> rightCmd(args.begin() + i + 1, args.end());
                rightCmd.push_back(nullptr);
                execvp(rightCmd[0], rightCmd.data());
                perror("execvp");
                exit(1);
            }
            return;
        }
        else if (strcmp(args[i], ">") == 0)
        {
            out_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            args[i] = nullptr;
            break;
        }
        else if (strcmp(args[i], ">>") == 0)
        {
            out_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            args[i] = nullptr;
            break;
        }
        else if (strcmp(args[i], "<") == 0)
        {
            in_fd = open(args[i + 1], O_RDONLY);
            args[i] = nullptr;
            break;
        }
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        if (in_fd != 0)
            dup2(in_fd, STDIN_FILENO);
        if (out_fd != 1)
            dup2(out_fd, STDOUT_FILENO);
        execvp(args[0], args.data());
        perror("execvp");
        exit(1);
    }
    else
    {
        if (!background)
            wait(nullptr);
    }
}

// -------------------- Task 6: Alias, History, Env --------------------

/**
 * @brief Applies alias substitution to the input command.
 * @param input User input string to modify in-place.
 */
void apply_aliases(char *input)
{
    string cmd(input);
    stringstream ss(cmd);
    string token;
    ss >> token;
    if (aliases.find(token) != aliases.end())
    {
        cmd.replace(0, token.length(), aliases[token]);
        strncpy(input, cmd.c_str(), MAX_INPUT);
    }
}

/**
 * @brief Expands environment variables in the input command.
 * @param input Input string with $VAR-style references.
 */
void expand_env_variables(char *input)
{
    stringstream ss(input);
    string token, result;
    while (ss >> token)
    {
        if (token[0] == '$')
        {
            const char *val = getenv(token.substr(1).c_str());
            result += val ? string(val) : "";
        }
        else
        {
            result += token;
        }
        result += " ";
    }
    strcpy(input, result.c_str());
}

// -------------------- Task 7: Shell Execution Loop --------------------

/**
 * @brief Main shell REPL loop.
 */
void run_shell_loop()
{
    char input[MAX_INPUT];

    while (true)
    {
        printf("JAM [%s]> ", current_time().c_str());
        fflush(stdout);

        if (!fgets(input, MAX_INPUT, stdin))
        {
            printf("\nSession terminated.\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0)
            continue;

        add_to_history(input);

        if (strncmp(input, "exit", 4) == 0)
            break;

        apply_aliases(input);
        expand_env_variables(input);

        // Tokenize input
        char *tokens[64];
        int token_count = 0;
        char *token = strtok(input, " ");
        while (token != NULL && token_count < 64)
        {
            tokens[token_count++] = token;
            token = strtok(NULL, " ");
        }
        tokens[token_count] = NULL;

        if (token_count == 0)
            continue;

        if (strcmp(tokens[0], "sgown") == 0 && token_count > 1)
        {
            auto result = search_directory(".", tokens[1]);
            display_search_results(result);
        }
        else if (strcmp(tokens[0], "help") == 0)
        {
            print_help_menu();
        }
        else if (strcmp(tokens[0], "locate") == 0 && token_count > 1)
        {
            auto results = find_paths_containing(".", tokens[1]);
            show_found_paths(results);
        }
        else if (strcmp(tokens[0], "jcreate") == 0 && token_count > 1)
        {
            create_file(tokens[1]);
        }
        else if (strcmp(tokens[0], "jsave") == 0 && token_count > 1)
        {
            save_file(tokens[1]);
        }
        else if (strcmp(tokens[0], "jedit") == 0 && token_count > 1)
        {
            edit_file(tokens[1]);
        }
        else if (strcmp(tokens[0], "jmodify") == 0 && token_count > 1)
        {
            modify_file(tokens[1]);
        }
        else if (strcmp(tokens[0], "jexecute") == 0 && token_count > 1)
        {
            execute_jam_script(tokens[1]);
        }
        else if (strcmp(tokens[0], "jrename") == 0 && token_count > 2)
        {
            rename_file(tokens[1], tokens[2]);
        }
        else if (strcmp(tokens[0], "cd") == 0 && token_count > 1)
        {
            change_directory(tokens[1]);
        }
        else if (strcmp(tokens[0], "history") == 0)
        {
            print_history();
        }
        else if (strcmp(tokens[0], "jschedule") == 0)
        {
            if (token_count < 2)
            {
                std::cerr << "Usage: jschedule <filename> [priority]\n";
                continue;
            }
            std::string filename = tokens[1];
            int priority = (token_count > 2) ? atoi(tokens[2]) : 2;
            jschedule_command(filename, priority);
        }
        else if (strcmp(tokens[0], "jschedulexecute") == 0)
        {
            jschedulexecute_command();
        }
        else if (strcmp(tokens[0], "alias") == 0 && token_count > 1)
        {
            std::string def_str = tokens[1];
            for (int i = 2; i < token_count; ++i)
            {
                def_str += " ";
                def_str += tokens[i];
            }
            auto eq = def_str.find('=');
            if (eq != std::string::npos)
            {
                std::string key = def_str.substr(0, eq);
                std::string val = def_str.substr(eq + 1);
                aliases[key] = val;
                std::cout << "Alias set: " << key << " -> " << val << std::endl;
            }
        }
        else if (strcmp(tokens[0], "jscheduleview") == 0)
        {
            print_scheduled_tasks();
        }
        else if (strcmp(tokens[0], "jschedulesave") == 0 && token_count > 1)
        {
            save_queues_to_file(tokens[1]);
        }
        else if (strcmp(tokens[0], "jschedulecancel") == 0 && token_count > 1)
        {
            int id = atoi(tokens[1]);
            cancel_task(id);
        }
        else if (strcmp(tokens[0], "jschedulemodify") == 0 && token_count > 2)
        {
            int id = atoi(tokens[1]);
            std::string new_cmd = tokens[2];
            for (int i = 3; i < token_count; ++i)
            {
                new_cmd += " ";
                new_cmd += tokens[i];
            }
            modify_task(id, new_cmd);
        }
        else if (is_shell_command(input))
        {
            execute_shell_command(input);
        }
        else
        {
            std::vector<char *> args;
            for (int i = 0; i < token_count; ++i)
            {
                args.push_back(tokens[i]);
            }

            bool background = false;
            if (!args.empty() && strcmp(args.back(), "&") == 0)
            {
                args.pop_back();
                background = true;
            }

            args.push_back(nullptr);
            handle_redirection_and_execute(args, background);
        }
    }
}

// -------------------- Task 8: Main Entry --------------------

/**
 * @brief Main function for JAM Shell.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Exit code.
 */
int main(int argc, char *argv[])
{
    show_banner();
    load_history();
    run_shell_loop();
    save_history();
    return 0;
}
