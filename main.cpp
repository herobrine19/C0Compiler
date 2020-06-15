#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <iomanip>
#include "asm.cpp"
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

        while(!tempstack.empty()){
            tempstack.pop();
        }
        cout<<"****************************************************************************************************************************************************"<<endl;
        printf("Now test%d:\n", i);
        char fileName[30];
        sprintf(fileName, "../test/test%d.txt", i);
        resource = readFileIntoString(fileName);
        //cout<<resource<<endl;
        //int n = resource.length();
        lex_init();
        getnext();
        init_symbol_table();
        init_fourcode();
        _program();
//        for (int j = 0; j < symbolTable.top; j++){
//            symbol << "name= " << symbolTable.elements[j].name << "\ttype= " << symbolTable.elements[j].type << "\tvalue= " << symbolTable.elements[j].value << "\tadress= " << symbolTable.elements[j].address << "\tpara= " << symbolTable.elements[j].para << endl;
//            //symbol<<symbolTable.elements[j].name <<'\t'<< symbolTable.elements[j].type << '\t'<< symbolTable.elements[j].value <<'\t'<< symbolTable.elements[j].address <<'\t'<< symbolTable.elements[j].para << endl;
//        }
        run();
//        for (int j = 0; j < codeNum; j++){
//            if (strcmp(midcode[j].op, " ") == 0){
//                continue;
//            }
//            four << "op=" << midcode[j].op << "\tnum_a=" << midcode[j].arg1 << "\tnum_b=" << midcode[j].arg2 << "\tresult=" << midcode[j].result << endl;
//        }
        printf("Test%d end!\n", i);
        cout<<"****************************************************************************************************************************************************"<<endl;

    }

    return 0;
}