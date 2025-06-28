#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ACTUAL_READ_BUFFER_SIZE 8192

static void doCat(const char *p, const char *);
static void die(const char *s);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "%s keyword [file1 file2 ...] must be given\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *keyword = argv[1];

    if (argc == 2) {
        doCat(NULL, keyword);
    } else {
        for (int i = 2; i < argc; i++) {
            doCat(argv[i], keyword);
        }
    }
    exit(EXIT_SUCCESS);
}

static void doCat(const char *path, const char *keyword) {
    int fd;
    char buf[ACTUAL_READ_BUFFER_SIZE + 1];

    ssize_t n;
    size_t total = 0; // Length of unhandled data at buf's start

    off_t current_file_offset = 0; // Total bytes read from start of file/stream

    if (path == NULL) {
        fd = STDIN_FILENO;
    } else {
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            die(path);
            return;
        }
    }

    while (1) {
        // Store the offset BEFORE the current read operation.
        // This is crucial for correctly calculating line offsets.
        off_t offset_before_read = current_file_offset; 

        // Read data into the buffer, starting after any unhandled data.
        n = read(fd, buf + total, ACTUAL_READ_BUFFER_SIZE - total);

        if (n == -1) {
            die(path ? path : "stdin");
        }
        if (n == 0) { // EOF
            break;
        }

        // Update the total file offset with the bytes just read.
        current_file_offset += n;

        size_t current_len = total + n;
        buf[current_len] = '\0';

        char *current_pos = buf;
        char *newline_pos;

        while ((newline_pos = strchr(current_pos, '\n')) != NULL) {
            *newline_pos = '\0';

            if (strstr(current_pos, keyword) != NULL) {
                if (path != NULL) {
                    if (write(STDOUT_FILENO, path, strlen(path)) < 0) die(path);
                    if (write(STDOUT_FILENO, ":", 1) < 0) die(path);
                }
                if (write(STDOUT_FILENO, current_pos, strlen(current_pos)) < 0) die(path);
                if (write(STDOUT_FILENO, "\n", 1) < 0) die(path);

                // Calculate the true starting offset of the line.
                // It's the offset *before* this read, plus the relative position of the line start in the buffer.
                off_t keyword_offset = offset_before_read + (current_pos - buf);
                printf("  (%s found near offset: %ld bytes)\n", keyword, (long)keyword_offset);
            }
            current_pos = newline_pos + 1;
        }

        total = current_len - (current_pos - buf);
        if (total > 0) {
            memmove(buf, current_pos, total);
        } else {
            total = 0;
        }
    }

    // Handle the very last line if it doesn't end with a newline.
    if (total > 0) {
        if (strstr(buf, keyword) != NULL) {
            if (path != NULL) {
                if (write(STDOUT_FILENO, path, strlen(path)) < 0) die(path);
                if (write(STDOUT_FILENO, ":", 1) < 0) die(path);
            }
            if (write(STDOUT_FILENO, buf, strlen(buf)) < 0) die(path);
            if (write(STDOUT_FILENO, "\n", 1) < 0) die(path);

            // For the very last line, its offset is `current_file_offset - total`
            // because `current_file_offset` now includes all bytes read from the file.
            off_t keyword_offset = current_file_offset - total; 
            printf("  (%s found near offset: %ld bytes)\n", keyword, (long)keyword_offset);
        }
    }

    if (path != NULL) {
        if (close(fd) < 0) {
            die(path);
        }
    }
}

static void die(const char *s) {
    perror(s);
    exit(EXIT_FAILURE);
}
