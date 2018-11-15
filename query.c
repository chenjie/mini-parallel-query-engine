#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

/* Call run_worker in parallel using fork. */
int main(int argc, char **argv) {
    char ch;
    char path[PATHLENGTH];
    char *startdir = ".";

    /* this models using getopt to process command-line flags and arguments */
    while ((ch = getopt(argc, argv, "d:")) != -1) {
        switch (ch) {
        case 'd':
            startdir = optarg;
            break;
        default:
            fprintf(stderr, "Usage: query [-d DIRECTORY_NAME]\n");
            exit(1);
        }
    }

    // Open the directory provided by the user (or current working directory)
    DIR *dirp;
    if ((dirp = opendir(startdir)) == NULL) {
        perror("opendir");
        exit(1);
    }

    /* For each entry in the directory, eliminate . and .., and check
     * to make sure that the entry is a directory, then call run_worker
     * to process the index file contained in the directory.
     * Note that this implementation of the query engine iterates
     * sequentially through the directories, and will expect to read
     * a word from standard input for each index it checks.
     */
    struct dirent *dp;
    // Create a global array to store all ptoc_pipe_fd[1]'s.
    int all_ptoc_pipe_ones[MAXWORKERS];
    int all_ctop_pipe_zeros[MAXWORKERS];
    int all_ptoc_pipe_ones_idx = 0;
    int all_ctop_pipe_zeros_idx = 0;
    int counter = 0;
    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 ||
            strcmp(dp->d_name, "..") == 0 ||
            strcmp(dp->d_name, ".svn") == 0 ||
            strcmp(dp->d_name, ".git") == 0) {
                continue;
        }

        strncpy(path, startdir, PATHLENGTH);
        strncat(path, "/", PATHLENGTH - strlen(path));
        strncat(path, dp->d_name, PATHLENGTH - strlen(path));
        path[PATHLENGTH - 1] = '\0';

        struct stat sbuf;
        if (stat(path, &sbuf) == -1) {
            // This should only fail if we got the path wrong
            // or we don't have permissions on this entry.
            perror("stat");
            exit(1);
        }

        // Only call run_worker if it is a directory
        // Otherwise ignore it.
        if (S_ISDIR(sbuf.st_mode)) {
            counter += 1;
            int ptoc_pipe_fd[2];
            int ctop_pipe_fd[2];
            if (pipe(ctop_pipe_fd) == -1) {
            	perror("pipe");
                exit(1);
            }
            if (pipe(ptoc_pipe_fd) == -1) {
            	perror("pipe");
                exit(1);
            }
            pid_t ret = fork();
            if (ret > 0) { // parent
                // printf("fork\n");
                all_ptoc_pipe_ones[all_ptoc_pipe_ones_idx] = ptoc_pipe_fd[1];
                all_ctop_pipe_zeros[all_ctop_pipe_zeros_idx] = ctop_pipe_fd[0];
                all_ptoc_pipe_ones_idx++;
                all_ctop_pipe_zeros_idx++;
                close(ptoc_pipe_fd[0]);
                close(ctop_pipe_fd[1]);
            } else if (ret == 0) { // child
                close(ptoc_pipe_fd[1]);
                close(ctop_pipe_fd[0]);
                // printf("[%d, %d]\n", ptoc_pipe_fd[0], ctop_pipe_fd[1]);
                run_worker(path, ptoc_pipe_fd[0], ctop_pipe_fd[1]);
                // printf("CLOSED\n");
                close(ptoc_pipe_fd[0]);
                close(ctop_pipe_fd[1]);
                exit(0);
            } else {
                perror("fork");
                exit(1);
            }
        }
    }

    // while loop
    while (1) {
        char input_word[MAXWORD];
        int scanf_ret;
        if ((scanf_ret = scanf("%s", input_word)) == EOF) {

            // Close write fd to workers
            for (int i = 0; i < all_ptoc_pipe_ones_idx; i++) {
                close(all_ptoc_pipe_ones[i]);
            }

            // Close read fd from workers
            for (int i = 0; i < all_ctop_pipe_zeros_idx; i++) {
                close(all_ctop_pipe_zeros[i]);
            }

            for (int i = 0; i < all_ptoc_pipe_ones_idx; i++) {
                int status;
                if (wait(&status) == -1) {
                    perror("wait");
                    exit(1);
                }
            }

            if (closedir(dirp) < 0) {
                perror("closedir");
            }

            break;
        }

        // Write to workers
        for (int i = 0; i < all_ptoc_pipe_ones_idx; i++) {
            // printf("[%d] write_fd\n", all_ptoc_pipe_ones[i]);
            if (write(all_ptoc_pipe_ones[i], input_word, MAXWORD) == -1) {
                perror("ptoc->write");
                exit(1);
            }
        }

        FreqRecord tmp_freq_array[MAXRECORDS];
        int tmp_freq_array_idx = 0;
        // Read from workers
        for (int i = 0; i < all_ctop_pipe_zeros_idx; i++) {
            FreqRecord fr;
            int ret;
            while ((ret = read(all_ctop_pipe_zeros[i], &fr, sizeof(FreqRecord))) != 0) {
                if (ret == -1) {
                    perror("ctop->read");
                    exit(1);
                }
                if ((fr.filename)[0] != '\0') {
                    tmp_freq_array[tmp_freq_array_idx] = fr;
                    tmp_freq_array_idx++;
                } else {
                    // Super important!!!
                    break;
                }
            }
            // Sorting part is ommited.
        }
        tmp_freq_array[tmp_freq_array_idx].freq = 0;
        (tmp_freq_array[tmp_freq_array_idx].filename)[0] = '\0';
        print_freq_records(tmp_freq_array);
    }

    return 0;
}
