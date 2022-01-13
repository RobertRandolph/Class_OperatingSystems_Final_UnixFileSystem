#include <unordered_map>
#include <string>
using namespace std;

#ifndef FILETABLE_H
#define FILETABLE_H

// An instance of an opened file
class FileHandle {
public:
    FileHandle(string fname, char m, int fblk):filename(fname), mode(m), rw(0), inode(fblk) {}
    string filename;
    char mode;
    int rw;
    int inode;
};

// An instance of a file
class File {
public:
    File(int o, int l):open(o), lock_id(l) {}
    int open;
    int lock_id;
};

// Holds all file handles and file information.
class FileTable {
    unordered_map<int, FileHandle> fileHandles;
    unordered_map<string, File> files;
    int id;
    int lock;

public:
    FileTable():id(1), lock(1) {}
    int openFile(char* filename, char m, int inode);
    bool closeFile(int filedesc);
    bool isLocked(char* filename);
    bool isOpen(char* filename);
    int isOpenable(char* filename, int lock_id);
    int lockFile(char* filename);
    bool unlockFile(char* filename, int lock_id);
    int getInode(int filedesc);
    char getMode(int filedesc);
    void incRW(int filedesc);
    void setRW(int filedesc, int val);
    bool isValidFileDesc(int filedesc);
    int getRW(int filedesc);
};

#endif
