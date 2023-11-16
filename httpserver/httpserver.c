#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>

#include "asgn2_helper_funcs.h"

#define BUFFER_SIZE 4096

#define PARSE_REQUEST                                                                              \
    "^([a-zA-Z]*) /([a-zA-Z.]*) (HTTP/[0-9].[0-9])\r\n(([a-zA-Z0-9.-]{0,128}: [ "                  \
    "-~]{0,128}\r\n)+)?\r\n"
#define PARSE_HEADER "(Content-Length): ([0-9]+)\r\n"

typedef struct {

    char buf[BUFFER_SIZE + 1];
    uint16_t bufsize;
    char command[10];
    char location[100];
    char version[10];
    char headers[2048];
    char key[200];
    char value[200];

} Command;

void response(int val, int fd, int len) {
    char buf[100];
    memset(buf, '\0', sizeof(buf));
    switch (val) {
    case 1:
        snprintf(buf, 100, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", len);
        write_n_bytes(fd, buf, strlen(buf));
        break;
    case 2:
        snprintf(buf, 100, "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK\n");
        write_n_bytes(fd, buf, strlen(buf));
        break;
    case 3:
        snprintf(buf, 100, "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n");
        write_n_bytes(fd, buf, strlen(buf));
        break;
    case 4:
        snprintf(buf, 100, "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n");
        write_n_bytes(fd, buf, strlen(buf));
        break;
    case 5:
        snprintf(buf, 100, "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n");
        write_n_bytes(fd, buf, strlen(buf));
        break;
    case 6:
        snprintf(buf, 100,
            "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n");
        write_n_bytes(fd, buf, strlen(buf));
        break;
    case 7:
        snprintf(buf, 100,
            "HTTP/1.1 505 Version Not Supported\r\nContent-Length: 22\r\n\r\nVersion Not "
            "Supported\n");
        write_n_bytes(fd, buf, strlen(buf));
        break;
    case 8:
        snprintf(buf, 100,
            "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 22\r\n\r\nInternal Server "
            "Error\n");
        write_n_bytes(fd, buf, strlen(buf));
        break;
    case 9:
        snprintf(buf, 100, "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n");
        write_n_bytes(fd, buf, strlen(buf));
        break;
    }
}

int read_until(int fd, char *buf, int n, char str[]) {

    int bytes_read = 0;
    int count = 0;
    char *check = NULL;
    do {
        if (bytes_read >= n) {
            response(5, fd, 0);
            return -1;
        }

        bytes_read = read(fd, buf + count, BUFFER_SIZE);
        check = strstr(buf, str);
        if (bytes_read < 0) {
            response(8, fd, 0);
            return -1;
        } else if (bytes_read == 0) {
            return count;
        }
        count += bytes_read;

    } while (check == NULL);
    return count;
}

void cmd_dump(Command *c) {

    fprintf(stderr, "Command: %s, %lu\n", c->command, strlen(c->command));
    fprintf(stderr, "Location: %s, %lu\n", c->location, strlen(c->location));
    fprintf(stderr, "Version: %s, %lu\n", c->version, strlen(c->version));
    fprintf(stderr, "Headers: %s, %lu\n", c->headers, strlen(c->headers));
    fprintf(stderr, "Key: %s, %lu\n", c->key, strlen(c->key));
    fprintf(stderr, "Value: %s, %lu\n", c->value, strlen(c->value));
}

int get(char *filename, int dest, int file_size) {

    int fd = open(filename, O_RDONLY);

    if (fd < 0) {
        response(4, dest, 0);
        return -1;
    }
    response(1, dest, file_size);
    pass_n_bytes(fd, dest, file_size);

    close(fd);
    return -1;
}

int set(char *filename, int dest, char *eoh, int remainder, int content_len) {

    if (access(filename, F_OK) != -1) {
        response(2, dest, 0);
    } else {
        response(3, dest, 0);
    }
    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0777);

    if (fd < 0) {
        response(8, dest, 0);
        return -1;
    }

    if (content_len <= remainder) {
        write_n_bytes(fd, eoh + 4, content_len);
        close(fd);
        return 0;
    } else if (content_len > remainder) {
        write_n_bytes(fd, eoh + 4, remainder);
        pass_n_bytes(dest, fd, content_len - remainder);
        close(fd);
        return 0;
    }
    close(fd);
    return 0;
}

int cmd_parse(Command *c, int fd) {

    memset(c->buf, '\0', sizeof(c->buf));
    memset(c->command, '\0', sizeof(c->command));
    memset(c->location, '\0', sizeof(c->location));
    memset(c->version, '\0', sizeof(c->version));
    memset(c->headers, '\0', sizeof(c->headers));
    memset(c->key, '\0', sizeof(c->key));
    memset(c->value, '\0', sizeof(c->value));

    regex_t re;
    regmatch_t first_matches[5];
    int rc;
    regmatch_t second_matches[3];
    int match_len = 0;

    c->bufsize = read_until(fd, c->buf, BUFFER_SIZE, "\r\n\r\n");
    if (c->bufsize > 0) {

        c->buf[c->bufsize] = '\0';

        rc = regcomp(&re, PARSE_REQUEST, REG_EXTENDED);
        assert(!rc);

        rc = regexec(&re, (char *) c->buf, 5, first_matches, 0);

        if (rc == 0) {
            match_len = first_matches[1].rm_eo - first_matches[1].rm_so;
            strncpy(c->command, c->buf + first_matches[1].rm_so, match_len);
            c->command[match_len] = '\0';

            match_len = first_matches[2].rm_eo - first_matches[2].rm_so;
            strncpy(c->location, c->buf + first_matches[2].rm_so, match_len);
            c->location[match_len] = '\0';

            match_len = first_matches[3].rm_eo - first_matches[3].rm_so;
            strncpy(c->version, c->buf + first_matches[3].rm_so, match_len);
            c->version[match_len] = '\0';

            match_len = first_matches[4].rm_eo - first_matches[4].rm_so;
            strncpy(c->headers, c->buf + first_matches[4].rm_so, match_len);
            c->headers[match_len] = '\0';

            if (strcmp(c->command, "GET") != 0 && strcmp(c->command, "PUT") != 0) {
                response(6, fd, 0);
                return -1;

            } else if (strcmp(c->version, "HTTP/1.1") != 0) {
                response(7, fd, 0);
                return -1;
            }

        } else {
            if (strlen(c->command) == 0) {
                response(5, fd, 0);
                return -1;

            } else if (strlen(c->location) == 0) {
                response(5, fd, 0);
                return -1;

            } else if (strlen(c->version) == 0) {
                response(5, fd, 0);
                return -1;
            }
        }
        regfree(&re);
        rc = regcomp(&re, PARSE_HEADER, REG_EXTENDED);
        assert(!rc);

        rc = regexec(&re, c->headers, 3, second_matches, 0);

        if (rc == 0) {

            match_len = second_matches[1].rm_eo - second_matches[1].rm_so;
            strncpy(c->key, c->headers + second_matches[1].rm_so, match_len);
            c->key[match_len] = '\0';

            match_len = second_matches[2].rm_eo - second_matches[2].rm_so;
            strncpy(c->value, c->headers + second_matches[2].rm_so, match_len);
            c->value[match_len] = '\0';
        }
        regfree(&re);

        if (strcmp(c->command, "GET") == 0) {

            struct stat fileStat;
            stat(c->location, &fileStat);

            if (S_ISDIR(fileStat.st_mode)) {
                response(9, fd, 0);
                return -1;
            }

            int file_size = fileStat.st_size;
            get(c->location, fd, file_size);

        } else if (strcmp(c->command, "PUT") == 0) {

            char *filename = c->location;
            char *eoh = strstr(c->buf, "\r\n\r\n");
            if (eoh == NULL) {
                response(5, fd, 0);
                return -1;
            }

            int bytes_processed = strlen(c->command) + strlen(c->location) + strlen(c->version)
                                  + strlen(c->headers) + 7;
            int to_write = c->bufsize - bytes_processed;
            char *ptr;
            long content_len = strtoul(c->value, &ptr, 10);
            set(filename, fd, eoh, to_write, content_len);
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Invalid Port\n");
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    Command c;
    (void) c;

    Listener_Socket socket;
    int valid = listener_init(&socket, port);
    if (valid < 0) {
        fprintf(stderr, "Invalid Port\n");
        exit(EXIT_FAILURE);
    }

    int socket_fd = 0;

    while (1) {
        socket_fd = listener_accept(&socket);
        cmd_parse(&c, socket_fd);
        close(socket_fd);
    }
    return 0;
}
