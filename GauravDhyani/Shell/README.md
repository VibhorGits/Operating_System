# JAM Shell (Integrated with JAM Compiler)
This is the custom shell that integrates the JAM scripting language with traditional shell features like command execution, history, and a multilevel scheduler.

## Required Directory Structure
   
   ```bash
   JAM-Shell-Project/
│
├── JAM/                       # JAM Compiler Source Files
│   ├── libjam.a               # Static library created from JAM source
│
├── Shell/                     # Shell Source Files
│   ├── include/
│       │   ├── json.hpp       # From nlohmann/json repo
│   ├── shell.cpp
│   ├── commands.cpp
│   ├── commands.h
│   ├── history.cpp
│   ├── history.h
│   ├── scheduler.cpp
│   ├── scheduler.h
│   ├── jambo.cpp
│   ├── jambo.h
│   ├── jam                    # Executable output after building
   ```
## Configuration
   **Before building and running, configure your Groq API key for the AI chatbot integration:**
   1. In jambo.cpp, locate this line:
   ```bash
   const std::string API_KEY = "EnterYourGroqKey";
   ```
   2. Replace "EnterYourGroqKey" with your actual Groq API key string.

## Installing Dependencies
   **Install required dependencies for compiling and running the JAM Shell:**
     
     1. If you’re on Ubuntu/Debian:
        ```bash
        sudo apt-get install libreadline-dev libcurl4-openssl-dev
        ```

     2. If you’re on Fedora/RHEL:
        ```bash
        sudo dnf install readline-devel libcurl-devel
        ```

##  How to Run
1. **Compile the program** 
   Open your terminal in the Shell directory and run:
   
   ```bash
   g++ shell.cpp jambo.cpp commands.cpp history.cpp scheduler.cpp -o jam -I../JAM -I. -Iinclude -L../JAM -ljam -lcurl -lreadline -pthread
   ```

2. **Execute the program**
   After successful compilation, run the integrated JAM shell:
  
   ```bash
   ./jam
   ```