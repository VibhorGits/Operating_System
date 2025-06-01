#include "commands.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include "scheduler.h"     
#include <sstream>         
#include "../JAM/executionengine.h"
#include "jambo.h"
using namespace std;

// -------------------------
// Help Menu
// -------------------------

/**
 * @brief Displays the list of available JAM Shell commands and their descriptions.
 * 
 * This function prints categorized help information to guide users through 
 * the built-in JAM shell commands.
 */
void print_help_menu() {
    printf("\n=========== JAM Shell Help Menu ===========\n");
    printf("General Commands:\n");
    printf("  help                         - Show this help menu\n");
    printf("  exit                         - Exit the JAM Shell\n");
    printf("  history                      - View command history\n");
    printf("  alias name=command           - Create an alias\n");

    printf("\nFile Operations:\n");
    printf("  jcreate <filename>           - Create a new file\n");
    printf("  jsave <filename>             - Save current content to a file\n");
    printf("  jedit <filename>             - Edit a file's contents\n");
    printf("  jmodify <filename>           - Modify a file interactively\n");
    printf("  jrename <old> <new>          - Rename a file\n");
    printf("  jexecute <filename>          - Execute a JAM script\n");

    printf("\nSearch & Navigation:\n");
    printf("  sgown <term>                 - Search for term in all files\n");
    printf("  locate <term>                - Find files/folders with term in name\n");
    printf("  cd <path>                    - Change working directory\n");

    printf("\nScheduling:\n");
    printf("  jschedule <file> [priority]  - Schedule a file for execution (1-high, 2-mid, 3-low)\n");
    printf("  jschedulexecute              - Execute all scheduled tasks\n");
    printf("  jscheduleview                - View tasks in scheduling queue\n");
    printf("  jschedulesave <filename>     - Save current queues to file\n");
    printf("  jschedulecancel <task_id>    - Cancel a scheduled task by ID\n");
    printf("  jschedulemodify <id> <cmd>   - Modify a scheduled task's command\n");

    printf("\nPipes & Redirection:\n");
    printf("  command > file               - Redirect stdout to file (overwrite)\n");
    printf("  command >> file              - Redirect stdout to file (append)\n");
    printf("  command < file               - Redirect stdin from file\n");
    printf("  command1 | command2          - Pipe output of one command into another\n");

    printf("\nBackground Execution:\n");
    printf("  command &                    - Run command in background\n");

    printf("\nJAMBO Command:\n");
    printf("  jambo                       - Launch interactive JAMBO shell\n");
    printf("  jambo -l <filename>         - Perform lexer analysis on a JAM file\n");
    printf("  jambo -p <filename>         - Parse a JAM source file\n");
    printf("  jambo -s <filename>         - Run semantic analysis on a JAM file\n");

    printf("===========================================\n\n");
}


// -------------------------
// File Management Commands
// -------------------------

/**
 * @brief Creates a new file with the specified filename.
 * 
 * @param filename The name of the file to be created.
 */
void create_file(const char* filename) {
    ofstream file(filename);
    if (file) {
        cout << "File '" << filename << "' created successfully.\n";
        file.close();
    } else {
        cerr << "Failed to create file '" << filename << "'.\n";
    }
}

/**
 * @brief Opens the specified file using the nano editor.
 * 
 * @param filename The file to be edited.
 */
void edit_file(const char* filename) {
    string command = "nano ";
    command += filename;
    system(command.c_str());
}

/**
 * @brief Appends text to the specified file interactively.
 * 
 * User input continues until "EOF" is entered on a new line.
 * 
 * @param filename The name of the file to modify.
 */
void modify_file(const char* filename) {
    ofstream file(filename, ios::app);
    if (!file) {
        cerr << "Cannot open file '" << filename << "' for modification.\n";
        return;
    }
    cout << "Enter lines to append (type 'EOF' on a new line to finish):\n";
    string line;
    while (getline(cin, line)) {
        if (line == "EOF") break;
        file << line << '\n';
    }
    file.close();
}

/**
 * @brief Renames a file from oldname to newname.
 * 
 * @param oldname Current name of the file.
 * @param newname New name for the file.
 */
void rename_file(const char* oldname, const char* newname) {
    if (rename(oldname, newname) == 0) {
        cout << "Renamed '" << oldname << "' to '" << newname << "'.\n";
    } else {
        perror("Rename failed");
    }
}

/**
 * @brief Saves a predefined comment to the specified file to simulate saving runtime data.
 * 
 * @param filename The name of the file where the message should be appended.
 */
void save_file(const char* filename) {
    ofstream file(filename, ios::app);
    if (file) {
        file << "\n# Saved from JAM shell at runtime\n";
        file.close();
        cout << "File '" << filename << "' saved.\n";
    } else {
        cerr << "Failed to save file.\n";
    }
}

// -------------------------
// Directory Navigation
// -------------------------

/**
 * @brief Changes the current working directory.
 * 
 * @param path The path to change the working directory to.
 */
void change_directory(const char* path) {
    if (chdir(path) == 0) {
        cout << "Changed directory to " << path << "\n";
    } else {
        perror("chdir failed");
    }
}

// -------------------------
// Shell Command Integration
// -------------------------

/**
 * @brief Checks if a given string is a valid system shell command.
 * 
 * @param input The command to validate.
 * @return true if it is a valid shell command, false otherwise.
 */
bool is_shell_command(const char* input) {
    return system((string("command -v ") + input + " > /dev/null 2>&1").c_str()) == 0;
}

/**
 * @brief Executes a raw shell command using the system() call.
 * 
 * @param input The command to be executed.
 */
void execute_shell_command(const char* input) {
    int result = system(input);
    if (result != 0) {
        cerr << "Command exited with code: " << result << "\n";
    }
}

// -------------------------
// JAM Script Utilities
// -------------------------

/**
 * @brief Checks whether a given input string refers to a JAM script.
 * 
 * JAM scripts are identified by the ".jam" file extension.
 * 
 * @param input The input string to evaluate.
 * @return true if the input contains ".jam", false otherwise.
 */
bool is_jam_script(const char* input) {
    string str = input;
    return str.find(".jam") != string::npos;
}

// -------------------------
// JAM Script Execution
// -------------------------

/**
 * @brief Executes a JAM script using the custom JAM interpreter.
 * 
 * @param filename The name of the JAM script to be executed.
 */
void execute_jam_script(const char* filename) {
    int result = run_jam_script(filename);
    if (result != 0) {
        std::cerr << "JAM execution failed with code: " << result << std::endl;
    }
}

// -------------------------
// Scheduled Task Execution
// -------------------------

/**
 * @brief Executes a scheduled task based on the task's command.
 * 
 * Currently supports only the "jexecute <filename>" format.
 * 
 * @param task The ScheduledTask to be executed.
 */
void execute_task(const Task& task) {
    const std::string& filename = task.command;

    if (!filename.empty()) {
        std::cout << "[Scheduler] Executing JAM script: " << filename << "\n";
        execute_jam_script(filename.c_str()); // Call your actual script execution logic
    } else {
        std::cerr << "[Scheduler] Error: Empty filename in task command.\n";
    }
}


// -------------------------
// JAMBO Command Handler
// -------------------------

/**
 * @brief Handles JAMBO command-line invocations based on provided tokens.
 * 
 * Interprets command-line tokens to determine which JAMBO operation to execute:
 * Prints appropriate status messages for each operation. 
 * Displays an error message if an unrecognized command or insufficient parameters are provided.
 * 
 * @param token_count The number of command-line tokens received.
 * @param tokens      An array of strings containing the command-line tokens.
 */
void handle_jambo_command(int token_count, char *tokens[])
{
    if (token_count == 1)
    {
        printf("Jambo invoked.\n");
        run_jambo();
    }
    else if (strcmp(tokens[1], "-l") == 0 && token_count > 2)
    {
        printf("Loading JAMBO file: %s\n", tokens[2]);
        analyse_lexer(tokens[2]);
    }
    else if (strcmp(tokens[1], "-s") == 0 && token_count > 2)
    {
        printf("Saving JAMBO file: %s\n", tokens[2]);
        analyse_semantics(tokens[2]);
    }
    else if (strcmp(tokens[1], "-p") == 0 && token_count > 2)
    {
        printf("Parsing JAMBO source: %s\n", tokens[2]);
        analyse_parser(tokens[2]);
    }
    else
    {
        printf("Unknown jambo command or missing parameters.\n");
    }
}
