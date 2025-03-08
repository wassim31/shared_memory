#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define SHM_SIZE 2048

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    int number = atoi(argv[1]);  // Read number from command line
    const char *name = "mem";
    int fd;
    void *mptr;

    pid_t pid = fork();

    if (pid == 0) {  // Child process
        fd = shm_open(name, O_CREAT | O_RDWR, 0666);
        ftruncate(fd, SHM_SIZE);
        void *ptr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);

        if (ptr == MAP_FAILED) {
            perror("mmap");
            exit(1);
        }

        char *shm_ptr = ptr;
        while (number != 1) {
            char buffer[16];  // Increased size
            sprintf(buffer, "%d,", number);
            strcpy(shm_ptr, buffer);
            shm_ptr += strlen(buffer);  // Move pointer correctly

            if (number % 2 == 0) {
                number /= 2;
            } else {
                number = number * 3 + 1;
            }
        }
        sprintf(shm_ptr, "1");  // Append last number
        close(fd);
        exit(0);  // Exit child
    } 
    
    else if (pid > 0) {  // Parent process
        wait(NULL);  // Wait for child

        fd = shm_open(name, O_RDONLY, 0666);
        mptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);

        if (mptr == MAP_FAILED) {
            perror("mmap");
            return 1;
        }

        printf("Collatz sequence: %s\n", (char *)mptr);

        shm_unlink(name);  // Cleanup
        close(fd);
    } 
    
    else {
        perror("fork");
        return 1;
    }

    return 0;
}
