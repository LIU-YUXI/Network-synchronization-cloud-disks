#define _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_SECURE_NO_WARNINGS
#include"fileactions.h"
#include<QDebug>
#include<QDateTime>
#include<QTime>
#include<QCoreApplication>
#define logfile "C:\\mycloud\\Liu\\run.log"
// 宽字节字符串转多字节字符串
using namespace std;

string FileTree;
string GbkToUtf8(const char *src_str)
{
    int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1]; memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    string strTemp = str;
    if (wstr) delete[] wstr;
    if (str) delete[] str;
    return strTemp;
}

void writeLog(string file,string operation,string status,string content){
    fstream out;
    out.open(file.c_str(),ios::out|ios::app);
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz ddd");
    out<<"["<<current_date.toStdString()<<"]"<<endl;
    out<<"operation:"<<operation<<endl;
    out<<"status:"<<status<<endl;
    out<<content<<endl<<endl;

    out.close();

}

void W2C(wchar_t* pwszSrc, int iSrcLen, char* pszDest, int iDestLen)
{
    ::RtlZeroMemory(pszDest, iDestLen);
    // 宽字节字符串转多字节字符串
    ::WideCharToMultiByte(CP_ACP,
        0,
        pwszSrc,
        (iSrcLen / 2),
        pszDest,
        iDestLen,
        NULL,
        NULL);
}
void checkFileschange(const char* dir) {
    HANDLE dir_handle = CreateFileA(dir, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if (dir_handle == INVALID_HANDLE_VALUE) {
        exit(0);
    }

    char szTemp[MAX_PATH] = { 0 };

    BOOL bRet = false;
    DWORD dwRet = 0;
    DWORD dwBufferSize = 2048;
    BYTE* pBuf = new BYTE[dwBufferSize];
    if (NULL == pBuf)
    {
        exit(0);
    }
    do
    {
        FILE_NOTIFY_INFORMATION* pFileNotifyInfo = (FILE_NOTIFY_INFORMATION*)pBuf;
        ::RtlZeroMemory(pFileNotifyInfo, dwBufferSize);
        // 设置监控目录
        bRet = ::ReadDirectoryChangesW(dir_handle,
            pFileNotifyInfo,
            dwBufferSize,
            TRUE,
            FILE_NOTIFY_CHANGE_SIZE |  // in file or subdir
            FILE_NOTIFY_CHANGE_ATTRIBUTES |
            FILE_NOTIFY_CHANGE_DIR_NAME | // creating, deleting a directory or sub
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_CREATION,
            //FILE_NOTIFY_CHANGE_LAST_WRITE,//如果不关闭这项会导致目录中的文件夹也显示被更改
            &dwRet,
            NULL,
            NULL);
        if (FALSE == bRet)
        {
            exit(0);
            break;
        }


        // 判断操作类型并显示
        while (1) {
            // 将宽字符转换成窄字符
            W2C((wchar_t*)(&pFileNotifyInfo->FileName), pFileNotifyInfo->FileNameLength, szTemp, MAX_PATH);
            switch (pFileNotifyInfo->Action)
            {
            case FILE_ACTION_ADDED:
            {
                // 新增文件
                printf("[File Added Action]%s\n", szTemp);
                break;
            }
            case FILE_ACTION_REMOVED:
            {
                // 新增文件
                printf("[File Removed Action]%s\n", szTemp);
                break;
            }
            case FILE_ACTION_MODIFIED:
            {
                // 新增文件
                printf("[File Modified Action]%s\n", szTemp);
                break;
            }
            case FILE_ACTION_RENAMED_NEW_NAME:
            {
                printf("[File New Name]%s\n", szTemp);
                break;
            }
            case FILE_ACTION_RENAMED_OLD_NAME:
            {
                printf("[File Old Name]%s\n", szTemp);
                break;
            }
            default:
            {
                break;
            }
            }
            if (pFileNotifyInfo->NextEntryOffset != 0)//多步操作，移动偏移量
            {
                pFileNotifyInfo = (FILE_NOTIFY_INFORMATION*)(((BYTE*)pFileNotifyInfo) + pFileNotifyInfo->NextEntryOffset);
            }
            else
            {
                break;
            }
        }
    } while (bRet);

    ::CloseHandle(dir_handle);
    delete[] pBuf;
    pBuf = NULL;
}
void openMonitorThread(string dir) {
    std::thread test1(checkFileschange, dir.c_str());
    test1.detach();
    writeLog(logfile,"Add monitor thread","success","Start to monitor "+dir);
    Sleep(10);
}
void startMonitor(string file) {
    fstream in;
    in.open(file.c_str(), ios::in);
    while (1) {
        char buff[1024] = { 0 };
        in.getline(buff, 1024);
        if (!in.good())
            break;
        string line = buff;
        cout << line << endl;
        if (line.find(">") != line.npos) {
            cout << line.substr(line.find(">") + 1) << endl;
            openMonitorThread(line.substr(line.find(">") + 1));
        }
    }

    in.close();
}
string findlinkFolder(string in,string path,bool &linked,int length) {
    linked = false;
    if ((int)path.length() < length)
        return "no";
    int pos1 = 0, pos2 = 0;

    path.erase(path.length()- 4);
    string folder = path.substr(length);
    qDebug()<<QString::fromStdString(folder)<<endl;
    while (1) {
        if (pos2 == (int)in.length()) {
            return "nothing";
        }
        if (in[pos2] == '\n') {
            string temp = in.substr(pos1, pos2 - pos1+1);
            qDebug()<<"temp"<<QString::fromStdString(temp)<<endl;
            if (temp.find(folder) != temp.npos) {
                if (temp.find(">") != temp.npos) {
                    linked = true;
                    return temp.substr(temp.find(">")+1);
                }
                else {
                    linked = false;
                    return "ababa";
                }
            }
            pos1 = pos2 + 1;
        }
        pos2++;
    }

}
bool search_folder(const char para[])
{
    WIN32_FIND_DATAA fdfile;
    HANDLE hFind = FindFirstFileA(para, &fdfile);//第一个参数是路径名，可以使用通配符，fd存储有文件的信息
    bool done = true;
    while (1)
    {
        done = FindNextFileA(hFind, &fdfile); //返回的值如果为0则没有文件要寻了
        if (!done)
            break;
        if (!strcmp(fdfile.cFileName, "..") || !strcmp(fdfile.cFileName, "."))
            continue;
        if (fdfile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            return true;
            FindClose(hFind);
        }
    }
    FindClose(hFind);
    return false;
}
void open_folder(const char para[], int round, int ctrl[],string content,int length)
{
    WIN32_FIND_DATAA fdfile;
    HANDLE hFind = FindFirstFileA(para, &fdfile);//第一个参数是路径名，可以使用通配符，fd存储有文件的信息
    if (hFind == INVALID_HANDLE_VALUE)
    {
        cout << "无效的路径 - ";
        for (int i = 2; para[i + 1] != '*'; i++)
        {
            if (para[i] <= 'z' && para[i] >= 'a')
                cout << char(para[i] - 32);
            else
                cout << para[i];
        }
        cout << "\n没有子文件夹\n\n";
        return;
    }
    bool done = true, found_ = search_folder(para);
    if (!found_)
        ctrl[round] = 0;
    int if_is_round1 = 1;
    while (1)
    {
        done = FindNextFileA(hFind, &fdfile);//返回的值如果为0则没有文件要寻了

        if (!done && if_is_round1)
            return;
        if (!strcmp(fdfile.cFileName, "..") || !strcmp(fdfile.cFileName, "."))
            continue;

        if (!done)
            break;
        if_is_round1 = 0;
        if (fdfile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;
        else
        {
            for (int i = 1; i <= round; i++)
                if (ctrl[i])
                    FileTree.append("|   ");
                else
                    FileTree.append("    ");
        }
        FileTree.append(fdfile.cFileName);
        FileTree.append("\n");
        if (!done)
        {
            FileTree.append("\n");
            break;
        }
    }
    for (int i = 1; i <= round; i++)
        if (ctrl[i])
            FileTree.append("|   ");
        else
            FileTree.append("    ");
    FileTree.append("\n");
    FindClose(hFind);
    done = true;
    if (!found_)
    {
        return;
    }


    WIN32_FIND_DATAA fdfolder;
    HANDLE hfFind = FindFirstFileA(para, &fdfolder);
    while (1)
    {
        bool done = true;
        done = FindNextFileA(hfFind, &fdfolder); //返回的值如果为0则没有文件要寻

        if (!done)
            break;
        if (!(fdfolder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || (fdfolder.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
            continue;
        if (!strcmp(fdfolder.cFileName, ".."))
            continue;
        else//找到的是文件夹
        {
            for (int i = 1; i <= round - 1; i++)
                if (ctrl[i])
                    FileTree.append("|   ");
                else
                    FileTree.append("    ");
        }
        char filename[260] = { 0 };
        strcat(filename, fdfolder.cFileName);
        WIN32_FIND_DATAA temp;
        HANDLE tempFind = FindFirstFileA(para, &temp);
        bool final = true;
        while (1)
        {
            final = FindNextFileA(tempFind, &temp);
            if (!strcmp(temp.cFileName, filename))
                break;
        }
        while (1)
        {
            final = FindNextFileA(tempFind, &temp);
            if (temp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                break;
            if (!final)
            {
                //final_folder = true;
                break;
            }

        }
        if (final){
            FileTree.append("+---");
            FileTree.append(fdfolder.cFileName);
            FileTree.append("\n");
        }
        else
        {
            FileTree.append("\\---");
            FileTree.append(fdfolder.cFileName);
            FileTree.append("\n");
            ctrl[round] = 0;
        }
        ctrl[round + 1] = 1;
        char adr[512] = { 0 };
        for (int i = 0; para[i] != '*'; i++)
            adr[i] = para[i];
        strcat(adr, filename);
        bool islinked;
        strcat(adr, "\\*.*");

        string a=findlinkFolder(content, adr, islinked, length);
        qDebug()<<islinked<<endl;
        if (islinked) {
            qDebug()<<QString::fromStdString(a)<<endl;
            memset(adr, 0, 260);
            a.erase(a.length() - 1);
            strcat(adr,a.c_str());
            strcat(adr, "\\*.*");
        }
        qDebug()<<"adr"<<adr<<endl;
        open_folder(adr, round + 1, ctrl,content,length);
        if (!done)
        {
            FindClose(hfFind);
            FileTree.append("\n");
            return;
        }
    }
}
string openFile(string filename,int length) {
    //从文件中拿到名字，如果没有箭头表示没有被绑定
    //否则表示被绑定，替换为后面的内容，调用open_folder
    FileTree.clear();
    ifstream in;
    in.open(filename.c_str(), ios::in);
    int ctrl[20] = { 0,1 };
    string content;
    while (in.good())
        content.insert(content.length(),1,(char)in.get());
    qDebug()<<QString::fromStdString(content)<<endl;
    in.close();
    FileTree.append("Liu-root\n");
    //open_folder("D:\\linux_beta\\*.*",1,ctrl,content,100);
    open_folder("C:\\mycloud\\Liu\\Liu-root\\*.*",1, ctrl, content,length);
    return GbkToUtf8(FileTree.c_str());
}

bool addDir(string filename,string dirname,string clientusrname) {
    fstream out;
    out.open(filename, ios::out | ios::app |ios::in);
    while (1) {
        char buff[1024] = { 0 };
        out.getline(buff, 1024);
        if (!out.good())
            break;
        string child = buff;
        if (child.find(dirname, 0) != child.npos){
            writeLog(logfile,"Add cloud path","fail","Cloud path already exist");
            //cout << "path already exist" << endl;
            return false;
        }
    }
    out.clear();
    out << dirname << endl;
    string path;
    path += path1;
    path += clientusrname;
    path += path2;
    path += dirname;
    path.append("\\");
    MakeSureDirectoryPathExists(path.c_str());
    out.close();
    writeLog(logfile,"Add cloud path","success","Successfully added a cloud path");
    return true;
}
bool bondDir(string filename,string dir1,string dir2) {

    WIN32_FIND_DATAA fdfile;
    HANDLE hFind = FindFirstFileA((dir2+"\\*.*").c_str(), &fdfile);
    if (hFind == INVALID_HANDLE_VALUE) {
        writeLog(logfile,"Bond cloud path","fail","local folder doesn't exist");
        //cout << "file doesn't exist" << endl;
        return false;//目的路径不存在
    }
    int level = 0;
    fstream file;
    file.open(filename.c_str(), ios::in | ios::out);
    for (int i = 0; i < (int)dir1.length(); i++) {
        if (dir1[i] == '\\')
            level++;
    }
    file.seekg(0, ios::beg);
    bool found=false;
    while (1) {
        char buff[1024] = { 0 };
        file.getline(buff, 1024);
        if (!file.good())
            break;

        int cur_level = 0;
        bool bonded = false;
        int i = 0;
        for (i = 0; buff[i] != '\0'; i++) {
            if (buff[i] == '\\')
                cur_level++;
            if (buff[i] == '>') {
                bonded = true;
                break;
            }
        }
        string child = buff;
        //cout << child << endl;
        //cout << cur_level << ' ' << level << endl;
        if (child.find(dir1, 0) != child.npos && bonded == true && cur_level>level) {//子文件夹被绑定了
            writeLog(logfile,"Bond cloud path","fail","A child path have been boned");
            //cout << "child path have been boned" << endl;
            return false;
        }
        if (dir1.find(child.substr(0,i), 0) != dir1.npos && bonded == true && cur_level < level) {//子文件夹被绑定了
            cout << "father path have been boned" << endl;
            return false;
        }
        if (child.find(dir1, 0) != child.npos && cur_level == level)
            found = true;

    }
    file.close();
    if (!found) {
        writeLog(logfile,"Bond cloud path","fail","netdisk folder "+dir1+" dosen't exist");
        //cout << "netdisk folder dosen't exist" << endl;
        return false;
    }
    file.open(filename.c_str(), ios::in | ios::out);
    string content;
    while(1){
        char buff[1024] = { 0 };
        file.getline(buff, 1024);
        if (!file.good())
            break;

        int cur_level = 0;
        int i = 0;
        for (i = 0; buff[i] != '\0'; i++) {
            if (buff[i] == '\\')
                cur_level++;
            if (buff[i] == '>')
                break;
        }
        string line = buff;
        if (line.find(dir1) != line.npos && cur_level == level) {
            buff[i] = '>';
            for (int j = i + 1; buff[j] != '\0'; j++)
                buff[j] = '\0';
            strcat(buff, dir2.c_str());
        }
        content.append(buff);
        content.append("\n");
    }
    file.close();
    file.open(filename.c_str(), ios::out | ios::trunc);
    //cout <<  content << endl;
    file << content;
    file.close();
    writeLog(logfile,"Bond cloud path","success","Successfully bond "+dir2+" to "+dir1);
    return true;
}
bool deleteDir(string filename, string dirname,string clientusrname) {//可以优化 是否删除目录下的全部文件
    fstream out;
    out.open(filename, ios::out | ios::app | ios::in);
    int level = 0;
    for (int i = 0; i < (int)dirname.length(); i++) {
        if (dirname[i] == '\\')
            level++;
    }
    string content;
    bool found=false;
    while (1) {
        char buff[1024] = { 0 };
        out.getline(buff, 1024);
        if (!out.good())
            break;

        int cur_level = 0;
        int i = 0;
        for (i = 0; buff[i] != '\0'; i++) {
            if (buff[i] == '\\')
                cur_level++;
            if (buff[i] == '>')
                break;
        }
        string child = buff;
        //cout << child << endl;
        //cout << cur_level << ' ' << level << endl;
        if (child.find(dirname, 0) != child.npos&&cur_level>level) {
            writeLog(logfile,"Delete cloud path","fail","Other folders in this folder");
            //cout << "other folders in this folder" << endl;
            return false;
        }
        if (child.find(dirname, 0) != child.npos && cur_level == level) {
            found=true;
            continue;

        }
        content += child;
        content.append("\n");
    }
    if (!found) {
        writeLog(logfile,"Delete cloud path","fail",dirname+" dosen't exist");
        //cout << "netdisk folder dosen't exist" << endl;
        return false;
    }
    string path;
    path += path1;
    path += clientusrname;
    path += path2;
    path += dirname;
    path.append("\\");
    RemoveDirectoryA(path.c_str());
    out.close();
    out.open(filename.c_str(), ios::out | ios::trunc);
    out << content;
    out.close();
    writeLog(logfile,"Delete cloud path","success","Successfully deleted a cloud path");
    return true;
}
bool unbondDir(string filename, string dirname) {
    fstream out;
    out.open(filename, ios::out | ios::app | ios::in);
    int level = 0;
    for (int i = 0; i < (int)dirname.length(); i++) {
        if (dirname[i] == '\\')
            level++;
    }
    string content;
    bool found = false;
    while (1) {
        char buff[1024] = { 0 };
        out.getline(buff, 1024);
        if (!out.good())
            break;

        int cur_level = 0;
        //bool bonded = false;
        int i = 0;
        for (i = 0; buff[i] != '\0'; i++) {
            if (buff[i] == '\\')
                cur_level++;
            if (buff[i] == '>')
                break;
        }
        string child = buff;
        //cout << child << endl;
        //cout << cur_level << ' ' << level << endl;
        if (child.find(dirname, 0) != child.npos && cur_level == level) {
            if (buff[i] != '>') {
                writeLog(logfile,"Unbond cloud path","fail",dirname+"isn't bonded");
                //cout << "this folder isn't bonded" << endl;
                return false;
            }
            found = true;
            content += child.substr(0,child.find(">"));
            content.append("\n");
            continue;
        }
        content += child;
        content.append("\n");
    }
    if (!found) {
        writeLog(logfile,"Unbond cloud path","fail",dirname+"doesn't exist");
        //cout << "netdisk folder dosen't exist" << endl;
        return false;
    }
    out.close();
    out.open(filename.c_str(), ios::out | ios::trunc);
    out << content;
    out.close();
    writeLog(logfile,"Unbond cloud path","success","Successfully unbonded a cloud path");
    return true;
}
void createFoldersbyFile(string filename,string dir) {
    string path = dir+"\\";
    fstream file;
    file.open(filename.c_str(), ios::in);
    while (1) {
        char buff[1024] = { 0 };
        file.getline(buff, 1024);
        if (!file.good())
            break;
        int i = 0;
        for (i = 0; buff[i] != '\0'&&buff[i]!='\r'; i++) {
            if (buff[i] == '>') {

                break;
            }
        }
        buff[i] = '\0';
        MakeSureDirectoryPathExists((path+buff+"\\").c_str());
    }
    writeLog(logfile,"Create folders","success","Successfully created folders");
    file.close();
}
void makesureConfigexist() {
    string clientusrname = "Liu";//到时候根据登录名进行修改
    char winusrname[256] = { 0 };
    DWORD dwSize = 256;
    GetUserNameA(winusrname, &dwSize);
    string file;
    file += file1;
    file += clientusrname;
    file += file2;
    fstream in;
    in.open(file.c_str());
    if (in) {
        //createFoldersbyFile(file, file1 + "fyl06" + file2 + clientusrname);
        //bondDir(file,clientusrname + "-root\\folderB", "D:\\linux_beta");
        //addDir(file, "rootB",clientusrname);
        //cout<<deleteDir(file, "Liu-root\\folderA", clientusrname);
        //cout << unbondDir(file, clientusrname + "-root\\folderA");
        in.close();
    }//配置文件存在，可以读取绑定目录
    else {
        string path;
        path += path1;
        path += clientusrname;
        path += path2;
        MakeSureDirectoryPathExists(path.c_str());
        fstream out;
        out.open(file.c_str(), ios::out);
        path += clientusrname;
        path.append("-root\\");
        MakeSureDirectoryPathExists(path.c_str());
        out << clientusrname << "-root" << endl;
        MakeSureDirectoryPathExists((path+"folderA\\").c_str());
        out << clientusrname << "-root\\folderA" << endl;
        MakeSureDirectoryPathExists((path + "folderB\\").c_str());
        out << clientusrname << "-root\\folderB" << endl;
        MakeSureDirectoryPathExists((path + "folderC\\").c_str());
        out << clientusrname << "-root\\folderC" << endl;

        out.close();



    }//进行目录绑定
    return ;
}

void mysleep(int period){
    QTime t;
    t.start();
    while(t.elapsed()<period*100){
        QCoreApplication::processEvents();
    }
}

