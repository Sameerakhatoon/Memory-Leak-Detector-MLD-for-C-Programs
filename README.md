## Memory Leak Detector (MLD) for C Programs

### Overview

Memory Leak Detector (MLD) is a lightweight tool designed to detect memory leaks in C programs. It helps developers identify dynamically allocated memory that is not freed before program termination. This tool is implemented in C and provides a detailed report of memory leaks, making debugging more efficient.

### Features

- Detects memory leaks in C programs.
    
- Supports two database structures for tracking allocations:
    
    - **Linked Lists**: Stores allocation records in a linked list.
        
    - **Hash Maps**: Uses a hashmap for faster lookups.
        
- Tracks allocated memory blocks, including file name, line number, size, and status.
    
- Reports leaked memory upon program termination.
    
- Lightweight with minimal performance overhead.
    

---

## Installation and Compilation

### Prerequisites

- GCC compiler (`gcc`)
    

### Steps to Compile

To compile the project, follow these steps:

1. **For Linked List Implementation:**
    
    `cd mld/mld_dbs_as_linkedlists gcc -o exe appn.c mld.c`
    
2. **For Hash Map Implementation:**
    
    `cd mld/mld_dbs_as_hashmaps gcc -o exe appn.c mld.c`
    

---

## Usage

1. Run the compiled executable:
    
    `./exe`
    
2. The program will execute and track memory allocations dynamically.
    
3. If memory leaks are detected, a report will be displayed at the end of execution.
    

---

## How It Works

- `malloc()` and `calloc()` allocations are tracked using a custom implementation.
    
- Each allocation is recorded in either a linked list or hashmap.
    
- Upon calling `free()`, the record is removed from the tracking structure.
    
- When the program terminates, the tool reports any memory that was allocated but not freed.

---
## Future Improvements and Enhancements

To further enhance the Memory Leak Detector (MLD) and make it more robust, the following improvements can be implemented:

### **1. Automatic `calloc`-based Structure Database & Object Database Management**

- Maintain an automatic structure and object database for tracking allocations.
    
- Store metadata for allocated structures without explicit user intervention.
    

### **2. Automatic Parsing for Structure Registration**

- Eliminate manual registration of structures by automatically detecting them at compile-time or runtime.
    
- Use **macro-based reflection** or **preprocessor-based code analysis** to identify memory allocations dynamically.
    

### **3. Automatic Global Root Object Registration**

- Automatically mark specific objects as **root objects** in the memory tracking system.
    
- Prevent tracking of short-lived objects by focusing on root objects.
    

### **4. Automatic Memory API Wrapping (`xcalloc`, `xmalloc`, `xfree`)**

- Override `malloc()`, `calloc()`, and `free()` with custom tracking functions (`xcalloc`, `xmalloc`, `xfree`).
    
- The user will only register **root objects**, and all child allocations will be tracked automatically.
    

### **5. Conditional Enabling/Disabling of MLD (`#ifdef` macros)**

- Allow dynamic control over memory tracking using **preprocessor directives**.
    
- This allows users to compile their program with or without memory tracking based on requirements.
    

### **6. Automatic Parsing Using a Preprocessor or Clang AST**

- Implement a **parser or AST-based tool** to detect structure definitions and auto-register them.
    
- Example: Use **Clang AST (Abstract Syntax Tree)** or **Lex/Yacc-based parsing**.
    

### **7. Separate Background Thread for Memory Tracking**

- Move memory leak detection into a separate thread for **asynchronous monitoring**.
    
- Reduce runtime overhead by performing expensive memory analysis in a background worker thread.

---
## Contributing

- Feel free to fork and improve the project.
    
- Open an issue if you find bugs or have suggestions.
