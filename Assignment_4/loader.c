#include "loader.h"
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PAGE_SIZE 4096
#define ERR_OPEN_FILE     1
#define ERR_SIZE_OF_FILE  2 
#define ERR_ALLOC_MEMORY  3
#define ERR_READ_FILE     4
#define ERR_MAP_SEGMENT   5
#define ERR_NO_ENTRY      6

Elf32_Ehdr *ehdr = NULL;
Elf32_Phdr *phdr = NULL;
int fd = -1;
void *file_content = NULL;
size_t page_fault_count = 0;
size_t page_alloc_count = 0;
size_t internal_fragmentation = 0;

void handle_error(int error_code) {
    switch (error_code) {
        case ERR_OPEN_FILE:
            perror("Couldn't open file");
            break;
        case ERR_SIZE_OF_FILE:
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
            perror("Entry point not found");
            break;
        default:
            perror("An unknown error occurred");
            break;
    }

    loader_cleanup();
    exit(EXIT_FAILURE);
}

void loader_cleanup() {
    if (file_content) {
        free(file_content);
        file_content = NULL;
    }
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            size_t segment_start = phdr[i].p_vaddr & ~(PAGE_SIZE - 1);
            size_t segment_size = phdr[i].p_memsz;
            size_t pages_to_unmap = (segment_size + PAGE_SIZE - 1) / PAGE_SIZE;
            munmap((void *)segment_start, pages_to_unmap * PAGE_SIZE);
        }
    }
}

// Map the ELF file into memory and return error code if any
int map_file_content(const char *filename) {
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        return ERR_OPEN_FILE;
    }

    off_t sizeoffile = lseek(fd, 0, SEEK_END);
    if (sizeoffile == -1) {
        return ERR_SIZE_OF_FILE;
    }
    lseek(fd, 0, SEEK_SET);

    file_content = malloc(sizeoffile);
    if (!file_content) {
        return ERR_ALLOC_MEMORY;
    }

    ssize_t readingspace = read(fd, file_content, sizeoffile);
    if (readingspace != sizeoffile) {
        return ERR_READ_FILE;
    }
    //ehdr = (Elf32_Ehdr *)file_content;
    return 0;
}

// Initialize ELF and program headers based on mapped file content
int initialize_elf_headers() {
    ehdr = (Elf32_Ehdr *)file_content;
    phdr = (Elf32_Phdr *)((char *)file_content + ehdr->e_phoff);

    // Verify if entry point exists in one of the PT_LOAD segments
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD &&
            ehdr->e_entry >= phdr[i].p_vaddr &&
            ehdr->e_entry < phdr[i].p_vaddr + phdr[i].p_memsz) {
            return 0;  // Entry point found in a PT_LOAD segment
        }
    }
    return ERR_NO_ENTRY;  // No entry point found in any loadable segment
}

// Page fault handler for page-by-page loading of ELF segments
void handle_page_fault(int sig, siginfo_t *info, void *context) {
    void *fault_addr = info->si_addr;

    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD &&(size_t)fault_addr >= phdr[i].p_vaddr &&
            (size_t)fault_addr < phdr[i].p_vaddr + phdr[i].p_memsz){

            size_t page_start = ((size_t)fault_addr / PAGE_SIZE) * PAGE_SIZE;
            size_t segment_offset = page_start - phdr[i].p_vaddr;
            size_t remaining_data = phdr[i].p_memsz - segment_offset;
            size_t copy_size = (remaining_data < PAGE_SIZE) ? remaining_data : PAGE_SIZE;

            if (copy_size > 0) {
                void *mapped_addr = mmap((void *)page_start, PAGE_SIZE,
                                         PROT_READ | PROT_WRITE | PROT_EXEC,
                                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

                if (mapped_addr == MAP_FAILED) {
                    perror("Page mapping failed");
                    exit(EXIT_FAILURE);
                }

                memcpy(mapped_addr, (char *)file_content + phdr[i].p_offset + segment_offset, copy_size);

                page_fault_count++;
                page_alloc_count++;
                internal_fragmentation += PAGE_SIZE - copy_size;

                return;
            }
        }
    }
    fprintf(stderr, "Segfault at address not within any segment\n");
    exit(EXIT_FAILURE);
}

void load_and_run_elf(char **exe) {
    int error_code;
    void *entry_point;

    // Step 1: Map the file content
    error_code = map_file_content(exe[0]);
    if (error_code != 0) {
        handle_error(error_code);
    }

    // Step 2: Initialize ELF headers and locate entry point
    error_code = initialize_elf_headers();
    if (error_code != 0) {
        handle_error(error_code);
    }

    entry_point = (void *)ehdr->e_entry;

    // Set up segfault handler for page faults
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_page_fault;
    sigaction(SIGSEGV, &sa, NULL);

    // Execute the entry point
    int (*_start)() = (int (*)())entry_point;
    int result = _start();
    printf("User _start return value = %d\n", result);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ELF Executable>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    load_and_run_elf(&argv[1]);

    printf("Total page faults: %zu\n", page_fault_count);
    printf("Total page allocations: %zu\n", page_alloc_count);
    printf("Internal fragmentation: %zu KB\n", internal_fragmentation / 1024);

    loader_cleanup();
    return 0;
}
