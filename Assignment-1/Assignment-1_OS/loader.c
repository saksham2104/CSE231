#include "loader.h"

Elf32_Ehdr *ehdr = NULL;
Elf32_Phdr *phdr = NULL;
int fd = -1;
void *file_content = NULL; //contains content about the file

//Error codes defined below,will help with switchases and int return type functions

#define ERR_OPEN_FILE     1
#define ERR_sizeoffile     2 
#define ERR_ALLOC_MEMORY  3
#define ERR_READ_FILE     4
#define ERR_MAP_SEGMENT   5
#define ERR_NO_ENTRY      6

//Function for displaying various errors
//Used switchase for a clean and organized error mapping.
void handle_error(int error_code) {
    switch (error_code) {
        case ERR_OPEN_FILE:
            perror("Couldn't open file");
            break;
        case ERR_sizeoffile:
            perror("Couldn't get file size");
            break;
        case ERR_ALLOC_MEMORY:
            perror("Memory allocation failed");
            break;
        case ERR_READ_FILE:
            perror("Couldn't read file");
            break;
        case ERR_MAP_SEGMENT:
            perror("Segment mapping failed");
            break;
        case ERR_NO_ENTRY:
            perror( "Entry point not found");
            break;
        default:
            perror( "An unknown error occurred");
            break;
    }

    loader_cleanup();
    exit(EXIT_FAILURE);
}




/*
 * Release memory and other cleanups
 */
void loader_cleanup() {
    if (file_content) {
        free(file_content);  // Free the memory that was allocated
        file_content = NULL;
    }
    if (fd >= 0) {
        close(fd);  // Close the file descriptor if it was opened
        fd = -1;
    }
}


 //Map the ELF file into memory and return the error code if any
 
int map_elf_file(const char *filename) {
    int error_code = 0; //initialise error code with zero and then update it as soon as an error occurs and leave the function

    // Open the file
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return ERR_OPEN_FILE;
    }

    // get the file size
    off_t sizeoffile = lseek(fd, 0, SEEK_END);
    if (sizeoffile == -1) {
        return ERR_sizeoffile;
    }
    lseek(fd, 0, SEEK_SET);  // Reset fd

    // Allocate memory for the file
    file_content = malloc(sizeoffile);
    if (!file_content) {
        return ERR_ALLOC_MEMORY;
    }

    // Read the file contents into the allocated memory
    ssize_t readingspace = read(fd, file_content, sizeoffile);
    if (readingspace != sizeoffile) {
        return ERR_READ_FILE;
    }

    // Assign the ELF header to the mapped memory
    ehdr = (Elf32_Ehdr *)file_content;
    return error_code;
}

// Load the segments from the ELF file into memory and return the error code if any
 
int load_segments(void **entry_point) {
    int error_code = 0;
    phdr = (Elf32_Phdr *)((char *)file_content + ehdr->e_phoff);  // Point to program headers
    *entry_point = NULL;
    // Iterate over all program headers and load the one into memory that has type PT_load and contains entry point
    for (int i = 0; i < ehdr->e_phnum; i++) {  //i reepresents segments
        if (phdr[i].p_type == PT_LOAD) {
            // Check entry point existence in that segment
            if (ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < phdr[i].p_vaddr + phdr[i].p_memsz) {
                // Map the segment into memory
                void *segment_add = mmap((void *)phdr[i].p_vaddr, phdr[i].p_memsz,
                                            PROT_READ | PROT_WRITE | PROT_EXEC,
                                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                if (segment_add == MAP_FAILED) {
                    return ERR_MAP_SEGMENT;
                }
                // Copy  segment data 
                memcpy(segment_add, (char *)file_content + phdr[i].p_offset, phdr[i].p_filesz);
                *entry_point = (void *)ehdr->e_entry;
                break; 
            }
        }
    }
    // If no entry point was found, return an error code
    if (!*entry_point) {
        return ERR_NO_ENTRY;
    }

    return error_code;
}

/*
 * Load and run the ELF executable file
 */

void load_and_run_elf(char** exe) {
    int error_code;
    void *entry_point = NULL;

    // Map the ELF file into memory
    error_code = map_elf_file(exe[0]);
    if (error_code != 0) {
        handle_error(error_code);
    }

    // Load the segments and get the entry point
    error_code = load_segments(&entry_point);
    if (error_code != 0) {
        handle_error(error_code);
    }

    // Use the entry to get the result
    int (*_start)() = (int (*)())entry_point;
    int result = _start();  
    printf("User _start return value = %d\n", result);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ELF Executable>\n", argv[0]); //error provided beforehand hence not integrated in switch case
        exit(EXIT_FAILURE);
    }

    // Load and run the ELF executable
    load_and_run_elf(&argv[1]); //had to pass it as reference as data type in argument and paramter isn't equal

    // Clean up resources
    loader_cleanup();
    return 0;
}

