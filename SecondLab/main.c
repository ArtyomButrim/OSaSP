/*
 Найти совпадающие по содержимому файлы в двух заданных каталогах
 (аргументы 1 и 2 командной строки) и всех их подкаталогах. Вывести на
 консоль и в файл (аргумент 3 командной строки) полный путь, размер, дату
 создания, права доступа, номер индексного дескриптора.

 Использовать функции read, write, fread, fwrite (чтобы читать файл целиком).
 Создавать буфер на несколько мегабайт. Проверять каждую функцию на возникновение ошибок.
 Динамически выделять память.
 */

#include "main.h"

#define BUFFER_SIZE (32*1024*1024)
#define STR_SIZE 512

int main(int argc, char *argv[]) {

    char *locale = setlocale(LC_ALL, "ru_RU.utf8");

    char *executableName = basename(argv[0]);

    if (executableName == NULL)
    {
        fprintf(stderr,"Error with executable file \"%s\"", argv[0]);
        return 1;
    }

    if (argc <4) {
        fprintf(stderr, "%s: Error: not enough command line arguments\n", executableName);
        return 1;
    }

    char *firstPrimaryDirectoryPath = realpath(argv[1],NULL);

    if (firstPrimaryDirectoryPath == NULL) {
        fprintf(stderr,"%s: %s %s", executableName, argv[1], strerror(errno));
        return 1;
    }

    char *secondPrimaryDirectoryPath = realpath(argv[2], NULL);

    if (secondPrimaryDirectoryPath == NULL) {
        fprintf(stderr,"%s: %s %s", executableName, argv[2], strerror(errno));
        return 1;
    }

    firstPrimaryDirectoryPath = strcat(firstPrimaryDirectoryPath, "/");
    secondPrimaryDirectoryPath = strcat(secondPrimaryDirectoryPath, "/");

    char *resultFile = argv[3];
    ArrayList *firstFileList = createList();
    ArrayList *secondFileList = createList();

    if (firstFileList == NULL) {
        fprintf(stderr, "%s: Error: can't create list of files\n", executableName);
        return 1;
    }

    if (secondFileList == NULL) {
        fprintf(stderr, "%s: Error: can't create list of files\n", executableName);
        return 1;
    }

    readDirectory(firstPrimaryDirectoryPath, firstFileList, executableName);
    readDirectory(secondPrimaryDirectoryPath, secondFileList, executableName);
    FILE *results;
    int firstFileAmount = firstFileList->size;
    int secondFileAmount = secondFileList->size;

    results = fopen(resultFile,"w");

    if (results == NULL) {
        fprintf(stderr, "%s : %s : %s\n", executableName, resultFile, strerror(errno));
        return 1;
    }
    fclose(results);

    results = fopen(resultFile,"a");

    if (results == NULL) {
        fprintf(stderr, "%s : %s : %s\n", executableName, resultFile, strerror(errno));
        return 1;
    }

    for (int i = 0; i < firstFileAmount; i++) {
        char *firstFilePath = getFromList(firstFileList,i);

        if (firstFilePath == NULL) {
            continue;
        }

        bool isExitOutLoop = false;
        for (int j =0; j< secondFileAmount; j++)
        {
            int firstFileBufferAmount = 0;
            int secondFileBufferAmount = 0;
            FileInfo *firstFileInfo = NULL;
            FileInfo *secondFileInfo = NULL;
            char *secondFilePath = NULL;
            bool isExitLoop = false;
            bool areEqual = false;

            do {
                firstFileInfo = getFileInfo(firstFilePath, &firstFileBufferAmount);

                if (firstFileInfo == NULL) {
                    fprintf(stderr, "%s: %s %s\n", executableName, firstFilePath, strerror(errno));
                    isExitLoop =true;
                    isExitOutLoop = true;
                    break;
                }

                secondFilePath =getFromList(secondFileList,j);
                if (secondFilePath == NULL) {
                    isExitLoop = true;
                    break;
                }

                secondFileInfo = getFileInfo(secondFilePath, &secondFileBufferAmount);

                if (secondFileInfo == NULL) {
                    isExitLoop = true;
                    break;
                }

                areEqual = areFilesEqual(firstFileInfo, secondFileInfo);

                if (!areEqual) {
                    free(firstFileInfo->fileContent);
                    free(secondFileInfo->fileContent);
                    free(firstFileInfo->creationDate);
                    free(secondFileInfo->creationDate);
                    free(firstFileInfo);
                    free(secondFileInfo);
                }
            } while (firstFileBufferAmount != 0 && secondFileBufferAmount != 0 && !isExitLoop && areEqual);

            if (isExitOutLoop) {
                break;
            }

            if (isExitLoop) {
                continue;
            }

            if (areEqual) {
                fprintf(stdout, "%s %ld %s %lo %ld", firstFilePath, firstFileInfo->fileSize, firstFileInfo->creationDate, firstFileInfo->accessType, firstFileInfo->indexNumberDescriptor);
                fprintf(stdout," == ");
                fprintf(stdout, "%s %ld %s %lo %ld\n", secondFilePath, secondFileInfo->fileSize, secondFileInfo->creationDate, secondFileInfo->accessType, secondFileInfo->indexNumberDescriptor);

                fprintf(results, "%s %ld %s %lo %ld == %s %ld %s %lo %ld\n", firstFilePath, firstFileInfo->fileSize, firstFileInfo->creationDate, firstFileInfo->accessType, firstFileInfo->indexNumberDescriptor, secondFilePath, secondFileInfo->fileSize, secondFileInfo->creationDate, secondFileInfo->accessType, secondFileInfo->indexNumberDescriptor);
                free(firstFileInfo->fileContent);
                free(secondFileInfo->fileContent);
                free(firstFileInfo->creationDate);
                free(secondFileInfo->creationDate);
                free(firstFileInfo);
                free(secondFileInfo);
            }
        }

        if (isExitOutLoop) {
            continue;
        }
    }

    free(firstFileList);
    free(secondFileList);
    fclose(results);
    return 0;
}

void readDirectory(char *directoryPath, ArrayList *fileList, char *executableName) {
    struct stat fileInfo;
    int returnCode = lstat(directoryPath, &fileInfo);

    if (returnCode == EOF) {
        fprintf(stderr, "%s: %s %s\n", executableName, directoryPath, strerror(errno));
        return;
    }

    if (!S_ISDIR(fileInfo.st_mode)) {
        fprintf(stderr, "%s: Error: \"%s\" id not a directory\n", executableName, directoryPath);
        return;
    }

    DIR *directory = opendir(directoryPath);

    if (directory == NULL) {
        fprintf(stderr, "%s: %s %s\n", executableName, directoryPath, strerror(errno));
        return;
    }

    struct dirent *directoryInfo;

    while ((directoryInfo =readdir(directory)) != NULL) {
        if (strcmp(directoryInfo->d_name, ".") == 0 || strcmp(directoryInfo->d_name, "..") == 0) {
            continue;
        }

        unsigned long pathLength = strlen(directoryPath) + strlen(directoryInfo->d_name) +1;
        char *filePath = malloc(pathLength * sizeof(char));
        memset(filePath,0,pathLength);

        strcat(strcat(filePath, directoryPath), directoryInfo->d_name);

        returnCode = lstat(filePath, &fileInfo);

        if (returnCode == EOF) {
            fprintf(stderr, "%s: %s %s\n", executableName, filePath, strerror(errno));
            continue;
        }

        if (S_ISREG(fileInfo.st_mode)) {
            returnCode = addToList(fileList, filePath);

            if (returnCode == -1) {
                fprintf(stderr, "%s: Error: can't add element \"%s\" to files list\n", executableName, filePath);
                return;
            }
        }
        else if (S_ISDIR(fileInfo.st_mode)) {
            filePath = realpath(filePath, NULL);

            if (filePath == NULL) {
                return;
            }

            strcat(filePath,"/");
            readDirectory(filePath, fileList, executableName);
        }
        else {
            continue;
        }
    }

    returnCode = closedir(directory);

    if (returnCode == -1) {
        fprintf(stderr, "%s: %s %s\n", executableName, directoryPath, strerror(errno));
    }
}

FileInfo *getFileInfo (char *filePath, int *buffersAmount) {
    char *string;
    struct tm lt;
    struct stat fileInfo;
    int returnCode = lstat(filePath, &fileInfo);

    if (returnCode ==EOF) {
        return NULL;
    }

    if (!S_ISREG(fileInfo.st_mode)) {
        return NULL;
    }

    FILE *file = fopen(filePath, "r");

    if (file == NULL) {
        return NULL;
    }

    int bufferSize = BUFFER_SIZE;

    if (fileInfo.st_size - (*buffersAmount) * BUFFER_SIZE <= BUFFER_SIZE) {
        bufferSize = (int) fileInfo.st_size - (*buffersAmount) * BUFFER_SIZE;
    }

    char *fileContent = malloc(bufferSize);

    if (fileContent == NULL) {
        fclose(file);
        return NULL;
    }

    returnCode = fseek(file,(*buffersAmount) * bufferSize, SEEK_SET);

    if (returnCode != 0) {
        fclose(file);
        free(fileContent);
        return NULL;
    }

    size_t readSize = fread(fileContent, 1, bufferSize, file);

    if (readSize != bufferSize) {
        fclose(file);
        free(fileContent);
        return NULL;
    }

    string = (char *)malloc(sizeof(char)*STR_SIZE);

    if (string == NULL) {
        fclose(file);
        return NULL;
    }

    localtime_r(&fileInfo.st_mtime,&lt);
    strftime(string,sizeof(char*)*STR_SIZE,"%d.%m.%Y", &lt);

    returnCode = fclose(file);

    if (returnCode == EOF) {
        free(fileContent);
        return NULL;
    }

    FileInfo *fileInformation = malloc(sizeof(FileInfo));
    fileInformation->fileSize = bufferSize;
    fileInformation->fileContent = fileContent;
    fileInformation->accessType = (unsigned long)fileInfo.st_mode;
    fileInformation->indexNumberDescriptor = (long)fileInfo.st_ino;
    fileInformation->creationDate = string;

    if (bufferSize == BUFFER_SIZE) {
        (*buffersAmount)++;
    }
    else
    {
        *buffersAmount = 0;
    }

    return fileInformation;
}

bool areFilesEqual(FileInfo *firstFileInfo, FileInfo *secondFileInfo) {
    if (firstFileInfo->fileSize != secondFileInfo->fileSize) {
        return false;
    }

    size_t fileSize = firstFileInfo->fileSize;

    return memcmp(firstFileInfo->fileContent, secondFileInfo->fileContent, fileSize) == 0;
}
