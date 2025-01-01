Contributions:Saksham->Updating loader.c from Assignment-1,debugging
              Atin-> Error handling,page_fault function 

Summary of the code:
    In this code  we load an executable into memory and execute it, with dynamic page loading on-demand by handling page faults. This is akin to virtual memory management in operating systems where pages of memory are loaded only when accessed, allowing efficient memory usage.


Code Flow
1:Main Function:
    Checks if the correct number of command-line arguments is passed (i.e., one ELF executable file).
    Calls load_and_run_elf with the filename as an argument to start the loading and execution process.
    After the execution, prints diagnostic information such as page fault count, page allocation count, and internal fragmentation.
    Finally, it calls loader_cleanup to free resources and close any open files.

2:load_and_run_elf Function:
    Step 1: Calls map_file_content to open and read the ELF file into memory.
    Step 2: Calls initialize_elf_headers to parse the ELF headers and locate the entry point.
    Segfault Handler: Sets up a signal handler (handle_page_fault) for page faults, using sigaction for catching SIGSEGV.
    Execution: Starts the execution of the ELF file from its entry point.

3:map_file_content Function:
    Opens the ELF file, retrieves its size, and maps it into memory.
    If any errors occur during this process (e.g., file open or read errors), it returns an error code.

4:initialize_elf_headers Function:
    Parses ELF and program headers to locate the entry point.
    Checks each program header to ensure the entry point lies within a PT_LOAD (loadable) segment.

5:handle_page_fault Function (Core Focus):
    Purpose: Handles page faults by loading the required page of the ELF segment into memory.
    Process:
    Retrieves the faulting address from siginfo_t (info->si_addr), identifying the page where the fault occurred.
    Iterates through each PT_LOAD segment in the ELF headers to check if the fault address falls within the segment’s virtual address range.
    Calculates the starting address of the page (page_start), segment offset, and the data size needed for the page (either a full page size or the remaining segment size if it's smaller than a page).
    Uses mmap to map a new page with PROT_READ | PROT_WRITE | PROT_EXEC permissions.
    If mmap succeeds, it copies the relevant portion of the segment data to the mapped memory using memcpy.
    Increments counters for page faults, page allocations, and calculates internal fragmentation as the difference between allocated page size and actual data size.
    Error Handling: If the fault address doesn't fall within any loadable segment, it triggers a segmentation fault and exits the program.

6:handle_error Function:
    Takes an error code and displays a corresponding error message.
    Calls loader_cleanup to free resources and exits the program.

7:loader_cleanup Function:
    Frees the mapped file content and closes the file descriptor.
    Iterates over each loadable segment and unmaps any allocated memory pages for cleanup.

github link -https://github.com/Atin1704/Os_Assignment_4.git
