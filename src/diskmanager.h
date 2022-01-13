using namespace std;

class DiskPartition {
  public:
    char partitionName;
    int partitionStart;
    int partitionSize;
};

class DiskManager {
  Disk *myDisk;
  int partCount, isNew;
  DiskPartition *diskP;

  int getInt(const char* buffer, int index);
  void placeInt(char* buffer, int index, int num);

  public:
    DiskManager(Disk *d, int partCount, DiskPartition *dp);
    ~DiskManager();
    int readDiskBlock(char partitionname, int blknum, char *blkdata);
    int writeDiskBlock(char partitionname, int blknum, char *blkdata);
    int getBlockSize() {return myDisk->getBlockSize();};
    int getPartitionSize(char partitionname);
    int getIsNew();
};

