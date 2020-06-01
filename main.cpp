#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include "grammar.cpp"

using namespace std;

//从文件读入到string里
string readFileIntoString(const string& filename)
{
    ifstream ifile(filename);
   
//将文件读入到ostringstream对象buf中
    ostringstream buf;
    char ch;
    while(buf&&ifile.get(ch))
        buf.put(ch);
//返回与流对象buf关联的字符串
    return buf.str();
}


int main()
{
    resource = readFileIntoString("../test/test7.txt");

    cout<<resource<<endl;
    int n = resource.length();
    getnext();
    _program();
    return 0;
}