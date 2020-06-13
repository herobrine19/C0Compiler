#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include "error.cpp"
using namespace std;
#define INT 1       //int
#define CHAR 2      //char
#define VOID 3      //void
#define CONST 4     //const
#define MAIN 5      //main
#define IF 6        //if
#define ELSE 7      //else
#define WHILE 8     //while
#define SWITCH 9    //switch
#define CASE 10     //case
#define SCANF 11    //scanf
#define PRINTF 12   //printf
#define RETURN 13   //return
#define ID 14       //id
#define BIGTH 15    //>
#define SMALLTH 16  //<
#define NOTSAMLL 17 //>=
#define NOTBIG 18   //<=
#define NOTEQUL 19  // !=
#define DOUEQUL 20  //==
#define EQUAL 21    //=
#define PLUS 22     //+
#define MINUS 23    //-
#define MULT 24     //*
#define DIV 25      // /
#define COMMA 26    //,
#define LBPAREN 27  //{
#define RBPAREN 28  //}
#define LMPAREN 29  //[
#define RMPAREN 30  //]
#define LPAREN 31   //(
#define RPAREN 32   //)
#define COLON 33    //:
#define SEMIC 34    //;
#define STRING 35   // 字符串
#define NUMBER 36   // 整数
#define CHARSYM 37  // 单个字符
#define SQUOTE 38   //'
#define DQUOTE 39   //"

typedef struct 
{
    string name;    // token名
    int nameid;     // token的id
    string id;      // 代码中的token
    int value;      // 值
} Token;

const int INTMAX = 2147483647;
Token token;
int filePoint = 0; // 文件指针
int lineIndex = 1; // 行号
string resource = ""; // 源代码
string reservedWord[] = {"int", "char", "void", "const", "main", "if", "else", "while", "switch", "case", "scanf", "printf", "return"}; // 保留字
string reservedName[] = {"INT", "CHAR", "VOID", "CONST", "MAIN", "IF", "ELSE", "WHILE", "SWITCH", "CASE", "SCANF", "PRINTF", "RETURN"};

ofstream tokenFile("token.txt");

bool is_digit(char a)
{
    return (a>='0' && a<='9');
}

bool is_letter(char a)
{
    return ((a>='a' && a<='z') || (a>='A' && a<='Z') || a=='_');
}

/**判断是否为保留字**/
int searchReservedWord(char temp[])
{
    for(int i=0;i<13;i++)
    {
        if(reservedWord[i].compare(temp) == 0)
        {
            return i;
        }
    }
    return -1;  
}

/**
 *
 * @param name
 * @param nameid
 * @param id
 * @param value
 */
void writeToken(string name, int nameid, string id, int value)
{
    token.name = name;
    token.nameid = nameid;
    token.id = id;
    token.value = value;
    //cout<<id<<endl;
    tokenFile<<name<<'\t'<<nameid<<'\t'<<id<<'\t'<<value<<endl;
}

void lex_init()
{
    filePoint = 0;
    lineIndex = 1;
}

/**获取下一个token**/
void getnext()
{
    char ch = resource[filePoint++];
    while(ch=='\n' || ch=='\t' || ch=='\v' || ch==' ' || ch=='\r')
    {
        if(ch=='\n')
        {
            lineIndex++; //记录行号
        }
        ch = resource[filePoint++];
    }
    if(is_digit(ch))
    {
        long long tmp=ch-'0';
        int flag = (ch=='0'?1:0);
        ch = resource[filePoint++];
        while(is_digit(ch))
        {
            tmp = tmp*10+(ch-'0');
            ch = resource[filePoint++];
        }
        if(flag == 1 && tmp!=0)
        {
            error(ZERO_START_ERROR, lineIndex);
            exit(0);
        }
        if(tmp > INTMAX)
        {
            error(OUT_OF_BOUND_ERROR, lineIndex);
            exit(0);
        }        
        writeToken("NUMBER", NUMBER, "num", tmp);
        filePoint--;//指针回退一格，很重要，防止文件末尾跳过换行
        return;
    }
    else if(is_letter(ch))
    {
        char tmps[100];
        int i = 1;
        tmps[0] = ch;
        ch = resource[filePoint++];
        while(is_letter(ch) || is_digit(ch))
        {
            tmps[i++] = ch;
            ch = resource[filePoint++];
        }
        tmps[i] = '\0';
        int n = searchReservedWord(tmps);
        if(n==-1)
        {
            writeToken("ID", ID, tmps, 0);
            filePoint--;
            return;
        }
        else
        {
            writeToken(reservedName[n], n+1, reservedWord[n], 0);
            filePoint--;
            return;
        }    
    }
    else if(ch == '>')
    {
        ch = resource[filePoint];
        if(ch == '=')
        {
            filePoint++;
            writeToken("NOTSMALL", NOTSAMLL, ">=", 0);
            return;
        }
        else
        {
            writeToken("BIGTH", BIGTH, ">", 0);
            return;
        }
    }
    else if(ch == '<')
    {
        ch = resource[filePoint];
        if(ch == '=')
        {
            filePoint++;
            writeToken("NOTBIG", NOTBIG, "<=", 0);
            return;
        }
        else
        {
            writeToken("SMALLTH", SMALLTH, "<", 0);
            return;
        }
    }
    else if(ch == '!'){
        ch = resource[filePoint++];
        if(ch!='='){
            error(INVALID_CHARACTER_ERROR, lineIndex);
            exit(0);
        }
        writeToken("NOTEQUAL", NOTEQUL, "!=", 0);
        return;
    }
    else if(ch=='='){
        ch = resource[filePoint];
        if(ch=='='){
            filePoint++;
            writeToken("DOUEQUAL", DOUEQUL, "==", 0);
            return;
        }
        else
        {
            writeToken("EQUAL", EQUAL, "=", 0);
            return ;
        } 
    }
    else if(ch=='+'){
        writeToken("PLUS", PLUS, "+", 0);
        return;
    }
    else if(ch == '-'){
        writeToken("MINUS", MINUS, "-", 0);
        return;
    }
    else if(ch == '*'){
        writeToken("MULT", MULT, "*", 0);
        return ;
    }
    else if(ch == '/'){
        writeToken("DIV", DIV, "/", 0);
        return;
    }
    else if(ch == ','){
        writeToken("COMMA", COMMA, ",", 0);
        return;
    }
    else if(ch=='{'){
        writeToken("LBPAREN", LBPAREN, "{", 0);
        return;
    }
    else if(ch=='}'){
        writeToken("RBPAREN", RBPAREN, "}", 0);
        return;
    }
    else if(ch=='['){
        writeToken("LMPAREN", LMPAREN, "[", 0);
        return;
    }
    else if(ch==']')
    {
        writeToken("RMPAREN", RMPAREN, "]", 0);
        return;
    }
    else if(ch=='(')
    {
        writeToken("LPAREN", LPAREN, "(", 0);
        return;
    }
    else if(ch==')')
    {
        writeToken("RPAREN", RPAREN, ")", 0);
        return;
    }
    else if(ch==':'){
        writeToken("COLON", COLON, ":", 0);
        return;
    }
    else if(ch==';'){
        writeToken("SEMIC", SEMIC, ";", 0);
        return;
    }
    else if(ch=='\'')
    {
        char tmps[4];
        ch = resource[filePoint++];
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || is_digit(ch) || is_letter(ch))
        {
            tmps[0] = ch;
            ch = resource[filePoint++];
            if (ch == '\'')
            {
                tmps[1] = '\0';
                writeToken("CHARSYM", CHARSYM, tmps, tmps[0]);
                return;
            }
            else
            {
                error(NO_MATCH_SQ_ERROR, lineIndex);
                exit(0);
            }
        }
        else
        {
            error(INVALID_CHAR_ERROR, lineIndex);
            exit(0);
        }
    }
    else if(ch=='\"')
    {
        char tmps[256];
        ch = resource[filePoint++];
        int i=0;
        while(ch==32 || ch==33 || (ch>=35 && ch<=126)){
            tmps[i++] = ch;
            ch = resource[filePoint++];           
        }
        if(ch=='\"')
        {
            tmps[i]='\0';
            writeToken("STRING", STRING, tmps, 0);
            return;
        }
        else{
            error(NO_MATCH_DQ_ERROR, lineIndex);
            exit(0);
        }
    }
    else if(ch=='\0'){
        printf("程序末尾\n");
    }else{
        error(INVALID_CHARACTER_ERROR, lineIndex);
        exit(0);
    } 
}