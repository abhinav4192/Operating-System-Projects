const char *Addronce    = "ERROR: direct address used more than once.\n";
const char *Badaddr     = "ERROR: bad direct address in inode.\n";
const char *Badfmt      = "ERROR: directory not properly formatted.\n";
const char *Badindir    = "ERROR: bad indirect address in inode.\n";
const char *Badinode    = "ERROR: bad inode.\n";
const char *Badrefcnt   = "ERROR: bad reference count for file.\n";
const char *Badroot     = "ERROR: root directory does not exist.\n";
const char *Dironce     = "ERROR: directory appears more than once in file system.\n";
const char *Imrkfree    = "ERROR: inode referred to in directory but marked free.\n";
const char *Imrkused    = "ERROR: inode marked use but not found in a directory.\n";
const char *Mismatch    = "ERROR: parent directory mismatch.\n";
const char *Mrkfree     = "ERROR: address used by inode but marked free in bitmap.\n";
const char *Mrkused     = "ERROR: bitmap marks block in use but it is not in use.\n";
const char *Nonexistant = "image not found.\n";
const char *Usage       = "Usage: xv6_fsck <file_system_image>\n";
const char *IDirE       = "ERROR: inaccessible directory exists.\n";
const int   bitmask[8]  = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

#define T_UNUSED 0
#define T_DIR  1  // Directory
#define T_FILE 2  // File
#define T_DEV  3  // Special device

#define ROOTINO 1 // root i-number
#define BSIZE 512 // block size

// File system super block
struct superblock {
    uint size;    // Size of file system image (blocks)
    uint nblocks; // Number of data blocks
    uint ninodes; // Number of inodes.
};

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
    short type;               // File type
    short major;              // Major device number (T_DEV only)
    short minor;              // Minor device number (T_DEV only)
    short nlink;              // Number of links to inode in file system
    uint  size;               // Size of file (bytes)
    uint  addrs[NDIRECT + 1]; // Data block addresses
};

#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i) ((i) / IPB + 2)

// Bitmap bits per block
#define BPB           (BSIZE * 8)

// Block containing bit for block b
#define BBLOCK(b, ninodes) (b / BPB + (ninodes) / IPB + 3)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct xv6_dirent {
    ushort inum;
    char   name[DIRSIZ];
};
