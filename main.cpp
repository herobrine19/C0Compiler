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
    for(int i=1;i<2;i++)
    {
        ofstream symbol("./symbol.txt");
        cout<<"**************************************************************************"<<endl;
        printf("Now test%d:\n", i);
        char fileName[30];
        sprintf(fileName, "../test/test%d.txt", i);
        resource = readFileIntoString(fileName);
        //cout<<resource<<endl;
        //int n = resource.length();
        init_symbol_table();
        lex_init();
        getnext();
        _program();
        for (int j = 0; j < symbolTable.top; j++)
        {
            symbol << "name= " << symbolTable.elements[j].name << " type= " << symbolTable.elements[j].type << " value= " << symbolTable.elements[j].value << " adress= " << symbolTable.elements[j].address << " para= " << symbolTable.elements[j].para << endl;
        }
        printf("Test%d end!\n", i);
        cout<<"**************************************************************************"<<endl;

    }

    return 0;
}