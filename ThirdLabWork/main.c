/*
 Написать программу поиска заданной пользователем комбинации из m байт (m <255) во
 всех файлах заданного каталога. Главный процесс открывает каталог и запускает для каждого
 файла каталога отдельный процесс поиска заданной комбинации из m байт. Каждый процесс выводит на
 экран свой pid, полный путь к файлу, общее число просмотренных байт и результаты (сколько раз
 найдена комбинация) поиска (все в одной строке). Число одновременно работающих процессов не
 должно превышать N (вводится пользователем). Проверить работу программы для каталога /etc
 и строки «ifconfig».
 */

#include <wait.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <bits/types/FILE.h>
#include <fcntl.h>
#include <locale.h>

#include "dynamic_list.h"

char *SEQUENCE;

void readDirectory(char *directoryPath, int *currentThreadsAmount, int maxThreadsAmount, DynList *childThreadsList, char *executableName);
void checkThreadsAmount(int *currentThreadsAmount, int maxThreadsAmount, const char *executableName);
void findBytes(char *filePath, char *sequence, char *executableName);

int main(int argc, char *argv[]) {

    setlocale(LC_ALL, "");

    char *executableName = basename(argv[0]);

    if (executableName == NULL)
    {
        fprintf(stderr, "Error with executable file \"%s\"", argv[0]);
        return 1;
    }

    if (argc < 4)
    {
        fprintf(stderr, "%d %s: Error: not enough command line arguments\n", getpid(), executableName);
    }

    char *primaryDirectoryPath = realpath(argv[1], NULL);

    if (primaryDirectoryPath == NULL)
    {
        fprintf(stderr,"%d: %s %s %s", getpid(), executableName, argv[1], strerror(errno));
        return 1;
    }

    SEQUENCE = argv[2];

    if (strlen(SEQUENCE) >= 255)
    {
        fprintf(stderr, "%d: %s:  Error:the length of sequence must be less than 255 charaCTERS\n", getpid(), executableName);
        return 1;
    }

    int maxThreadsAmount = atoi(argv[3]);

    if (maxThreadsAmount <= 1)
    {
        fprintf(stderr, "%d: %s:  Error: N must be greater than 1\n", getpid(), executableName);
        return 1;
    }

    int currentThreadAmount = 1;
    DynList *childThreadList = createList();

    readDirectory(primaryDirectoryPath, &currentThreadAmount, maxThreadsAmount, childThreadList, executableName);

    for (int i = 0; i < childThreadList->size; i++ )
    {
        int pid = getFromList(childThreadList, i);

        if (pid == -1)
        {
            continue;
        }

        waitpid(pid, 0, 0);
    }
    return 0;
}

void readDirectory(char *directoryPath, int *currentThreadsAmount, int maxThreadsAmount, DynList *childThreadsList, char *executableName)
{
    DIR *directory = opendir(directoryPath);

    if (directory == NULL)
    {
        fprintf(stderr, "%d: %s: %s %s\n", getpid(), executableName, directoryPath, strerror(errno));
        return;
    }

    struct dirent *d;

    while ((d = readdir(directory)) != NULL)
    {
        if (!strcmp(".", d->d_name) || !strcmp("..", d->d_name))
        {
            continue;
        }

        int size = strlen(directoryPath) + strlen(d->d_name) + 1;
        char *buffer = (char*)malloc(size*sizeof(char) + 1);
        if (buffer == NULL)
        {
            fprintf(stderr, "%d: %s: %s %s\n", getpid(), executableName, directoryPath, strerror(errno));
            continue;
        }

        strcpy(buffer, directoryPath);
        strcat(buffer, "/");
        strcat(buffer, d->d_name);

        if (d->d_type == DT_DIR)
        {
            readDirectory(buffer, currentThreadsAmount, maxThreadsAmount, childThreadsList, executableName);
        }
        else if (d->d_type == DT_REG)
        {
            checkThreadsAmount(currentThreadsAmount, maxThreadsAmount, executableName);

            int pid = fork();

            if (pid == 0)
            {

                FILE *file = fopen(buffer,"r");

                if (file == NULL)
                {
                    fprintf(stderr, "%d: %s: %s %s\n", getpid(), executableName, buffer, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                findBytes(buffer, SEQUENCE, executableName);

                int returnCode = fclose(file);

                if (returnCode != 0)
                {
                    fprintf(stderr, "%d: %s: %s %s\n", getpid(), executableName, buffer, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                exit(EXIT_SUCCESS);
            }
            else
            {
                (*currentThreadsAmount)++;
                addToList(childThreadsList, pid);

                if (childThreadsList == NULL)
                {
                    fprintf(stderr, "%d: %s: %s %s\n", getpid(), executableName, buffer, strerror(errno));
                    free(buffer);
                    continue;
                }
            }
        }

        free(buffer);
    }

    int returnCode = closedir(directory);

    if (returnCode == -1 )
    {
        fprintf(stderr, "%d: %s: %s %s\n", getpid(), executableName, directoryPath, strerror(errno));
    }

    return;
}


void findBytes(char *filePath, char *sequence,char *executableName)
{

    int fd = open(filePath, O_RDONLY);

    if (fd == -1)
    {
        fprintf(stderr, "%d: %s: %s %s\n", getpid(), executableName, filePath, strerror(errno));
        return;
    }

    int i = 0;
    int entriesCount = 0;

    ssize_t totalBytes = 0;
    size_t sequenceLength = strlen(sequence);

    char c = 0;

    ssize_t bytesRead;

    while ((bytesRead = read(fd,&c, sizeof(char))) > 0)
    {
        totalBytes++;

        (sequence[i] == c) ? (i++) : (i = 0);
        if (sequenceLength == i)
        {
            entriesCount++;
            i = 0;
        }
    }

    if (bytesRead == -1)
    {
        fprintf(stderr, "%d: %s: %s %s\n", getpid(), executableName, filePath, strerror(errno));
        return;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "%d: %s: %s %s\n", getpid(), executableName, filePath, strerror(errno));
        return;
    }

    if (entriesCount != 0)
    {
        fprintf(stdout, "%d: %s: %s %ld %d\n", getpid(), executableName, filePath, totalBytes, entriesCount);
    }
}

void checkThreadsAmount(int *currentThreadsAmount, int maxThreadsAmount, const char *executableName)
{
    if ((*currentThreadsAmount) == maxThreadsAmount) {
        int status = -1;
        int exitedPid = wait(&status);

        while (exitedPid == -1) {
            fprintf(stderr, "%d: %s: %s\n", getpid(), executableName, strerror(errno));
            exitedPid = wait(&status);
        }

        (*currentThreadsAmount)--;
    }
}
