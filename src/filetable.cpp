#include "filetable.h"
#include <iostream>
#include <unordered_map>
#include <string>
using std::unordered_map;
using std::string;

// Returns the filedesc of the newly created file handle
int FileTable::openFile(char* filename, char m, int inode) {
    int filedesc = id++;            // File handle ID
    string fname = filename;        // File name
    FileHandle fh(fname, m, inode); // File handle

    // Adding file handle
    fileHandles.insert({filedesc, fh});
    setRW(filedesc, 0);

    // Incrimenting openFile count
    // Checking if file instance exists
    if (files.find(fname) == files.end()) {
        // File instance doesn't exist
        // Creating new one
        File f(1, -1);
        files.insert({fname, f});
    }
    else files.at(fname).open += 1;

    return filedesc;
}

// Returns true if the file handle was successfully closed
bool FileTable::closeFile(int filedesc) {
    // Checking if filedesc exists
    if (fileHandles.find(filedesc) == fileHandles.end()) return false;

    // Closing file handle
    string fname = fileHandles.at(filedesc).filename;
    fileHandles.erase(filedesc);

    // Decrimenting openFile count
    files.at(fname).open -= 1;
    if (files.at(fname).open == 0 && files.at(fname).lock_id == -1)
        files.erase(fname);

    // File handle successfully closed
    return true;
}

// Returns true if the file is locked
bool FileTable::isLocked(char* filename) {
    string fname = filename;

    // Checking if file instance exists
    if (files.find(fname) == files.end()) return false; // File not locked
    if (files.at(fname).lock_id == -1) return false;    // File not locked

    // File is locked
    return true;
}

// Returns true if the file is open
bool FileTable::isOpen(char* filename) {
    string fname = filename;

    // Checking if file instance is open
    if (files.find(fname) == files.end()) return false; // File not open
    if (files.at(fname).open == 0) return false;        // File not open

    // File is open
    return true;
}

// Returns 0 if the file is not locked
// Returns 1 if the file lock is passed
// Returns -1 if the file isn't openable
int FileTable::isOpenable(char* filename, int lock_id) {
    string fname = filename;

    // Checking if file is locked
    if (!isLocked(filename)) return 0;  // File isn't locked

    // File is locked
    // Checking if lock_id's match
    if (files.at(fname).lock_id == lock_id) return 1;

    // File isn't openable
    return -1;
}

// Locks a given filename
// Returns the lock_id key used to lock the file, in order to unlock it
int FileTable::lockFile(char* filename) {
    string fname = filename;
    int lock_id = lock++;

    // Checking if file instance exists
    if (files.find(fname) == files.end()) {
        // File instance doesn't exist
        // Creating new one
        File f(0, lock_id);
        files.insert({fname, f});
    }
    else
        files.at(fname).lock_id = lock_id;

    return lock_id;
}

// Returns true if the file was unlocked
bool FileTable::unlockFile(char* filename, int lock_id) {
    string fname = filename;

    if (files.at(fname).lock_id != lock_id) return false;   // Invalid lock_id
    files.at(fname).lock_id = -1;
    if (files.at(fname).open == 0)
        files.erase(fname);

    // File successfully unlocked
    return true;
}

int FileTable::getInode(int filedesc){
    return fileHandles.at(filedesc).inode;
}

char FileTable::getMode(int filedesc){
    return fileHandles.at(filedesc).mode;
}

// increment read/write pointer by one for given file 
// handle pointed to by filedesc
void FileTable::incRW(int filedesc){
    fileHandles.at(filedesc).rw++;
}

// Set read/write pointer for file handle pointed to by filedesc
void FileTable::setRW(int filedesc, int val){
    fileHandles.at(filedesc).rw = val;
}

// Return true if the file description exists
// Return false otherwise
bool FileTable::isValidFileDesc(int filedesc){
    if(fileHandles.find(filedesc) == fileHandles.end()) return false;

    return true;
}

int FileTable::getRW(int filedesc){
    return fileHandles.at(filedesc).rw;
}
