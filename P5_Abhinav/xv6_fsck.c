#include "utils.h"

// 1) Each inode is either unallocated or one of the valid types (T_FILE, T_DIR, T_DEV). ERROR: bad inode.
// 2) For in-use inodes, each address that is used by inode is valid (points to a valid datablock address
// within the
// image). If the direct block is used and is invalid, print ERROR: bad direct address in inode.; if the
// indirect block
// 5) For in-use inodes, each address in use is also marked in use in the bitmap. ERROR: address used by inode
// but marked free in bitmap.
// 6) For blocks marked in-use in bitmap, actually is in-use in an inode or indirect block somewhere. ERROR:
// bitmap marks block in use but it is not in use.
// is in use and is invalid, print ERROR: bad indirect address in inode
// 7) For in-use inodes, direct address in use is only used once. ERROR: direct address used more than once.
// 8) For in-use inodes, indirect address in use is only used once. ERROR: indirect address used more than
// once.
void dataBlockAddressChecks(void *fs_ptr, int ninodes, int totalBlocks) {
    long unsigned int minBlockNumber = BBLOCK(totalBlocks, ninodes) + 1;

    // 0 means not address is not used. 1 means used.
    int *addrUsed = (int *)malloc(totalBlocks * sizeof(int *));
    memset(addrUsed, 0, totalBlocks * sizeof(addrUsed[0]));

    // O means block is free. 1 means used
    int *bitmapUsed = (int *)malloc(totalBlocks * sizeof(int *));
    memset(bitmapUsed, 0, totalBlocks * sizeof(bitmapUsed[0]));

    char *bitmap_ptr      = (char *)(fs_ptr + BBLOCK(0, ninodes) * BSIZE);
    struct dinode *di_ptr = (struct dinode *)(fs_ptr + 2 * BSIZE);

    // Read data block bitmap.
    for (int i = minBlockNumber; i < totalBlocks; i++) {
        bitmapUsed[i] = (*(bitmap_ptr + i / 8) & bitmask[i % 8]) != 0;
    }

    for (int i = 0; i < ninodes; i++) {
        if ((di_ptr->type == T_FILE) || (di_ptr->type == T_DIR) || (di_ptr->type == T_DEV)) {
            for (int j = 0; j <= NDIRECT; j++) {
                if (di_ptr->addrs[j] != 0) {
                    if (j != NDIRECT) {
                        dataBlockAddValidCheck(di_ptr->addrs[j], minBlockNumber, totalBlocks, Badaddr);
                    } else {
                        dataBlockAddValidCheck(di_ptr->addrs[j], minBlockNumber, totalBlocks, Badindir);
                    }
                    addressRepeatCheck(addrUsed, di_ptr->addrs[j]);
                    checkBitmapFree(bitmapUsed, di_ptr->addrs[j]);
                }
            }
            if (di_ptr->addrs[NDIRECT] != 0) {
                uint *indirect_block = (uint *)(fs_ptr + di_ptr->addrs[NDIRECT] * BSIZE);
                for (int j = 0; j < NINDIRECT; j++) {
                    if (*indirect_block != 0) {
                        dataBlockAddValidCheck(*indirect_block, minBlockNumber, totalBlocks, Badindir);
                        addressRepeatCheck(addrUsed, *indirect_block);
                        checkBitmapFree(bitmapUsed, *indirect_block);
                    }
                    indirect_block++;
                }
            }
        } else if (di_ptr->type != T_UNUSED) {
            fprintf(stderr, "%s", Badinode);
            errorExit();
        }
        di_ptr++;
    }
    checkBitMapUsed(bitmapUsed, totalBlocks);
    free(addrUsed);
    free(bitmapUsed);
}

// 3) Root directory exists, its inode number is 1, and the parent of the root directory is itself. ERROR: root
// directory does not exist.
// 4) Each directory contains . and .. entries, and the . entry points to the directory itself. ERROR:
// directory not properly formatted.
// 9) For all inodes marked in use, must be referred to in at least one directory. ERROR: inode marked use but
// not found in a directory.
// 10) For each inode number that is referred to in a valid directory, it is actually marked in use. ERROR:
// inode referred to in directory but marked free.
// 11) Reference counts (number of links) for regular files match the number of times file is referred to in
// directories (i.e., hard links work correctly). ERROR: bad reference count for file.
// 12) No extra links allowed for directories (each directory only appears in one other directory). ERROR:
// directory appears more than once in file system.
void directoryChecks(void *fs_ptr, int ninodes) {
    int *inodeMarked = (int *)malloc(ninodes * sizeof(int *));
    memset(inodeMarked,   0, ninodes * sizeof(inodeMarked[0]));
    int *inodeReferred = (int *)malloc(ninodes * sizeof(int *));
    memset(inodeReferred, 0, ninodes * sizeof(inodeReferred[0]));
    int *fileLinks = (int *)malloc(ninodes * sizeof(int *));
    memset(fileLinks,     0, ninodes * sizeof(fileLinks[0]));
    int *dirPresent = (int *)malloc(ninodes * sizeof(int *));
    memset(dirPresent,    0, ninodes * sizeof(dirPresent[0]));
    int *dirReferences = (int *)malloc(ninodes * sizeof(int *));
    memset(dirReferences, 0, ninodes * sizeof(dirReferences[0]));
    int *dirParent = (int *)malloc(ninodes * sizeof(int *));
    memset(dirParent,     0, ninodes * sizeof(dirParent[0]));

    struct dinode *di_ptr = (struct dinode *)(fs_ptr + 2 * BSIZE);
    int rootExists        = 0;
    for (int i = 0; i < ninodes; i++) {
        if ((di_ptr->type == T_FILE) || (di_ptr->type == T_DIR) || (di_ptr->type == T_DEV)) inodeMarked[i]++;
        if (di_ptr->type == T_FILE) fileLinks[i] += di_ptr->nlink;
        if (di_ptr->type == T_DIR) {
            dirPresent[i]++;
            if (i == 1) rootExists = 1;
            int selfLink   = 0;
            int parentLink = 0;

            for (int j = 0; j < NDIRECT; j++) {
                if (di_ptr->addrs[j] != 0) {
                    struct xv6_dirent *dir_ptr = (struct xv6_dirent *)(fs_ptr + di_ptr->addrs[j] * BSIZE);
                    for (int z = 0; z < BSIZE / sizeof(struct xv6_dirent); z++) {
                        if (dir_ptr->inum > 0) {
                            inodeReferred[dir_ptr->inum]++;
                            if (selfLink == 0) selfLink = checkSelfLink(dir_ptr, i);
                            if (parentLink == 0) parentLink = checkParentLink(dir_ptr, i);
                            addToDirReferences(dirReferences, dir_ptr);
                            storeParent(dir_ptr, dirParent, i);
                        }
                        dir_ptr++;
                    }
                }
            }

            if (di_ptr->addrs[NDIRECT] != 0) {
                uint *indirect_block = (uint *)(fs_ptr + di_ptr->addrs[NDIRECT] * BSIZE);
                for (int j = 0; j < NINDIRECT; j++) {
                    if (*indirect_block != 0) {
                        struct xv6_dirent *dir_ptr = (struct xv6_dirent *)(fs_ptr + (*indirect_block) * BSIZE);
                        for (int z = 0; z < BSIZE / sizeof(struct xv6_dirent); z++) {
                            if (dir_ptr->inum > 0) {
                                inodeReferred[dir_ptr->inum]++;
                                if (selfLink == 0) selfLink = checkSelfLink(dir_ptr, i);
                                if (parentLink == 0) parentLink = checkParentLink(dir_ptr, i);
                                addToDirReferences(dirReferences, dir_ptr);
                                storeParent(dir_ptr, dirParent, i);
                            }
                            dir_ptr++;
                        }
                    }
                    indirect_block++;
                }
            }
            if ((selfLink != 1) || (parentLink != 1)) {
                fprintf(stderr, "%s", Badfmt);
                errorExit();
            }
        }
        di_ptr++;
    }
    if (rootExists == 0) {
        fprintf(stderr, "%s", Badroot);
        errorExit();
    }
    checkInodeMarkedReferred(inodeMarked, inodeReferred, fileLinks, ninodes);
    checkExtraDirLinks(dirPresent, dirReferences, ninodes);
    checkTracesBackToParent(dirPresent, dirParent, ninodes);
    free(inodeMarked);
    free(inodeReferred);
    free(fileLinks);
    free(dirPresent);
    free(dirReferences);
    free(dirParent);
}

int main(int argc, char *argv[]) {
    int   fd                  = getFileDescriptor(argc, argv);
    void *fs_ptr              = getMmapImage(fd);
    struct superblock *sb_ptr = (struct superblock *)(fs_ptr + BSIZE);

    dataBlockAddressChecks(fs_ptr, sb_ptr->ninodes, sb_ptr->size);
    directoryChecks(fs_ptr, sb_ptr->ninodes);
    close(fd);
    successExit();
    return 0;
}
