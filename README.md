# Class: Operating Systems Project

## Project Goal

Design and implement a subset of the unix file system.

### Files

The files were provided for this project

  - BitVector: Data structure
  - Disk: Simulates the behavior of a disk using a unix file
  - DiskManager: Partitions and manages access to a disk
  - PartitionManager: Manages partitions in a disk
  - FileSystem: Implements various filesystem operations
  - FileTable: Data structure to help manage opening and closing files (Created by us)
  - Client: Uses a file system
  - Driver0 to Driver6: Test file system

### Contribution

The files/functions that were implemented were

  - DiskManager.cpp: All of it (Me)
  - PartitionManager: All but last 3 functions (last 3 already there) (Partner)
  - filesystem.cpp & .h: All of it
    - Me: constructor, createFile, createDirectory, lockFile, unlockFile, deleteFile, deleteDirectory, openFile, closeFile, seekFile, renameFile, getAttributes, setAttribute, validName, exists, findFile, placeFile, cleanFile, cleanDirectory, cleanExtensions, getInt, placeInt
    - Partner: readFile, writeFile, appendFile, seekFile, findBlockLocation
  - fileTable.cpp & .h: All of it
    - Me: FileHandle, File, FileTable, openFile, closeFile, isLocked, isOpen, isOpenable, lockFile, unlockFile
    - Partner: FileHandle, getInode, getMode, incRW, setRW, isValidFileDesc, getRW


## Notes

This was a final project with one partner, name was removed for confidentiallity.

Parts were divided between us and are noted where each person contributed. Some places we worked on the same thing (they are also noted).  
We also had lots of discussion and bug fixes for things we couldn't get working alone.

Many of the files were removed as they weren't necessary to showcase code.
