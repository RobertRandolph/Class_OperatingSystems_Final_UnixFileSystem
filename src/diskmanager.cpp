#include "disk.h"
#include "bitvector.h"
#include "diskmanager.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

// Constructor
DiskManager::DiskManager(Disk *d, int partcount, DiskPartition *dp)
{
    myDisk = d;
    partCount = partcount;
    char buffer[64];
    isNew = myDisk->initDisk();
    int offset = 1;
    int i;
    
    // Checking if DISK exists or not
    if (isNew) {
        // DISK didn't exist and was created
        // Init SuperBlock and partitions array
        diskP = new DiskPartition[partCount];
        for (i = 0; i < 64; i++) buffer[i] = ' ';
        
        // Filling SuperBlock and partitions array
        for (i = 0; i < partCount; i++) {
            // Partitions array
            diskP[i] = dp[i];
            diskP[i].partitionStart = offset;

            // SuperBlock
            buffer[i*9] = dp[i].partitionName;              // Partition name
            placeInt(buffer, i*9 + 1, offset);              // Partition start block
            placeInt(buffer, i*9 + 5, dp[i].partitionSize); // Partition size

            // Updating offset for partiton starting blocks
            offset += dp[i].partitionSize;
        }

        // Writing SuperBlock
        myDisk->writeDiskBlock(0, buffer);

        // Init partition blks
        for (i = 0; i < 64; i++) buffer[i] = ' ';
        for (i = 0; i < partCount; i++)
            writeDiskBlock(diskP[i].partitionName, 1, buffer);
        for (i = 0; i < partCount; i++) {
            BitVector bv(diskP[i].partitionSize);
            bv.setBit(0);   // Bitvector
            bv.setBit(1);   // Root directory
            bv.getBitVector((unsigned int*) buffer);
            writeDiskBlock(diskP[i].partitionName, 0, buffer);
        }
    }
    else {
        // DISK already exists
        partCount = 0;

        // Reading partitions from SuperBlock
        myDisk->readDiskBlock(0, buffer);
        while (buffer[partCount*9] != ' ')
            partCount++;

        // Init and filling partition array
        diskP = new DiskPartition[partCount];
        for (int i = 0; i < partCount; i++) {
            diskP[i].partitionName = buffer[i*9];               // Partition name
            diskP[i].partitionStart = getInt(buffer, i*9 + 1);  // Partition start block
            diskP[i].partitionSize = getInt(buffer, i*9 + 5);   // Partition size
        }
    }
}

// Destructor
DiskManager::~DiskManager() {
    delete [] diskP;
}

/*
 *   returns: 
 *   0, if the block is successfully read;
 *  -1, if disk can't be opened; (same as disk)
 *  -2, if blknum is out of bounds; (same as disk)
 *  -3 if partition doesn't exist
 */
int DiskManager::readDiskBlock(char partitionname, int blknum, char *blkdata)
{
    // Checking if partition exists
    bool exist = false;
    DiskPartition dp;
    for (int i = 0; i < partCount; i++)
        if (partitionname == diskP[i].partitionName) {
            dp = diskP[i];
            exist = true;
            break;
        }
    if (!exist) return -3;

    // Checking if blknum is out of bounds of partition
    if (blknum < 0 || blknum >= dp.partitionSize) return -2;

    // Attempting to read disk.
    return myDisk->readDiskBlock(blknum + dp.partitionStart, blkdata);
}


/*
 *   returns: 
 *   0, if the block is successfully written;
 *  -1, if disk can't be opened; (same as disk)
 *  -2, if blknum is out of bounds;  (same as disk)
 *  -3 if partition doesn't exist
 */
int DiskManager::writeDiskBlock(char partitionname, int blknum, char *blkdata)
{
    // Checking if partition exists
    bool exist = false;
    DiskPartition dp;
    for (int i = 0; i < partCount; i++)
        if (partitionname == diskP[i].partitionName) {
            dp = diskP[i];
            exist = true;
            break;
        }
    if (!exist) return -3;

    // Checking if blknum is out of bounds of partition
    if (blknum < 0 || blknum >= dp.partitionSize) return -2;

    // Attempting to write to disk.
    return myDisk->writeDiskBlock(blknum + dp.partitionStart, blkdata);
}

/*
 * return size of partition
 * -1 if partition doesn't exist.
 */
int DiskManager::getPartitionSize(char partitionname)
{
    // Checking if partition exists
    for (int i = 0; i < partCount; i++)
        if (partitionname == diskP[i].partitionName)
            return diskP[i].partitionSize;

    return -1;
}

// Returns an integer from the buffer
int DiskManager::getInt(const char* buffer, int index) {
    string temp;
    for (int i = 0; i < 4; i++) temp += buffer[index+i];

    return atoi(temp.c_str());
}

// Places an integer into the buffer
void DiskManager::placeInt(char* buffer, int index, int num) {
    char temp[4];
    sprintf(temp, "%d", num);
    string temp2 = temp;
    for (int i = temp2.size(); i < 4; i++) temp2.insert(0,"0");
    for (int i = 0; i < 4; i++) buffer[index+i] = temp2[i];
}

int DiskManager::getIsNew(){
    return isNew;
}
