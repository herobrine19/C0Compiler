#include <iostream>
#include <cstring>

using namespace std;

//符号表中type字段
#define TYPE_CONST  0
#define TYPE_VAR    1
#define TYPE_FUNC   2
#define TYPE_PARA   3
#define TYPE_ARRAY  4
#define TYPE_STRING 5

//符号表中 value字段
#define VALUE_VAR         -1
#define VALUE_FUNC_VOID   0
#define VALUE_FUNC_INT    1
#define VALUE_FUNC_CHAR   2

const int MAX_SYMBOL_NUM = 512;
const int MAX_FUNC_NUM = 512;

typedef struct
{
    char name[512]; //标识符的名字
    int type;       //0-常量 1-变量 2-函数 3-参数 4-数组 5-string 
    int value;      //常量的值，若type是函数，则0为void，1为int，2为char
    int address;    //标识符存储地址或者地址偏移量
    int para;       //函数参数个数或者数组大小
} Symbol;

typedef struct
{
    Symbol elements[MAX_SYMBOL_NUM];    //符号表
    int top;                            //符号表栈顶指针
    int funcTotal;                      //当前符号表拥有的函数总数
    int funcIndex[MAX_FUNC_NUM];        //函数在表中的索引
} SymbolTable;

SymbolTable symbolTable;

// /**初始化符号表**/
// void init_symbol_table()
// {
//     symbolTable.top = 0;
//     symbolTable.funcIndex[0] = 0;
//     symbolTable.funcTotal = 1;
// }

/**向符号表插入元素**/
void insert_symbol(char *name, int type, int value, int address, int para)
{
    if(symbolTable.top >= MAX_SYMBOL_NUM){
        printf("ERROR:too many symbol\n");
        exit(0);
    }
    int i;
    if(type == TYPE_FUNC)
    {
        for(i=1;i<symbolTable.funcTotal;i++){
            if(strcmp(symbolTable.elements[symbolTable.funcIndex[i]].name, name)==0){
                printf("ERROR:function has been used\n");
                exit(0);
            }
        }
        if(i>=symbolTable.funcTotal){
            symbolTable.funcIndex[symbolTable.funcTotal++] = symbolTable.top;
        }
    }
    else
    {
        i = symbolTable.funcIndex[symbolTable.funcTotal-1];
        for(;i<symbolTable.top;i++){
            if(strcmp(symbolTable.elements[i].name, name)==0){
                printf("ERROR:ID has been named\n");
                exit(0);
            }
        }
    }
    strcpy(symbolTable.elements[symbolTable.top].name, name);
    symbolTable.elements[symbolTable.top].type = type;
    symbolTable.elements[symbolTable.top].value = value;
    symbolTable.elements[symbolTable.top].address = address;
    symbolTable.elements[symbolTable.top].para = para;
    symbolTable.top++;
}

void insert_para(int n)
{
    int i = symbolTable.funcIndex[symbolTable.funcTotal-1];
    symbolTable.elements[i].para = n;
}

int search_symbol(char *name, int type)
{
    int i;
    if(type == TYPE_FUNC)
    {//寻找函数
        for(i=0;i<symbolTable.funcTotal;i++){
            if(strcmp(symbolTable.elements[symbolTable.funcIndex[i]].name, name)==0){
                break;
            }
        }
        if(i>=symbolTable.funcTotal){
            printf("ERROR:undefined function\n");
            exit(0);
        }
        else
        {
            return i;
        }
    }
    else
    {
        i = symbolTable.funcIndex[symbolTable.funcTotal-1];
        for(;i<symbolTable.top;i++){//寻找局部变量
            if(strcmp(symbolTable.elements[i].name, name)==0){
                break;
            }
        }
        if(i>=symbolTable.top){//寻找全局变量
            for(i=0;i<symbolTable.funcIndex[0];i++){//TODO
                if(strcmp(symbolTable.elements[i].name, name)==0){
                    break;
                }
            }
            if(i>=symbolTable.funcIndex[0]){//TODO
                printf("ERROR:undefined id name\n");
                exit(0);
            }
            else
                return i;    
        }
        else
            return i;   
    }
    return -1;
}