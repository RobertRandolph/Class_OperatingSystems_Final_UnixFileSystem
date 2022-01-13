
/* Driver 6*/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include "disk.h"
#include "diskmanager.h"
#include "bitvector.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include "filetable.h"
#include "client.h"

using namespace std;

/*
  This driver will test the getAttributes() and setAttributes()
  functions. You need to complete this driver according to the
  attributes you have implemented in your file system, before
  testing your program.
  
  
  Required tests:
  get and set on the fs1 on a file
    and on a file that doesn't exist
    and on a file in a directory in fs1
    and on a file that doesn't exist in a directory in fs1

 fs2, fs3
  on a file both get and set on both fs2 and fs3

  samples are provided below.  Use them and/or make up your own.


*/

void printA(int R, int G, int B, int rating, string tag);

int main()
{

  Disk *d = new Disk(300, 64, const_cast<char *>("DISK1"));
  DiskPartition *dp = new DiskPartition[3];

  dp[0].partitionName = 'A';
  dp[0].partitionSize = 100;
  dp[1].partitionName = 'B';
  dp[1].partitionSize = 75;
  dp[2].partitionName = 'C';
  dp[2].partitionSize = 105;

  DiskManager *dm = new DiskManager(d, 3, dp);
  FileSystem *fs1 = new FileSystem(dm, 'A');
  FileSystem *fs2 = new FileSystem(dm, 'B');
  FileSystem *fs3 = new FileSystem(dm, 'C');
  Client *c1 = new Client(fs1);
  Client *c2 = new Client(fs2);
  Client *c3 = new Client(fs3);
  Client *c4 = new Client(fs1);
  Client *c5 = new Client(fs2);

 /*
  * IMPORTANT NOTE
  * These tests are ment to be run alone without
  * drivers 1-6 having already been run.
  */

  int r, R, G, B, rating;
  string tag;

  // Init
  c1->myFS->createFile( const_cast<char *>("/a"), 2);
  c1->myFS->createFile( const_cast<char *>("/b"), 2);
  c1->myFS->createFile( const_cast<char *>("/c"), 2);
  c1->myFS->createDirectory(const_cast<char *>("/d"), 2);
  c1->myFS->createFile( const_cast<char *>("/d/a"), 4);
  c1->myFS->createFile( const_cast<char *>("/d/b"), 4);
  c1->myFS->createFile( const_cast<char *>("/d/c"), 4);

  c2->myFS->createFile( const_cast<char *>("/a"), 2);
  c2->myFS->createFile( const_cast<char *>("/b"), 2);
  c2->myFS->createFile( const_cast<char *>("/c"), 2);
  c2->myFS->createDirectory(const_cast<char *>("/d"), 2);
  c2->myFS->createFile( const_cast<char *>("/d/a"), 4);
  c2->myFS->createFile( const_cast<char *>("/d/b"), 4);
  c2->myFS->createFile( const_cast<char *>("/d/c"), 4);

  c3->myFS->createFile( const_cast<char *>("/a"), 2);
  c3->myFS->createFile( const_cast<char *>("/b"), 2);
  c3->myFS->createFile( const_cast<char *>("/c"), 2);
  c3->myFS->createDirectory(const_cast<char *>("/d"), 2);
  c3->myFS->createFile( const_cast<char *>("/d/a"), 4);
  c3->myFS->createFile( const_cast<char *>("/d/b"), 4);
  c3->myFS->createFile( const_cast<char *>("/d/c"), 4);

  // Tests
  // Basic set get tests
  // Valid
  cout << "Client 1: Basic set and get with single file" << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/a"), 2, 1,1,1, 15, "myTag");
  cout << "rv from setAttribute 1,1,1 ; 15 ; myTag is " << r << (r==0?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/b"), 2, 5,8,12, 210, "epic");
  cout << "rv from setAttribute 5,8,12 ; 210 ; epic is " << r << (r==0?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/c"), 2, 185,28,112, 2110, "evenBetter");
  cout << "rv from setAttribute 185,28,112 ; 2110 ; evenBetter is " << r << (r==0?" Correct": " failure") << endl;

  r = c1->myFS->getAttribute(const_cast<char *>("/a"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c1->myFS->getAttribute(const_cast<char *>("/b"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c1->myFS->getAttribute(const_cast<char *>("/c"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);

  r = c1->myFS->setAttribute(const_cast<char *>("/c"), 2, 100,20,100, 1, "bloop");
  cout << "rv from setAttribute 100,20,100 ; 1 ; bloop is " << r << (r==0?" Correct": " failure") << endl;
  r = c1->myFS->getAttribute(const_cast<char *>("/c"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);

  // Edge Cases
  cout << "\nClient 1: Edge cases with single file" << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/a"), 2, 0,0,0, 0, "");
  cout << "rv from setAttribute 0,0,0 ; 0 ; '' is " << r << (r==0?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/b"), 2, 255,255,255, 9999, "abcdefghijkl");
  cout << "rv from setAttribute 255,255,255 ; 9999 ; abcdefghijkl is " << r << (r==0?" Correct": " failure") << endl;

  r = c1->myFS->getAttribute(const_cast<char *>("/a"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c1->myFS->getAttribute(const_cast<char *>("/b"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);

  // Basic set get tests with directory
  cout << "\nClient 1: Basic set and get with directory file" << endl;
  r = c1->myFS->getAttribute(const_cast<char *>("/d/a"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);

  r = c1->myFS->setAttribute(const_cast<char *>("/d/a"), 4, 1,1,1, 15, "myTag");
  cout << "rv from setAttribute 1,1,1 ; 15 ; myTag is " << r << (r==0?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/d/b"), 4, 5,8,12, 210, "epic");
  cout << "rv from setAttribute 5,8,12 ; 210 ; epic is " << r << (r==0?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/d/c"), 4, 185,28,112, 2110, "evenBetter");
  cout << "rv from setAttribute 185,28,112 ; 2110 ; evenBetter is " << r << (r==0?" Correct": " failure") << endl;

  r = c1->myFS->getAttribute(const_cast<char *>("/d/a"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c1->myFS->getAttribute(const_cast<char *>("/d/b"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c1->myFS->getAttribute(const_cast<char *>("/d/c"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);

  r = c1->myFS->setAttribute(const_cast<char *>("/d/c"), 4, 100,20,100, 1, "bloop");
  cout << "rv from setAttribute 100,20,100 ; 1 ; bloop is " << r << (r==0?" Correct": " failure") << endl;
  r = c1->myFS->getAttribute(const_cast<char *>("/d/c"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);

  // Invalid tests
  // With file
  cout << "\nClient 1: invalid file, locked, opened with file" << endl;
  c1->myFS->lockFile( const_cast<char *>("/a"), 2);
  c1->myFS->openFile(const_cast<char *>("/b"), 2, 'r', -1);

  r = c1->myFS->setAttribute(const_cast<char *>("/d"), 2, 0,0,0, 0, "");
  cout << "rv from setAttribute /d is " << r << (r==-1?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/e"), 2, 0,0,0, 0, "");
  cout << "rv from setAttribute /e is " << r << (r==-1?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("a"), 1, 0,0,0, 0, "");
  cout << "rv from setAttribute a is " << r << (r==-1?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/a"), 2, 0,0,0, 0, "");
  cout << "rv from setAttribute /a is " << r << (r==-2?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/b"), 2, 0,0,0, 0, "");
  cout << "rv from setAttribute /b is " << r << (r==-2?" Correct": " failure") << endl;

  r = c1->myFS->getAttribute(const_cast<char *>("/d"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute /d is " << r << (r==-1?" Correct": " failure") << endl;
  r = c1->myFS->getAttribute(const_cast<char *>("/e"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute /e is " << r << (r==-1?" Correct": " failure") << endl;
  r = c1->myFS->getAttribute(const_cast<char *>("a"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute d is " << r << (r==-1?" Correct": " failure") << endl;
  r = c1->myFS->getAttribute(const_cast<char *>("/a"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute /a is " << r << (r==-2?" Correct": " failure") << endl;
  r = c1->myFS->getAttribute(const_cast<char *>("/b"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute /b is " << r << (r==-2?" Correct": " failure") << endl;

  cout << "\nClient 1: invalid RGB, rating, tag" << endl;
  cout << "/a and /b is locked and opened respectively" << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/c"), 2, -1,0,0, 0, "");
  cout << "rv from setAttribute -1,0,0 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/c"), 2, 0,-1,0, 0, "");
  cout << "rv from setAttribute 0,-1,0 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/c"), 2, 0,0,-1, 0, "");
  cout << "rv from setAttribute 0,0,-1 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/c"), 2, 256,0,0, 0, "");
  cout << "rv from setAttribute 256,0,0 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/c"), 2, 0,256,0, 0, "");
  cout << "rv from setAttribute 0,256,0 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/c"), 2, 0,0,256, 0, "");
  cout << "rv from setAttribute 0,0,256 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;

  r = c1->myFS->setAttribute(const_cast<char *>("/c"), 2, 0,0,0, -1, "");
  cout << "rv from setAttribute 0,0,0 ; -1 ; '' is " << r << (r==-4?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/c"), 2, 0,0,0, 10000, "");
  cout << "rv from setAttribute 0,0,0 ; 10000 ; '' is " << r << (r==-4?" Correct": " failure") << endl;

  r = c1->myFS->setAttribute(const_cast<char *>("/c"), 2, 0,0,0, 0, "abcdefghijklm");
  cout << "rv from setAttribute 0,0,0 ; 0 ; abcdefghijklm is " << r << (r==-5?" Correct": " failure") << endl;

  // With directory
  cout << "\nClient 1: invalid file, locked, opened with directory" << endl;
  c1->myFS->lockFile( const_cast<char *>("/d/a"), 4);
  c1->myFS->openFile(const_cast<char *>("/d/b"), 4, 'r', -1);

  r = c1->myFS->setAttribute(const_cast<char *>("/d/d"), 4, 0,0,0, 0, "");
  cout << "rv from setAttribute /d/d is " << r << (r==-1?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/d/a"), 4, 0,0,0, 0, "");
  cout << "rv from setAttribute /d/a is " << r << (r==-2?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/d/b"), 4, 0,0,0, 0, "");
  cout << "rv from setAttribute /d/b is " << r << (r==-2?" Correct": " failure") << endl;

  r = c1->myFS->getAttribute(const_cast<char *>("/d/d"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute /d/d is " << r << (r==-1?" Correct": " failure") << endl;
  r = c1->myFS->getAttribute(const_cast<char *>("/d/a"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute /d/a is " << r << (r==-2?" Correct": " failure") << endl;
  r = c1->myFS->getAttribute(const_cast<char *>("/d/b"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute /d/b is " << r << (r==-2?" Correct": " failure") << endl;

  cout << "\nClient 1: invalid RGB, rating, tag" << endl;
  cout << "/d/a and /d/b is locked and opened respectively" << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/d/c"), 4, -1,0,0, 0, "");
  cout << "rv from setAttribute -1,0,0 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/d/c"), 4, 0,-1,0, 0, "");
  cout << "rv from setAttribute 0,-1,0 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/d/c"), 4, 0,0,-1, 0, "");
  cout << "rv from setAttribute 0,0,-1 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/d/c"), 4, 256,0,0, 0, "");
  cout << "rv from setAttribute 256,0,0 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/d/c"), 4, 0,256,0, 0, "");
  cout << "rv from setAttribute 0,256,0 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/d/c"), 4, 0,0,256, 0, "");
  cout << "rv from setAttribute 0,0,256 ; 0 ; '' is " << r << (r==-3?" Correct": " failure") << endl;

  r = c1->myFS->setAttribute(const_cast<char *>("/d/c"), 4, 0,0,0, -1, "");
  cout << "rv from setAttribute 0,0,0 ; -1 ; '' is " << r << (r==-4?" Correct": " failure") << endl;
  r = c1->myFS->setAttribute(const_cast<char *>("/d/c"), 4, 0,0,0, 10000, "");
  cout << "rv from setAttribute 0,0,0 ; 10000 ; '' is " << r << (r==-4?" Correct": " failure") << endl;

  r = c1->myFS->setAttribute(const_cast<char *>("/d/c"), 4, 0,0,0, 0, "abcdefghijklm");
  cout << "rv from setAttribute 0,0,0 ; 0 ; abcdefghijklm is " << r << (r==-5?" Correct": " failure") << endl;

  // Tests with other clients
  cout << "\nOther clients: with file" << endl;
  r = c2->myFS->getAttribute(const_cast<char *>("/a"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c3->myFS->getAttribute(const_cast<char *>("/b"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c4->myFS->getAttribute(const_cast<char *>("/c"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c5->myFS->getAttribute(const_cast<char *>("/a"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);

  r = c2->myFS->setAttribute(const_cast<char *>("/a"), 2, 1,1,1, 15, "myTag");
  cout << "rv from setAttribute 1,1,1 ; 15 ; myTag is " << r << (r==0?" Correct": " failure") << endl;
  r = c3->myFS->setAttribute(const_cast<char *>("/b"), 2, 5,8,12, 210, "epic");
  cout << "rv from setAttribute 5,8,12 ; 210 ; epic is " << r << (r==0?" Correct": " failure") << endl;
  r = c4->myFS->setAttribute(const_cast<char *>("/c"), 2, 185,28,112, 2110, "evenBetter");
  cout << "rv from setAttribute 185,28,112 ; 2110 ; evenBetter is " << r << (r==0?" Correct": " failure") << endl;
  r = c5->myFS->setAttribute(const_cast<char *>("/a"), 2, 2,21,12, 125, "myzzTag");
  cout << "rv from setAttribute 2,21,12 ; 125 ; myzzTag is " << r << (r==0?" Correct": " failure") << endl;

  r = c2->myFS->getAttribute(const_cast<char *>("/a"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c3->myFS->getAttribute(const_cast<char *>("/b"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c4->myFS->getAttribute(const_cast<char *>("/c"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c5->myFS->getAttribute(const_cast<char *>("/a"), 2, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);

  // with directory
  cout << "\nOther clients: with directory" << endl;
  r = c2->myFS->getAttribute(const_cast<char *>("/d/a"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c3->myFS->getAttribute(const_cast<char *>("/d/b"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c4->myFS->getAttribute(const_cast<char *>("/d/c"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c5->myFS->getAttribute(const_cast<char *>("/d/a"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);

  r = c2->myFS->setAttribute(const_cast<char *>("/d/a"), 4, 1,1,1, 15, "myTag");
  cout << "rv from setAttribute 1,1,1 ; 15 ; myTag is " << r << (r==0?" Correct": " failure") << endl;
  r = c3->myFS->setAttribute(const_cast<char *>("/d/b"), 4, 5,8,12, 210, "epic");
  cout << "rv from setAttribute 5,8,12 ; 210 ; epic is " << r << (r==0?" Correct": " failure") << endl;
  r = c4->myFS->setAttribute(const_cast<char *>("/d/c"), 4, 185,28,112, 2110, "evenBetter");
  cout << "rv from setAttribute 185,28,112 ; 2110 ; evenBetter is " << r << (r==0?" Correct": " failure") << endl;
  r = c5->myFS->setAttribute(const_cast<char *>("/d/a"), 4, 2,21,12, 125, "myzzTag");
  cout << "rv from setAttribute 2,21,12 ; 125 ; myzzTag is " << r << (r==0?" Correct": " failure") << endl;

  r = c2->myFS->getAttribute(const_cast<char *>("/d/a"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c3->myFS->getAttribute(const_cast<char *>("/d/b"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c4->myFS->getAttribute(const_cast<char *>("/d/c"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);
  r = c5->myFS->getAttribute(const_cast<char *>("/d/a"), 4, R,G,B, rating, tag);
  cout << "rv from getAttribute is " << r << (r==0?" Correct": " failure") << endl;
  if (r == 0) printA(R, G, B, rating, tag);

  return 0;
}

void printA(int R, int G, int B, int rating, string tag) {
    cout << "RGB: " << R << ", " << G << ", " << B << "; rating: " << rating << "; tag: " << tag << endl;
}

