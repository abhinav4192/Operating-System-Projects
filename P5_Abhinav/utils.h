#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "p5_defs.h"

void errorExit() {
    exit(1);
}

void successExit() {
    exit(0);
}

int getFileDescriptor(int argc, char *argv[]) {
    // Basic input args validation
    if ((argc != 2) && (argc != 3)) {
        fprintf(stderr, "%s", Usage);
        errorExit();
    }
    if (argc == 3) {
        if ((strcmp(argv[1], "-r") == 0) || (strcmp(argv[2], "-r") == 0)) {
            fprintf(stderr, "%s", "Repair mode not implemented\n");
            errorExit();
        } else {
            fprintf(stderr, "%s", Usage);
            errorExit();
        }
    }

    // Open file system image and return file descriptor
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "%s", Nonexistant);
        errorExit();
    }
    return fd;
}

void* getMmapImage(int fd) {
    struct stat buf;
    int rc = -1;
    rc = fstat(fd, &buf);
    if (rc != 0) {
        fprintf(stderr, "Cannot read file stats\n");
        errorExit();
    }

    void *fs_ptr = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (fs_ptr == MAP_FAILED) {
        if (rc == 0) {
            fprintf(stderr, "Mapping to memory space failed\n");
            errorExit();
        }
    }
    return fs_ptr;
}

void dataBlockAddValidCheck(int blockNum, int minBlockNumber, int maxBlockNumber, const char *error) {
    if ((blockNum < minBlockNumber) || (blockNum >= maxBlockNumber)) {
        fprintf(stderr, "%s", error);
        errorExit();
    }
}

void addressRepeatCheck(int *addrUsed, int addrCurr) {
    if (addrUsed[addrCurr] == 1) {
        fprintf(stderr, "%s", Addronce);
        errorExit();
    } else {
        addrUsed[addrCurr] = 1;
    }
}

void checkBitmapFree(int *bitmapUsed, int addrCurr) {
    if (bitmapUsed[addrCurr] != 1) {
        fprintf(stderr, "%s", Mrkfree);
        errorExit();
    } else {
        bitmapUsed[addrCurr] = 0;
    }
}

void checkBitMapUsed(int *bitmapUsed, int totalBlocks) {
    for (int i = 0; i < totalBlocks; i++) {
        if (bitmapUsed[i] == 1) {
            fprintf(stderr, "%s", Mrkused);
            errorExit();
        }
    }
}

int checkSelfLink(struct xv6_dirent *dir_ptr, int inum) {
    if (strcmp(dir_ptr->name, ".") == 0) {
        if (dir_ptr->inum != inum) {
            fprintf(stderr, "%s", Badfmt);
            errorExit();
        }
        return 1;
    }
    return 0;
}

int checkParentLink(struct xv6_dirent *dir_ptr, int inum) {
    if (strcmp(dir_ptr->name, "..") == 0) {
        if ((inum == 1) && (dir_ptr->inum != 1)) {
            fprintf(stderr, "%s", Badroot);
            errorExit();
        }
        return 1;
    }
    return 0;
}

void storeParent(struct xv6_dirent *dir_ptr, int *dirParent, int inum) {
    if (strcmp(dir_ptr->name, "..") == 0) {
        dirParent[inum] = dir_ptr->inum;
    }
}

void checkTracesBackToParent(int *dirPresent, int *dirParent, int  ninodes) {
    for (int i = 0; i < ninodes; i++) {
        if (dirPresent[i] > 0) {
            int *visited = (int *)malloc(ninodes * sizeof(int *));
            memset(visited, 0, ninodes * sizeof(visited[0]));
            int t = i;
            while (dirParent[t] != 1) {
                if (visited[t] == 1) {
                    free(visited);
                    fprintf(stderr, "%s", IDirE);
                    errorExit();
                } else {
                    visited[t] = 1;
                }
                t = dirParent[t];
            }
            free(visited);
        }
    }
}

void checkInodeMarkedReferred(int *inodeMarked, int *inodeReferred, int *fileLinks, int  ninodes) {
    for (int i = 0; i < ninodes; i++) {
        if ((fileLinks[i] > 0) && (inodeReferred[i] != fileLinks[i])) {
            fprintf(stderr, "%s", Badrefcnt);
            errorExit();
        }
        if ((inodeMarked[i] == 0) && (inodeReferred[i] == 0)) continue;
        else if ((inodeMarked[i] > 0) && (inodeReferred[i] > 0)) continue;
        else if ((inodeMarked[i] > 0) && (inodeReferred[i] == 0)) {
            fprintf(stderr, "%s", Imrkused);
            errorExit();
        } else if ((inodeMarked[i] == 0) && (inodeReferred[i] > 0)) {
            fprintf(stderr, "%s", Imrkfree);
            errorExit();
        }
    }
}

void addToDirReferences(int *dirReferences, struct xv6_dirent *dir_ptr) {
    if ((strcmp(dir_ptr->name, "..") != 0) && (strcmp(dir_ptr->name, ".") != 0)) {
        dirReferences[dir_ptr->inum]++;
    }
}

void checkExtraDirLinks(int *dirPresent, int *dirReferences, int  ninodes) {
    for (int i = 0; i < ninodes; i++) {
        if (dirPresent[i] > 0) {
            if (dirReferences[i] > 1) {
                fprintf(stderr, "%s", Dironce);
                errorExit();
            }
        }
    }
}
