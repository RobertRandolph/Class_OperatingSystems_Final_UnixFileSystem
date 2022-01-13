#include "disk.h"
#include "bitvector.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include <iostream>
using namespace std;


PartitionManager::PartitionManager(DiskManager *dm, char partitionname, int partitionsize)
{
  myDM = dm;
  myPartitionName = partitionname;
  myPartitionSize = myDM->getPartitionSize(myPartitionName);

  bv = new BitVector(myPartitionSize);
  /* initialize bit vector to keep track of free and allocted blocks in this partition */
  char buffer[64];
  myDM->readDiskBlock(myPartitionName,0,buffer);
  bv->setBitVector((unsigned int *) buffer);
}

PartitionManager::~PartitionManager()
{
  // delete the dynamically allocated bit vector
  delete bv;
  // myDM is not deleted because it could still exist in another PartitionManager
}

/*
 * return blocknum, -1 otherwise
 */
int PartitionManager::getFreeDiskBlock()
{
  // searth through the bit vector. No point checking 0 (the bitvector)
  // or 1 (Root)
  for(int i = 2; i < myPartitionSize; ++i){ 
    if(bv->testBit(i) == 0){ //find a free block
      bv->setBit(i); //mark this block as used

      char buffer[64]; 
      bv->getBitVector((unsigned int*) buffer); //put the bitvector into a buffer
      myDM->writeDiskBlock(myPartitionName,0,buffer); //write out the new BV
      return i;
    }
  }
  //we will only reach this if the entire bit vector is full
  return -1;
}

/*
 * return 0 for sucess, -1 otherwise
 */
int PartitionManager::returnDiskBlock(int blknum)
{
  char buffer[64];
  for(int i = 0; i < 64; ++i){
    buffer[i] = 'c';
  }
  int rtn = myDM->writeDiskBlock(myPartitionName, blknum, buffer);

  if(rtn == 0){ //if the data was overwritten
    bv->resetBit(blknum); //this spot is now free
    bv->getBitVector((unsigned int*) buffer);
    myDM->writeDiskBlock(myPartitionName,0,buffer);
    return 0;
  }
  return -1; //should only hit this if the data was not overwritten
}


int PartitionManager::readDiskBlock(int blknum, char *blkdata)
{
  return myDM->readDiskBlock(myPartitionName, blknum, blkdata);
}

int PartitionManager::writeDiskBlock(int blknum, char *blkdata)
{
  return myDM->writeDiskBlock(myPartitionName, blknum, blkdata);
}

int PartitionManager::getBlockSize() 
{
  return myDM->getBlockSize();
}
