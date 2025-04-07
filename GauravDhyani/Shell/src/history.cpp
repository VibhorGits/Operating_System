#include "history.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define HISTORY_FILE ".jam_history"
#define MAX_HISTORY 100


// -------------------------
// Internal History Storage
// -------------------------

static std::vector<std::string> history;

// -------------------------
// History Management
// -------------------------

/**
 * @brief Adds a command to the history.
 * 
 * If the history exceeds the MAX_HISTORY limit, the oldest entry is removed.
 * 
 * @param command The command string to be added to the history.
 */
void add_to_history(const char* command) {
    if (history.size() >= MAX_HISTORY) {
        history.erase(history.begin());
    }
    history.push_back(command);
}

/**
 * @brief Prints the current command history to the console.
 */
void print_history() {
    for (size_t i = 0; i < history.size(); ++i) {
        std::cout << i + 1 << ": " << history[i] << "\n";
    }
}


/**
 * @brief Loads command history from the persistent history file.
 * 
 * Reads each line from HISTORY_FILE and adds it to the in-memory history vector.
 */
void load_history() {
    std::ifstream file(HISTORY_FILE);
    std::string line;
    while (getline(file, line)) {
        if (!line.empty()) {
            history.push_back(line);
        }
    }
    file.close();
}

/**
 * @brief Saves the current in-memory command history to a persistent file.
 * 
 * Writes each command from the history vector into HISTORY_FILE.
 */
void save_history() {
    std::ofstream file(HISTORY_FILE);
    for (const auto& cmd : history) {
        file << cmd << "\n";
    }
    file.close();
}
