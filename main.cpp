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
    for(int i=0;i<19;i++)
    {
        cout<<"**************************************************************************"<<endl;
        printf("Now test%d:\n", i);
        char name[30];
        sprintf(name, "../test/test%d.txt", i);
        resource = readFileIntoString(name);
        //cout<<resource<<endl;
        //int n = resource.length();
        lex_init();
        getnext();
        _program();
        printf("Test%d end!\n", i);
        cout<<"**************************************************************************"<<endl;

    }

    return 0;
}