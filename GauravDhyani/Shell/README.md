# JAM Shell (Integrated with JAM Compiler)
This is the custom shell that integrates the JAM scripting language with traditional shell features like command execution, history, and a multilevel scheduler.

## Required Directory Structure
   
   ```bash
   JAM-Shell-Project/
│
├── JAM/                       # JAM Compiler Source Files
│   ├── lexer.c
│   ├── lexer.h
│   ├── parser.c
│   ├── parser.h
│   ├── semanticanalyser.c
│   ├── semanticanalyser.h
│   ├── executionengine.c
│   ├── executionengine.h
│   ├── main.c                 # Optional: for standalone JAM execution
│   ├── libjam.a               # Static library created from JAM source
│
├── Shell/                     # Shell Source Files
│   ├── shell.cpp
│   ├── commands.cpp
│   ├── commands.h
│   ├── history.cpp
│   ├── history.h
│   ├── scheduler.cpp
│   ├── scheduler.h
│   ├── jam_shell              # Executable output after building
   ```
## 🚀 How to Run
1. **Compile the program** 
   Open your terminal in the Shell directory and run:
   
   ```bash
   g++ shell.cpp commands.cpp history.cpp scheduler.cpp -o jam_shell -I../JAM -I. -L../JAM -ljam -pthread
   
  -I../JAM includes header files from the JAM directory
  -L../JAM links the static library libjam.a
  -ljam links the JAM interpreter
  -pthread enables thread support
   ```

2. **Execute the program**
   After successful compilation, run the integrated JAM shell:
  
   ```bash
   ./jam_shell
   ```