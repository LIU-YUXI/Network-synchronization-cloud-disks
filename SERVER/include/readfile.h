#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include <queue>
#include <stdio.h>
#include <dirent.h>
using namespace std;
#define myOK 0
#define myERROR -1
// 读入一个文件
int read(string filename, string &content);
struct file
{
    bool is_file;
    string filename;
    string md5;
    string path;
};
// 将目录下所有文件存入files
int userfiles(int userid, string rootdir, queue<file> &files, int rootlength, bool is_root = false);
// 如果存在该文件则往后写，如果不存在则从头写
int writefile(string filename, string content);
int renewfile(string filename, string content);