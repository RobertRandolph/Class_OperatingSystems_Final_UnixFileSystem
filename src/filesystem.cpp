#include "disk.h"
#include "diskmanager.h"
#include "bitvector.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include "filetable.h"
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
using namespace std;

FileSystem::FileSystem(DiskManager *dm, char fileSystemName)
{
    // Init
    myDM = dm;
    myfileSystemName = fileSystemName;
    myfileSystemSize = dm->getPartitionSize(myfileSystemName);
    myPM = new PartitionManager(myDM, myfileSystemName, myfileSystemSize);
}

// Returns 0 if the file was created successfully
// Returns -1 if the file already exists
// Returns -2 if there isn't enough disk space
// Returns -3 If the file has an invalid name
// Returns -4 For anything else
int FileSystem::createFile(char *filename, int fnameLen)
{
    char buffer[64];
    char tmp[] = "Jim's Great!";
    int blk, dblk, r;

    // Validating
    if (fnameLen <= 0) return -4;                   // No name
    if (!validName(filename, fnameLen)) return -3;  // Invalid name
    r = exists(filename, fnameLen, dblk);
    if (r == 0) return -1;                          // File exists
    if (r == -3) return -4;                         // Directory exists
    if (r == -1) return -4;                         // File path doesn't exist
    blk = myPM->getFreeDiskBlock();
    if (blk == -1) return -2;                       // Not enough disk space

    // Init buffer and creating file inode
    for (int i = 0; i < 64; i++) buffer[i] = ' ';
    buffer[0] = filename[fnameLen - 1]; // File name
    buffer[1] = 'F';                    // File type
    placeInt(buffer, 2, 0);             // File size
    
    // Setting default attributes
    // Color: 22-33 RGB values, (JIM)
    placeInt(buffer, 22, 74);
    placeInt(buffer, 26, 73);
    placeInt(buffer, 30, 77);
    // Rating: 34-37 value, 9001 (It's over 9000!)
    placeInt(buffer, 34, 9001);
    // Tag: 38-63 (the rest), (Jim's great!)
    for (int i = 0; i < 12; i++)
        buffer[i+38] = tmp[i];

    // Attemping disk write
    r = myPM->writeDiskBlock(blk, buffer);
    if (r != 0) {
        // Failed disk write
        myPM->returnDiskBlock(blk);
        return -4;
    }

    // Attemping to place file inside directory
    if (!placeFile(filename[fnameLen - 1], blk, dblk)) {
        // Failed to place file (not enough disk space)
        myPM->returnDiskBlock(blk);
        return -2;
    }

    // File was successfully created
    return 0;
}

// Returns 0 if the directory was created successfully
// Returns -1 if the directory already exists
// Returns -2 if there isn't enough disk space
// Returns -3 If the directory has an invalid name
// Returns -4 for anything else
int FileSystem::createDirectory(char *dirname, int dnameLen)
{
    char buffer[64];
    int blk, dblk, r;

    // Validating
    if (dnameLen <= 0) return -4;                   // No name
    if (!validName(dirname, dnameLen)) return -3;   // Invalid name
    r = exists(dirname, dnameLen, dblk, false);
    if (r == 0) return -1;                          // Directory exists
    if (r == -3) return -4;                         // File exists
    if (r == -1) return -4;                         // Directory path doesn't exist
    blk = myPM->getFreeDiskBlock();
    if (blk == -1) return -2;                       // Not enough disk space

    // Init buffer and creating directory inode
    for (int i = 0; i < 64; i++) buffer[i] = ' ';

    // Attempting disk write
    r = myPM->writeDiskBlock(blk, buffer);
    if (r != 0) {
        // Failed disk write
        myPM->returnDiskBlock(blk);
        return -4;
    }

    // Attempting to place directory inside directory
    if (!placeFile(dirname[dnameLen - 1], blk, dblk, false)) {
        // Failed to place directory (not enough disk space)
        myPM->returnDiskBlock(blk);
        return -2;
    }

    // Directory was successfully created
    return 0;
}

// Returns a number > 0 if the file was successfully locked
// The number returned is the key to unlock the file
// Returns -1 if the file is already locked
// Returns -2 if the file doesn't exists
// Returns -3 if the file is open
// Returns -4 for anything else
int FileSystem::lockFile(char *filename, int fnameLen)
{
    int dblk, r;

    // Validating
    if (fnameLen <= 0) return -4;                   // No name
    if (!validName(filename, fnameLen)) return -4;  // Invalid name
    r = exists(filename, fnameLen, dblk);
    if (r == -2) return -2;                         // File doesn't exist
    if (r == -1) return -4;                         // File path doesn't exist
    if (r == -3) return -4;                         // It's a directory
    if (fileTable.isLocked(filename)) return -1;    // File already locked
    if (fileTable.isOpen(filename)) return -3;      // File is open

    // Locking file
    r = fileTable.lockFile(filename);

    // Returning lock_id
    return r;
}

// Returns 0 if the file was successfully unlocked
// Returns -1 if the lock_id is invalid
// Returns -2 For anything else
int FileSystem::unlockFile(char *filename, int fnameLen, int lockId)
{
    int dblk, r;

    // Validating
    if (fnameLen <= 0) return -2;                   // No name
    if (!validName(filename, fnameLen)) return -2;  // Invalid name
    r = exists(filename, fnameLen, dblk);
    if (r == -2) return -2;                         // File doesn't exist
    if (r == -1) return -2;                         // File path doesn't exist
    if (r == -3) return -2;                         // It's a directory
    if (!fileTable.isLocked(filename)) return -2;   // File isn't locked
    if (!fileTable.unlockFile(filename, lockId))
        return -1;                                  // Invalid lock_id

    // File was successfully unlocked
    return 0;
}

// Returns 0 if the file was successfully deleted
// Returns -1 if the file doesn't exist
// Returns -2 if the file is locked or in use
// Returns -3 for anything else
int FileSystem::deleteFile(char *filename, int fnameLen)
{
    int dblk, r;

    // Validating
    if (fnameLen <= 0) return -3;                   // No name
    if (!validName(filename, fnameLen)) return -3;  // Invalid name
    r = exists(filename, fnameLen, dblk);
    if (r == -2) return -1;                         // File doesn't exist
    if (r == -1) return -3;                         // File path doesn't exist
    if (r == -3) return -3;                         // It's a directory
    if (fileTable.isLocked(filename)) return -2;    // File is locked
    if (fileTable.isOpen(filename)) return -2;      // File is in use

    // Cleaning file
    cleanFile(filename[fnameLen - 1], dblk);

    // File was successfully deleted
    return 0;
}

// Returns 0 if the directory was successfully deleted
// Returns -1 if the directory doesn't exist
// Returns -2 is not empty
// Returns -3 for anything else
int FileSystem::deleteDirectory(char *dirname, int dnameLen)
{
    int dblk, r;

    // Validating
    if (dnameLen <= 0) return -3;                   // No name
    if (!validName(dirname, dnameLen)) return -3;   // Invalid name
    r = exists(dirname, dnameLen, dblk, false);
    if (r == -2) return -1;                         // Directory doesn't exist
    if (r == -1) return -3;                         // Directory path doesn't exist
    if (r == -3) return -1;                         // It's a file

    // Cleaning directory
    if (!cleanDirectory(dirname[dnameLen - 1], dblk))
        return -2;                                  // Directory isn't empty

    // Directory wsa successfully deleted
    return 0;
}

// Returns a number > 0 if the file was successfully opened
// The number returned is the filedesc to edit it's current instance
// Returns -1 if file doesn't exist
// Returns -2 if mode is invalid
// Returns -3 if the file cannot be opened due to lock
// Returns -4 for anything else
int FileSystem::openFile(char *filename, int fnameLen, char mode, int lockId)
{
    int dblk, eblk, dpos, fblk, r;

    // Validating
    if (fnameLen <= 0) return -4;                   // No name
    if (!validName(filename, fnameLen)) return -1;  // Invalid name
    r = exists(filename, fnameLen, dblk);
    if (r == -2) return -1;                         // File doesn't exist
    if (r == -1) return -4;                         // File path doesn't exist
    if (r == -3) return -1;                         // It's a directory
    if (!(mode == 'r' || mode == 'w' || mode == 'm'))
        return -2;                                  // Invalid mode
    r = fileTable.isOpenable(filename, lockId);
    if (r == 0 && lockId != -1) return -3;          // Invalid lock_id
    if (r == -1) return -3;                         // Invalid lock_id

    // Opening instance of file
    findFile(filename[fnameLen - 1], dblk, eblk, dpos, fblk);
    r = fileTable.openFile(filename, mode, fblk);

    // File was successfully opened
    return r;
}

// Returns 0 if the file was successfully closed
// Returns -1 if the file descriptor is invalid
// Returns -2 for anything else
int FileSystem::closeFile(int fileDesc)
{
    // Validating
    if (!fileTable.closeFile(fileDesc)) return -1;

    // File was successfully closed
    return 0;
}

// Returns -1 if file description invalid
// Returns -2 if length is negative
// Returns -3 if opperation is not permitted
// Returns number of bytes read if sucessfull
int FileSystem::readFile(int fileDesc, char *data, int len)
{
  char buffer[64];
  int rw, fileSize, curentblock;
  int index(0);
  int i(0);

  // if len is an invalid number
  if (len < 0) return -2;

  // if fileDesc is not the key to any file
  if (!fileTable.isValidFileDesc(fileDesc)) return -1;

  // if the file is not open for reading
  if(fileTable.getMode(fileDesc) != 'r' && fileTable.getMode(fileDesc) != 'm') return -3;

  int inodelocation = fileTable.getInode(fileDesc);
  char inode[64];
  myPM->readDiskBlock(inodelocation,inode);
  rw = fileTable.getRW(fileDesc);
  fileSize = getInt(inode,2);

  // if the file size is 0 return 0.
  if(fileSize == 0) return 0;

  curentblock = rw / 64;

  myPM->readDiskBlock(findBlockLocation(inode,curentblock),buffer);

  index = rw % 64; //where in the current block we should start reading
  while(index < 64 && i < len && rw < fileSize){
      data[i] = buffer[index];
      index++;
      i++;
      rw++;
  }
  curentblock++; //move to the next block
  index = 0; //reset index

 //continue reading until len is reached or we run out of file
  while(i < len && i < fileSize && rw < fileSize){
    myPM->readDiskBlock(findBlockLocation(inode,curentblock),buffer);
    //copy block into data array
    while(index < 64 && i < len && rw < fileSize){
      data[i] = buffer[index];
      index++;
      i++;
      rw++;
    }
    index = 0;
    curentblock++;
  }

  // i bytes have been read. set rw pointer to rw + i
  fileTable.setRW(fileDesc, rw);
  
  return i;
}

// Returns -1 if file description invalid
// Returns -2 if length is negative
// Returns -3 if opperation is not permitted
// Returns 0 if there was not enough space to complete write (space in file or space in partition)
// Returns len if all data is written.
// Note: This implementation will NOT perform a partial write.
//       I think this is a good policy.
int FileSystem::writeFile(int fileDesc, char *data, int len)
{
  // The biggest problem with this function is that we can only write blocks
  // of data. If rw is at byte 62 of a block, we have to rewrite all other 
  // bytes

  char buffer[64]; //use this to write out 64 bytes from 'data' at a time
  int inodeLocation; //location of the file's inode
  char inode[64], indirectInode[64]; //this will hold the inode's data
  int rw, fileSize, fileblocks, necessaryblocks(0), blocktoplace;;

  /* ---------------------- input validation ------------------ */
  if (len < 0) return -2;

  // if fileDesc is not the key to any file
  if (!fileTable.isValidFileDesc(fileDesc)) return -1;

  // if the file is not open for reading
  if(fileTable.getMode(fileDesc) != 'w' && fileTable.getMode(fileDesc) != 'm'){
   return -3;
  }
  /* --------------------- end input validation ------------------ */

  /* --------------------- general info we need ------------------ */
  inodeLocation = fileTable.getInode(fileDesc);
  myPM->readDiskBlock(inodeLocation,inode);
  rw = fileTable.getRW(fileDesc);
  fileSize = getInt(inode,2);
  fileblocks = (fileSize + 63) / 64; // '+63' basically makes this a ceiling function
  necessaryblocks = (rw + len + 63) / 64;
  /* --------------------- end gen. info ------------------------ */

  /* ------------------- memory allocation -------------------- */
  //if we need to request more memory
  if(necessaryblocks > fileblocks){

    //if our file implementation cannot hold this much data
    if (necessaryblocks > 19){
      return -3; //might as well return
    }

    //necessaryblocks is now the number of blocks we need to allocate
    necessaryblocks -= fileblocks;

    int newblocks[necessaryblocks];
    int indirectLocation(0);

    //if the indirect has not been allocated and we will need it
    if(fileblocks < 4 && necessaryblocks + fileblocks > 3){
      indirectLocation = myPM ->getFreeDiskBlock();
      for(int i = 0; i < 64; ++i) indirectInode[i] = ' ';
      placeInt(inode,18,indirectLocation);
    } else if (necessaryblocks + fileblocks > 3) { //if we need the indirect Inode and it is allocated
        myPM->readDiskBlock(getInt(inode, 18), indirectInode);
    }


    for(int i = 0; i < necessaryblocks; ++i){
      //grab a disk block
      newblocks[i] = myPM->getFreeDiskBlock();

      //if the partition is full
      if(newblocks[i] < 1){
          //loop through what has already been allocated
          for(int j = 0; j < i; ++j){
              //and return them.
              myPM->returnDiskBlock(newblocks[j]);
          }
          //return the indirect inode if necessary
          if(indirectLocation != 0){
              myPM->returnDiskBlock(indirectLocation);
          }
          return -3; //memory too full to complete the write. Might as well return now.
      }
    }

    for(int i = 0; i < necessaryblocks; ++i){
        blocktoplace = (i + fileblocks);
        if(blocktoplace < 3){
          placeInt(inode,(blocktoplace*4) + 6,newblocks[i]);
        } else {
          placeInt(indirectInode, (blocktoplace - 3) * 4,newblocks[i]);
        }
    }

    //write the inode and indirect if necessary
    if(rw + len > 192){
        myPM->writeDiskBlock(getInt(inode, 18), indirectInode);
        ;
    }
    myPM->writeDiskBlock(inodeLocation,inode);
  } else if(rw + len > 192){
      myPM->readDiskBlock(getInt(inode,18),indirectInode);
  }
  /* ----------------- end memory allocation ------------------------ */
  /* At this point:
     * We know there is enough space in the partition 
     * Enough blocks have been allocated
     * The inode (and indirect inode) have the correct information.*/
  /* ---------------------- edge case -------------------------- */
  // if we need to partially write a block
  int currentblock = rw / 64;
  //bindex: buffer index. This is reset to 0 before every block read
  //index: index from 'data'. This is never reset. when it reaches len we are done.
  int bindex = rw % 64, index = 0;
  myPM->readDiskBlock(findBlockLocation(inode,currentblock),buffer);
  
  while(bindex < 64 && index < len){
    buffer[bindex] = data[index];
    bindex++;
    index++;
  } 
  bindex = 0;
  myPM->writeDiskBlock(findBlockLocation(inode,currentblock), buffer);
  currentblock++;
  /* -------------------- end edge case ---------------------- */

  /* ------------------ the rest of the write -------------- */
  while(index < len){
    myPM->readDiskBlock(findBlockLocation(inode,currentblock),buffer);
    while(bindex < 64 && index < len){
      buffer[bindex] = data[index];
      index++;
      bindex++;
    }
    myPM->writeDiskBlock(findBlockLocation(inode,currentblock),buffer);

    currentblock++;
    bindex = 0;
  }

  fileTable.setRW(fileDesc,rw + index);
  //if the file size increased
  if((rw + index) - fileSize > 0){
      placeInt(inode, 2, rw + index);
      myPM->writeDiskBlock(inodeLocation, inode);
  }
   
  return index;
}

// Returns -1 if file description invalid
// Returns -2 if length is negative
// Returns -3 if opperation is not permitted
// Returns number of bytes appended if sucessfull
int FileSystem::appendFile(int fileDesc, char *data, int len)
{
  // append is the same as write if the rw pointer is at the end of the file
  char inode[64];
  /* ---------------------- input validation ------------------ */
  if (len < 0) return -2;

  // if fileDesc is not the key to any file
  if (!fileTable.isValidFileDesc(fileDesc)) return -1;

  // if the file is not open for reading
  if(fileTable.getMode(fileDesc) != 'w' && fileTable.getMode(fileDesc) != 'm') return -3;
  /* --------------------- end input validation ------------------ */
  int inodeLocation = fileTable.getInode(fileDesc);
  myPM->readDiskBlock(inodeLocation,inode);
  int fileSize = getInt(inode,2);
  fileTable.setRW(fileDesc, fileSize);
  /* --------------------- end gen. info ------------------------ */

  //now that rw = fileSize, we will let writeFile handle it.
  return writeFile(fileDesc, data, len);
}

// Returns 0 if successful
// Returns -1 if the file descriptor, offset, or flag is invalid
// Returns -2 if if outside of file bounds
int FileSystem::seekFile(int fileDesc, int offset, int flag)
{
    char buffer[64];
    int rw, size, inode;

    // Checking if invalid file descriptor
    if (!fileTable.isValidFileDesc(fileDesc)) return -1;

    // Calculating new rw
    if (!flag) rw = fileTable.getRW(fileDesc) + offset;
    else rw = offset;

    // Getting file size
    inode = fileTable.getInode(fileDesc);
    myPM->readDiskBlock(inode, buffer);
    size = getInt(buffer, 2);

    // Checking if invalid offset or out of bounds
    if (flag && (rw < 0 || rw > size)) return -1;   // Invalid offset
    if (!flag && (rw < 0 || rw > size)) return -2;  // Out of bounds

    // Setting rw
    fileTable.setRW(fileDesc, rw);
    
    // Was successful
    return 0;
}

// Returns 0 if the file was successfully renamed
// Returns -1 if invalid filename
// Returns -2 if the file doesn't exist
// Returns -3 if the new filename already exists
// Returns -4 if the file is open or locked
// Returns -5 For anything else
int FileSystem::renameFile(char *filename1, int fnameLen1, char *filename2, int fnameLen2)
{
    char buffer[64];
    char type;
    int dblk, eblk, dpos, fblk, r;

    // Validating
    if (fnameLen1 <= 0 || filename2 <= 0) return -5;    // No name
    if (!validName(filename1, fnameLen1)) return -1;    // Invalid name
    if (!validName(filename2, fnameLen2)) return -1;    // Invalid name
    r = exists(filename1, fnameLen1, dblk);
    if (r == -2) return -2;                             // File doesn't exist
    if (r == -1) return -5;                             // File path doesn't exist
    if (fileTable.isLocked(filename1)) return -4;       // File is locked
    if (fileTable.isOpen(filename1)) return -4;         // File is open
    if (findFile(filename2[fnameLen2 - 1], dblk, eblk, dpos, fblk))
        return -3;                                      // New filename already exists

    // Finding file to rename
    findFile(filename1[fnameLen1 - 1], dblk, eblk, dpos, fblk);

    // Getting file type
    myPM->readDiskBlock(eblk, buffer);
    type = buffer[dpos*6 + 5];

    // Changing file entry in directory
    buffer[dpos*6] = filename2[fnameLen2 - 1];
    myPM->writeDiskBlock(eblk, buffer);

    // Checking if type is file
    if (type == 'F') {
        // Changing file inode name
        myPM->readDiskBlock(fblk, buffer);
        buffer[0] = filename2[fnameLen2 - 1];
        myPM->writeDiskBlock(fblk, buffer);
    }

    // File was successfully renamed
    return 0;
}

// Returns 0 if attributes were successfully gathered
// Returns -1 if invalid file or if it doesn't exist
// Returns -2 if the file is locked or open
// Returns -3 for anything else
int FileSystem::getAttribute(char *filename, int fnameLen,
        int& R, int& G, int& B, int& rating, string& tag)
{
    char buffer[64];
    int dblk, eblk, dpos, fblk, r;

    // Validating
    if (fnameLen <= 0) return -3;                   // No name
    if (!validName(filename, fnameLen)) return -1;  // Invalid filename
    r = exists(filename, fnameLen, dblk);
    if (r != 0) return -1;                          // Invalid file/Doesn't exist
    if (fileTable.isLocked(filename)) return -2;    // File is locked
    if (fileTable.isOpen(filename)) return -2;      // File is open
    
    // Getting file inode
    findFile(filename[fnameLen - 1], dblk, eblk, dpos, fblk);
    myPM->readDiskBlock(fblk, buffer);

    // Getting attributes
    R = getInt(buffer, 22);
    G = getInt(buffer, 26);
    B = getInt(buffer, 30);
    rating = getInt(buffer, 34);
    tag = "";
    for (r = 0; r < 12; r++)
        tag += buffer[r+38];
    
    // Done getting attributes
    return 0;
}

// Returns 0 if attributes were successfully placed
// Returns -1 if invalid file or if it doesn't exist
// Returns -2 if the file is locked or open
// Returns -3 if the RGB value is invalid
// RGB must be 3 values from 0-255
// Returns -4 if the rating is invalid
// Rating must be a value from 0-9999
// Returns -5 if the tag is invalid
// tag must be 0-# characters long
// Returns -6 for any other reason
int FileSystem::setAttribute(char *filename, int fnameLen,
        int R, int G, int B, int rating, string tag)
{
    char buffer[64];
    int dblk, eblk, dpos, fblk, r;

    // Validating File
    if (fnameLen <= 0) return -6;                   // No name
    if (!validName(filename, fnameLen)) return -1;  // Invalid filename
    r = exists(filename, fnameLen, dblk);
    if (r != 0) return -1;                          // invalid file/doesn't exist
    if (fileTable.isLocked(filename)) return -2;    // File is locked
    if (fileTable.isOpen(filename)) return -2;      // File is open

    // Validating attributes
    if (R < 0 || G < 0 || B < 0 || R > 255 || G > 255 || B > 255)
        return -3;                              // Invalid RGB values
    if (rating < 0 || rating > 9999) return -4; // Invalid rating
    r = tag.size();
    if (r > 12) return -5;                      // Invalid tag

    // Getting file inode
    findFile(filename[fnameLen - 1], dblk, eblk, dpos, fblk);
    myPM->readDiskBlock(fblk, buffer);

    // Setting attributes
    placeInt(buffer, 22, R);
    placeInt(buffer, 26, G);
    placeInt(buffer, 30, B);
    placeInt(buffer, 34, rating);
    for (r; r < 12; r++)    // Padding tag
        tag += ' ';
    for (r = 0; r < 12; r++)
        buffer[r+38] = tag[r];

    // Writing back new attributes
    myPM->writeDiskBlock(fblk, buffer);

    // Attributes successfully set
    return 0;
}

// Returns true if the file name is valid
bool FileSystem::validName(char* name, int nameLen) {
    char c;

    for (int i = 0; i < nameLen; i++) {
        c = name[i];
        if(i%2) {
            // Looking for A-Z, a-z
            if (!((c >= 65 && c <= 90) || (c >= 97 && c <= 122)))
                return false;
        }
        else {
            // Looking for /
            if (c != '/')
                return false;
        }
    }

    return true;
}

// Returns 0 if the file exists
// Returns -1 if the path doesn't exist
// Returns -2 if the file doesn't exist
// Returns -3 if the file is wrong type
// @dblk: Returns the directory block in which the file was found
// @flag: Check for either a file or directory
// true => file ; false => directory
// Default: true
int FileSystem::exists(char* path, int pathLen, int& dblk, bool flag) {
    char buffer[64];
    char c;
    int levels = pathLen/2 - 1; // The number of levels before filename
    int eblk = 1;   // Directory extension/part being looked at
    int dpos, fblk;
    bool exists;    // Check to see if a name exists
    dblk = 1;       // Starting at root directory

    // Looking through the path levels to ensure they exist
    for (int i = 0; i < levels; i++) {
        // Getting directory
        myPM->readDiskBlock(eblk, buffer);

        // Looking through directory
        exists = false;
        for (int j = 0; j < 10; j++) {
            c = buffer[j*6];            // Getting filename
            if (c == path[i*2 + 1]) {   // Checking if level was found
                c = buffer[j*6 + 5];    // Getting file type
                if (c = 'D') {          // Checking if file is a directory
                    exists = true;      // Found a valid level
                    dblk = getInt(buffer, j*6 + 1); // Getting next directory
                    eblk = dblk;
                    break;
                }
                else return -1; // Type was not a directory, path not found
            }
        }

        // Checking if level exists
        // If not checks for directory extension
        if (!exists) {
            c = buffer[60];
            if (c == ' ') return -1;    // End of directory, path not found
            eblk = getInt(buffer, 60);  // Getting directory extension
            i--;                        // Not done with current level
        }
    }

    // Checking if file exists
    if (!findFile(path[pathLen - 1], dblk, eblk, dpos, fblk)) return -2;

    // Gettin file type
    myPM->readDiskBlock(eblk, buffer);
    c = buffer[dpos*6 + 5];

    // Checking file type
    if ((flag && c == 'D') || (!flag && c == 'F')) return -3;

    // File exists
    return 0;
}

// Return true if the file was found
// @dblk: The directory in which to look for the file
// @eblk: Returns the directory extension in which the file was found
// @dpos: Returns the directory position where the file was found
// @fblk: Returns the file block of the file
bool FileSystem::findFile(char name, int dblk, int& eblk, int& dpos, int& fblk) {
    char buffer[64];
    char c;
    bool found = false;
    eblk = dblk;

    while (!found) {
        // Getting directory
        myPM->readDiskBlock(eblk, buffer);

        // Looking through directory for file
        for (dpos = 0; dpos < 10; dpos++) {
            c = buffer[dpos*6];     // Getting filename
            if (c == name) {        // Checking if file was found
                found = true;       // Found file
                fblk = getInt(buffer, dpos*6 + 1); // Setting "file block"
                break;
            }
        }

        // Checking if file was found
        // If not checks for directory extension
        if (!found) {
            c = buffer[60];
            if (c == ' ') return false; // End of directory, file not found
            eblk = getInt(buffer, 60);  // Getting directory extension
        }
    }

    return true;
}

// Returns true if the file was succesfully placed inside a directory
// @dblk: The directory in which to place the file
// @blk: The inode of the file
// @flag: Check to place either a file or directory
// true => file ; false => directory
// Default: true
bool FileSystem::placeFile(char name, int blk, int dblk, bool flag) {
    char buffer[64];
    char c;
    int eblk, dpos, i;
    bool free = false;  // Check to see if a free spot was found

    // Looking for free spot in directory
    if(!findFile(' ', dblk, eblk, dpos, i)) {
        // Free spot wasn't found
        // Attempting to extend directory
        i = myPM->getFreeDiskBlock();
        if (i == -1) return false;    // Failed to extend directory

        // Updating current directory's extension
        myPM->readDiskBlock(eblk, buffer);
        placeInt(buffer, 60, i);
        myPM->writeDiskBlock(eblk, buffer);

        // Getting new directory extension
        eblk = i;
        dpos = 0;
        myPM->readDiskBlock(eblk, buffer);

        // Init buffer (directory extension)
        for (i = 0; i < 64; i++) buffer[i] = ' ';
    }
    else myPM->readDiskBlock(eblk, buffer);

    // Adding file to directory @flag
    buffer[dpos*6] = name;              // File name
    placeInt(buffer, dpos*6 + 1, blk);  // File location
    if (flag) buffer[dpos*6 + 5] = 'F'; // File type
    else buffer[dpos*6 + 5] = 'D';      // Directory type
    myPM->writeDiskBlock(eblk, buffer);

    // File successfully placed
    return true;
}

// Returns true if the file was successfully cleaned
// @dblk: The directory where the file is
bool FileSystem::cleanFile(char name, int dblk) {
    char dbuffer[64];   // Directory where inode is
    char fbuffer[64];   // The inode data
    char ibuffer[64];   // The inode index data
    char c;
    int eblk, fblk, iblk, dpos, size;
    bool found = false;

    // Getting file
    findFile(name, dblk, eblk, dpos, fblk);
    myPM->readDiskBlock(eblk, dbuffer);

    // Getting file inode information
    myPM->readDiskBlock(fblk, fbuffer);
    size = getInt(fbuffer, 2);
    if (size%64 != 0) size = size/64 + 1;
    else size = size/64;
    
    // Cleaning file inode data blocks
    switch (size) {
    default:
        // Cleaning index data blk
        iblk = getInt(fbuffer, 18);
        myPM->readDiskBlock(iblk, ibuffer);
        for (int i = 0; i < 16; i++) {
            c = ibuffer[i*4];
            if (c == ' ') break;    // Done
            myPM->returnDiskBlock(getInt(ibuffer, i*4));
        }
        myPM->returnDiskBlock(iblk);
    case 3:
        // Cleaning direct data blk 3
        myPM->returnDiskBlock(getInt(fbuffer, 14));
    case 2:
        // Cleaning direct data blk 2
        myPM->returnDiskBlock(getInt(fbuffer, 10));
    case 1:
        // Cleaning direct data blk 1
        myPM->returnDiskBlock(getInt(fbuffer, 6));
    case 0:
        // Cleaning file inode
        myPM->returnDiskBlock(fblk);
    }

    // Removing file inode from directory
    for (int i = 0; i < 6; i++) dbuffer[dpos*6 + i] = ' ';
    myPM->writeDiskBlock(eblk, dbuffer);

    // Cleaning any extensions not being used
    cleanExtensions(dblk);

    // file successfully cleaned
    return true;
}

// Returns true if the directory was successfully cleaned
// @dblk: The directory where the D-file is
bool FileSystem::cleanDirectory(char name, int dblk) {
    char dbuffer[64];   // Directory data
    char fbuffer[64];   // D-file data
    char c;
    int eblk, fblk, dpos;
    bool empty = false; // Check to see if D-file is empty

    // Getting directory
    findFile(name, dblk, eblk, dpos, fblk);
    myPM->readDiskBlock(eblk, dbuffer);

    // Getting D-file
    myPM->readDiskBlock(fblk, fbuffer);

    // Looking through D-file
    // No need to look through extensions if it exists,
    // if an extension exists a file exists in the directory
    // Unecessary extensions are already cleaned up on a delete
    for (int i = 0; i < 11; i++) {
        c = fbuffer[i*6];
        if (c != ' ') return false; // File found, D-file not empty
    }

    // Cleaning D-file
    myPM->returnDiskBlock(fblk);

    // Removing D-file from directory
    for (int i = 0; i < 6; i++) dbuffer[dpos*6 + i] = ' ';
    myPM->writeDiskBlock(eblk, dbuffer);

    // Cleaning any extensions not being used
    cleanExtensions(dblk);

    // Directory was successfully cleaned
    return true;
}

// Cleans up any unnecessary directory extensions
// @dblk: The directory to clean extensions from
void FileSystem::cleanExtensions(int dblk) {
    char buffer[64];
    char c;
    int blks[6];    // The extension blks
    int size = 1;   // The number of blks assosiated with the directory (up to 6)
    int i, j;
    bool free[6];   // The blks that don't contain files
    for (int i = 0; i < 6; i++) free[i] = false;
    blks[0] = dblk; // Starting with first blk (the directory start)

    // Looking through directory extensions
    for (i = 0; i < size; i++) {
        // Getting directory extension
        myPM->readDiskBlock(blks[i], buffer);

        // Looking through extension
        for (j = 0; j < 10; j++) {
            c = buffer[j*6];        // Getting filename
            if (c != ' ') break;    // Checking if there is a file
        }
        if (j == 10) free[i] = true;// No files in current extension

        // Checking if directory has extension
        c = buffer[60];
        if (c != ' ') {
            size++; // Found another extension to the directory
            blks[i + 1] = getInt(buffer, 60);
        }
    }

    // Cleaning unnecessary extensions
    // Removes extends at the end of the directory that aren't being used.
    // Extensions that don't have files but lead to an extension that does
    // will not be cleaned.
    for (i = size - 1; i > 0; i--) {    // Starting at the end
        if (free[i]) {  // Checking if the extension is free
            myPM->returnDiskBlock(blks[i]); // Freeing extension
            if (!free[i - 1] || i == 1) {   // updating previous extension
                // Won't update previous extension if it is also free,
                // unless it is the last part of the directory.
                myPM->readDiskBlock(blks[i - 1], buffer);
                for (j = 0; j < 4; j++) buffer[60 + j] = ' ';
                myPM->writeDiskBlock(blks[i - 1], buffer);
            }
        }
        else {
            // Remaining extensions are needed
            break;
        }
    }
}

// Returns an integer from the buffer
int FileSystem::getInt(const char* buffer, int index) {
    string temp;
    for (int i = 0; i < 4; i++) temp += buffer[index+i];

    return atoi(temp.c_str());
}

// Places an integer into the buffer
void FileSystem::placeInt(char* buffer, int index, int num) {
    char temp[4];
    sprintf(temp, "%d", num);
    string temp2 = temp;
    for (int i = temp2.size(); i < 4; i++) temp2.insert(0,"0");
    for (int i = 0; i < 4; i++) buffer[index+i] = temp2[i];
}

// this function will use the given inode to locate where the requested block of
// memory is stored. If blocknumber > 2 then the function will look in the indirect
// to find the block location.
// param inode: the file inode to use
// param blocknumber: 0-18 representing which block of data you would like read
// return: location of the requested block of the file
int FileSystem::findBlockLocation(char* inode, int blocknumber){
    // this block's location is in the inode. We just need to grab it.
    if(blocknumber < 3){
        return getInt(inode, (blocknumber*4) + 6);
    } else { // this block's location is in the indirect inode
        char buf[64];
        myPM->readDiskBlock(getInt(inode, 18),buf); //read the indirect inode
        // -3 because the first 3 blocks are in the direct inode.
        // *4 because an int is 4 bytes.
        return getInt(buf, (blocknumber - 3)*4); 
    }
}




