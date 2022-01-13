#include "filetable.h"

class FileSystem {
  DiskManager *myDM;
  PartitionManager *myPM;
  char myfileSystemName;
  int myfileSystemSize;
  FileTable fileTable;
  
  bool validName(char* name, int nameLen);
  int exists(char* path, int pathLen, int& dblk, bool flag = true);
  bool findFile(char name, int dblk, int& eblk, int& dpos, int& fblk);
  bool placeFile(char name, int blk, int dblk, bool flag = true);

  bool cleanFile(char name, int dblk);
  bool cleanDirectory(char name, int dblk);
  void cleanExtensions(int dblk);

  int getInt(const char* buffer, int index);
  void placeInt(char* buffer, int index, int num);

  public:
    FileSystem(DiskManager *dm, char fileSystemName);
    int createFile(char *filename, int fnameLen);
    int createDirectory(char *dirname, int dnameLen);
    int lockFile(char *filename, int fnameLen);
    int unlockFile(char *filename, int fnameLen, int lockId);
    int deleteFile(char *filename, int fnameLen);
    int deleteDirectory(char *dirname, int dnameLen);
    int openFile(char *filename, int fnameLen, char mode, int lockId);
    int closeFile(int fileDesc);
    int readFile(int fileDesc, char *data, int len);
    int writeFile(int fileDesc, char *data, int len);
    int appendFile(int fileDesc, char *data, int len);
    int seekFile(int fileDesc, int offset, int flag);
    int renameFile(char *filename1, int fnameLen1, char *filename2, int fnameLen2);
    int getAttribute(char *filename, int fnameLen, int& R, int& G, int& B, int& rating, string& tag);
    int setAttribute(char *filename, int fnameLen, int R, int G, int B, int rating, string tag);
    int findBlockLocation(char* inode, int blocknumber);
};
