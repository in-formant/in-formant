#include "file_logger.h"

#if defined(ANDROID) || defined(__ANDROID__)

#elif defined(__linux__)

#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/prctl.h>
#include <string.h>
#include <time.h>

static void appendToLine(char *linebuf, const char *str, size_t strsz)
{
    if (linebuf[0] == '\0') {
        time_t timer;
        struct tm* tm_info;

        timer = time(NULL);
        tm_info = localtime(&timer);

        strftime(linebuf, 23, "[%Y-%m-%d %H:%M:%S] ", tm_info);
    }

    strncat(linebuf, str, strsz);
}

void openFileLogger(const char *name) {
    char filename[32];
    snprintf(filename, 32, "%s.log", name);

    int fd = open(filename, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    
    if (fd == -1) {
        perror("open");
        return;
    }

    int pipefd[2];
    pipe(pipefd);

    pid_t ppid = getpid();
    pid_t cpid = fork();
    if (cpid == -1) {
        perror("fork");
        return;
    }

    if (cpid == 0) {
        int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (r == -1) {
            perror("prctl");
            exit(EXIT_FAILURE);
        }
        if (getppid() != ppid) {
            exit(EXIT_FAILURE);
        }

        close(pipefd[1]);

        char linebuf[1024];
        linebuf[0] = '\0';

        int count;
        char rdbuf[33];
        while (getppid() == ppid) {
            count = read(pipefd[0], rdbuf, 32);
            if (count == -1) {
                perror("read");
                break;
            }
            if (count == 0) {
                usleep(10000);
                continue;
            }
            
            rdbuf[count] = '\0';

            char *pch;
            pch = (char *) memchr(rdbuf, '\n', count);
            
            if (pch == NULL) {
                appendToLine(linebuf, rdbuf, count);
            }
            else {
                appendToLine(linebuf, rdbuf, pch - rdbuf + 1);
                write(STDOUT_FILENO, linebuf, strnlen(linebuf, 1024));
                write(fd, linebuf, strnlen(linebuf, 1024));
                linebuf[0] = '\0';

                pch++;
                count -= pch - rdbuf;

                char *pch2;
                while (count > 0 && (pch2 = (char *) memchr(pch, '\n', count)) != NULL) {
                    appendToLine(linebuf, pch, pch2 - pch + 1);
                    write(STDOUT_FILENO, linebuf, strnlen(linebuf, 1024));
                    write(fd, linebuf, strnlen(linebuf, 1024));
                    linebuf[0] = '\0';

                    pch = pch2 + 1;
                    count -= pch2 - pch;
                }

                appendToLine(linebuf, pch, count);
            }

            usleep(10000);
        }

        close(fd);
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }
    else {
        close(fd);
        close(pipefd[0]);
        
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
    }
}

#elif defined(_WIN32)

#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include <thread>

static void appendToLine(char *linebuf, const char *str, size_t strsz)
{
    if (linebuf[0] == '\0') {
        time_t timer;
        struct tm* tm_info;

        timer = time(NULL);
        tm_info = localtime(&timer);

        strftime(linebuf, 23, "[%Y-%m-%d %H:%M:%S] ", tm_info);
    }

    strncat(linebuf, str, strsz);
}

void openFileLogger(const char *name) {
    char filename[256];
    snprintf(filename, 256, "%s.log", name);

    HANDLE logFile = CreateFileA(
            filename,
            FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

    if (logFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Unable to open log file.\n");
        return;
    }

    int fd = _open_osfhandle((intptr_t) logFile, O_WRONLY | O_APPEND | O_TEXT);
    _close(1);
    _close(2);
    _dup2(fd, 1);
    _dup2(fd, 2);
}

#endif
