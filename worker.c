#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

/* Counts the number of files */
int get_num_of_files(char **file_names) {
    int counter;
    for (counter = 0; counter < MAXFILES; counter++) {
        if (file_names[counter] == NULL) {
            break;
        }
    }
    return counter;
}


/* Complete this function for Task 1. Including fixing this comment.
*/
FreqRecord *get_word(char *word, Node *head, char **file_names) {
    FreqRecord *fr_array = NULL;
    Node *nodep_cur = head;
    unsigned char found = 0;
    while (nodep_cur != NULL) {
        if (strcmp(nodep_cur->word, word) == 0) {
            found = 1;
            int num_of_files = get_num_of_files(file_names);
            fr_array = malloc(sizeof(FreqRecord) * (num_of_files + 1));
            for (int i = 0; i < num_of_files; i++) {
                fr_array[i].freq = (nodep_cur->freq)[i];
                // Sweet, tested on midterm 18f.
                strncpy(fr_array[i].filename, file_names[i], PATHLENGTH);
            }
            fr_array[num_of_files].freq = 0;
            (fr_array[num_of_files].filename)[0] = '\0';
            break;
        }
        nodep_cur = nodep_cur->next;
    }
    if (found == 0) {
        // No word is found in the index
        FreqRecord *ret = malloc(sizeof(FreqRecord));
        ret->freq = 0;
        (ret->filename)[0] = '\0';
        return ret;
    }
    return fr_array;
}

/* Print to standard output the frequency records for a word.
* Use this for your own testing and also for query.c
*/
void print_freq_records(FreqRecord *frp) {
    int i = 0;

    while (frp != NULL && frp[i].filename[0] != '\0') {
        printf("%d    %s\n", frp[i].freq, frp[i].filename);
        i++;
    }
}

/* Complete this function for Task 2 including writing a better comment.
*/
void run_worker(char *dirname, int in, int out) {
    Node *head = NULL;
    char **filenames = init_filenames();

    char index_path[strlen(dirname)+1+strlen("index")+1];
    char filenames_path[strlen(dirname)+1+strlen("filenames")+1];
    strcpy(index_path, dirname);
    strcat(index_path, "/index");
    // printf("[info] %s\n", index_path);
    strcpy(filenames_path, dirname);
    strcat(filenames_path, "/filenames");

    read_list(index_path, filenames_path, &head, filenames);
    int num_of_files = get_num_of_files(filenames);

    char word_buf[MAXWORD];
    int ret;
    while ((ret = read(in, word_buf, MAXWORD)) != 0) {
        if (ret == -1) {
            perror("read");
            exit(1);
        }
        word_buf[ret] = '\0';
        FreqRecord *fr_array = get_word(word_buf, head, filenames);
        for (int i = 0; i < num_of_files + 1; i++) {
            // Must write num_of_files+1 number of structs
            if ((fr_array[i].freq != 0 && (fr_array[i].filename)[0] != '\0') ||
                (fr_array[i].freq == 0 && (fr_array[i].filename)[0] == '\0')) {
                if (write(out, fr_array+i, sizeof(FreqRecord)) == -1) {
                    perror("run_worker->write");
                    exit(1);
                }
            }
        }
    }
    return;
}
