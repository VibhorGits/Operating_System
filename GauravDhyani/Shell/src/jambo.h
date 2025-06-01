#ifndef JAMBO_H
#define JAMBO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <curl/curl.h>


#ifdef __cplusplus
#include <iostream>
#include <sstream>
#include <string>
#include "./include/json.hpp"
using json = nlohmann::json;

// C++ only declarations
typedef struct {
    ASTNode* ast;
    std::stringstream* output;
} PrintASTArgs;

void printAST_to_stdout(void* arg);

typedef struct {
    ASTNode* ast;
} DebugTraverseArgs;

void debugTraverse_to_stdout(void* arg);

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);
std::string callGroqAPI(const std::string& userMessage);
void run_jambo();

#endif // __cplusplus

// Analysis functions (available in both C and C++)
void analyse_lexer(const char* filename);
void analyse_parser(const char* filename);
void analyse_semantics(const char* filename);

// stdout capture utility (available in both C and C++)
char* capture_stdout_output(void (*func)(void*), void* arg);
std::string sanitizeResponse(const std::string& input);

#endif // JAMBO_H
