//
// Created by jin on 2020/6/11.
//

#include "grammar.cpp"

using namespace std;

ofstream asmfile("output/asm.txt");

typedef struct
{
    char name[128];
    char value[128];
} Glbconst; //存放全局常量和变量的名字和值

Glbconst glbconst[128];
int glbNum = 0; //全局变量的数目
int doneNum = 0; //当前翻译的四元式的索引
int preFunc = 0; //当前函数的索引
int paraNum = 0; //当前函数的参数数量


int is_num(char a);
int find_addr(char *name);
void load_num();
void glbconstdef();
void glbvardef();
void initasm();

/**下面是四元式到汇编代码的转换的具体函数**/
void func_mips();
void para_mips();
void int_mips();
void char_mips();
void ret_mips();
void reta_mips();
void end_mips();
void add_mips();
void sub_mips();
void mul_mips();
void div_mips();
void prtf_mips();
void main_mips();
void call_mips();
void scf_mips();
void lab_mips();
void calpara_mips();
void jmp_mips();
void jne_mips();
void assign_mips();
void bt_mips();
void st_mips();
void eql_mips();
void neq_mips();
void nbt_mips();
void nst_mips();
void array_mips();
void arrayl_mips();

void gen_asm();


/**以下是具体的函数定义**/


/**
 * 判断字符是否是数字
 * @param a 字符
 * @return 1或0
 */
int is_num(char a)
{
    if(a>='0' && a<='9')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * 找符号的地址
 * @param symbolName 符号名
 * @return 符号的address属性，若为-1说明是全局变量
 */
int find_addr(char *symbolName)
{
    int i = symbolTable.funcIndex[preFunc];
    int n=0;
    if(preFunc<symbolTable.funcTotal-1)//如果当前的函数不是最后一个
    {
        //n存储下一个函数在符号表中的索引
        n=symbolTable.funcIndex[preFunc+1];
    }
    else
    {
        //n存储符号表最后一个元素的索引
        n=symbolTable.top;
    }
    for(;i<n;i++)
    {
        //找到了局部变量或者函数的参数
        if(strcmp(symbolTable.elements[i].name, symbolName)==0)
        {
            break;
        }
    }
    //局部变量没找到，就是全局变量
    if(i>=n)
    {
        return -1;
    }
    else
    {
        return symbolTable.elements[i].address;
    }
}

/**
 * 把指令的两个操作数载入寄存器t0、t1，后面加减乘除还有条件跳转指令要重复用到，所以提取出来写成函数
 */
void load_num()
{
    int addr1;
    int addr2;
    if((is_num(midcode[doneNum].arg1[0])==1)||midcode[doneNum].arg1[0]=='-')
    {
        asmfile<<"\t\tli\t$t0,\t"<<midcode[doneNum].arg1<<"\t\t\t#取立即数"<<endl;
    }
    else
    {
        addr1=find_addr(midcode[doneNum].arg1);
        if(addr1==-1)
        {
            asmfile <<"\t\tla\t$t0,\t"<<midcode[doneNum].arg1
                    <<"\t\t\t#载入全局变量的地址"<<endl;
            asmfile <<"\t\tlw\t$t0,($t0)"<<endl;
        }
        else
        {
            if(addr1 <= paraNum)
            {
                addr1 = addr1 - paraNum -1;
            }
            else
            {
                addr1 = addr1 - paraNum + 2;
            }
            asmfile <<"\t\tlw\t$t0,"<<-4*addr1<<"($fp)"
                    <<"\t\t\t#加载局部变量"<<endl;
        }
    }
    if(is_num(midcode[doneNum].arg2[0])==1||midcode[doneNum].arg2[0]=='-')
    {
        asmfile <<"\t\tli\t$t1,\t"<<midcode[doneNum].arg2
                <<"\t\t\t#取立即数"<<endl;
    }
    else
    {
        addr2 = find_addr(midcode[doneNum].arg2);
        if(addr2==-1)
        {
            asmfile <<"\t\tla\t$t1,\t"<<midcode[doneNum].arg2
                    <<"#\t\t\tload static number"<<endl;
            asmfile <<"\t\tlw\t$t1,($t1)"<<endl;
        }
        else
        {
            if(addr2<=paraNum)
            {
                addr2 = addr2-paraNum-1;
            }
            else
            {
                addr2 = addr2-paraNum+2;
            }
            asmfile << "\t\tlw\t$t1,"<<-4*addr2<<"($fp)"
                    << "#\t\t\t加载局部变量"<<endl;
        }
    }
}

/**
 * 定义全局常量
 */
void glbconstdef()
{
    int i = 0;
    while(strcmp(midcode[i].op, "const")==0)
    {
        strcpy(glbconst[glbNum].name, midcode[i].result);//保存常量名
        strcpy(glbconst[glbNum].value, midcode[i].arg2);//保存常量值
        //.word:以32位（一个字长）存储数值表
        if(strcmp(midcode[i].arg1,"char")==0)
        {
            asmfile<<midcode[i].result<<":\t.word\t\'"<<midcode[i].arg2<<'\''<<endl;
        }
        else
        {
            asmfile<<midcode[i].result<<":\t.word\t"<<midcode[i].arg2<<endl;
        }
        i++;
    }
    glbNum=i;
}

/**
 * 定义全局变量
 */
void glbvardef()
{
    int i=glbNum; //从全局常量下一个开始
    while(strcmp(midcode[i].op, "int")==0||strcmp(midcode[i].op, "char")==0)
    {
        //全局数组变量
        if(strcmp(midcode[i].arg1, "array")==0)
        {
            strcpy(glbconst[glbNum].name, midcode[i].result);//保存数组名
            strcpy(glbconst[glbNum].value, SPACE);//数组定义没有值
            int size=atoi(midcode[i].arg2);//数组大小
            asmfile<<midcode[i].result<<":\t.space\t"<<size*4<<endl;//开辟内存空间给数组
            glbNum++;
            i++;
        }
        else//普通的全局变量
        {
            strcpy(glbconst[glbNum].name, midcode[i].result);
            strcpy(glbconst[glbNum].value, SPACE);
            asmfile<<midcode[i].result<<":\t.space\t"<<"4"<<endl;//开辟4个字节空间给变量
            glbNum++;
            i++;
        }
    }
}

/**
 * 初始化汇编代码，完成全局变量和常量的设置，还有string的设置
 */
void initasm()
{

    //定义程序的数据段，程序的变量需要在该伪指令下定义，汇编程序会分配和初始化变量的存储空间
    asmfile<<".data"<<endl;
    glbconstdef();//全局常量
    glbvardef();//全局变量
    doneNum = glbNum;
    int i=glbNum;//跳过全局常量查找printf语句
    int stringCnt = 0;
    char temp[128];
    while(i<codeNum)
    {
        if(strcmp(midcode[i].op, "prtf")==0){
            if(strcmp(midcode[i].arg1, SPACE)!=0){
                sprintf(temp, "string%d", stringCnt);
                strcpy(glbconst[glbNum].name, temp);
                strcpy(glbconst[glbNum].value, midcode[i].arg1);
                strcpy(midcode[i].arg1, temp);
                //.asciiz:伪指令，为一个ASCII字符串分配字节序列，并在字符串末尾增加NULL字符
                asmfile<<temp<<":\t.asciiz\t"<<"\""<<glbconst[glbNum].value<<"\""<<endl;
                glbNum++;
                stringCnt++;
            }
        }
        i++;
    }
    asmfile << ".text"<<endl; //接下来定义程序的代码段
    // 声明一个符号位全局的，全局符号可以被其他的文件引用，用该伪指令声明一个程序的main过程
    asmfile << ".global main" << endl;
    asmfile <<"\t\tj\tmain"<<endl; //首先跳转到主函数
}

/**
 * 生成函数定义的汇编代码
 */
void func_mips()
{
    preFunc = search_symbol(midcode[doneNum].result, TYPE_FUNC);
    paraNum = symbolTable.elements[symbolTable.funcIndex[preFunc]].para;
    asmfile << "#函数定义" << endl;
    asmfile << midcode[doneNum].result<<":"<<endl;
    asmfile << "\t\tsw\t$fp, ($sp)"
            << "\t\t\t#push old frame pointer(dynamic link)" <<endl;
    asmfile << "\t\tmove\t$fp, $sp"
            << "\t\t\t#frame pointer now points to the top of stack"<<endl;
    asmfile << "\t\tsubu\t$sp, $sp, 500"
            << "\t\t\t#allocate 500 bytes in the stack"<<endl;
    asmfile << "\t\tsw\t$ra, -4($fp)"
            << "\t\t\t#save return address in frame"<<endl;
    asmfile << "\t\tsw\t$v0, -8($fp)"
            << "\t\t\t#save static link in frame"<<endl;
    asmfile << "#func fours end"<<endl;
}

/**
 * TODO
 */
void para_mips()
{
    asmfile<<"#parameter may be no function"<<endl;
}

/**
 * int 数组定义
 */
void int_mips()
{
    int n;
    if(strcmp(midcode[doneNum].arg1, "array")==0)
    {
        n = atoi(midcode[doneNum].arg2);
        asmfile <<"\t\tsubu\t$sp, $sp, "<<-4*n
                <<"\t\t\t#define array "<<midcode[doneNum].result
                <<endl;
    }
}

/**
 * char 数组定义
 */
void char_mips()
{
    int n;
    if(strcmp(midcode[doneNum].arg1, "array")==0)
    {
        n=atoi(midcode[doneNum].arg2);
        asmfile <<"\t\tsubu\t$sp, $sp, "<<-4*n
                <<"\t\t\t#define array "<<midcode[doneNum].result
                <<endl;
    }
}

/**
 * 无返回值函数返回语句汇编
 */
void ret_mips()
{
    asmfile <<"#函数返回开始"<<endl;
    asmfile <<"\t\tlw\t$ra, -4($fp)"
            <<"\t\t\t#restore return address"<<endl;
    asmfile <<"\t\tmove\t$sp, $fp"
            <<"\t\t\t#pop frame"<<endl;
    asmfile <<"\t\tlw\t$fp,($fp)"
            <<"\t\t\t#restore old frame pointer"<<endl;
    asmfile <<"\t\tjr\t$ra"
            <<"\t\t\t#return"<<endl;
    asmfile <<"#函数返回结束"<<endl;
}

/**
 * 有返回值函数返回语句的汇编
 */
void reta_mips()
{
    asmfile <<"#函数返回开始"<<endl;
    //返回一个数字
    if(is_num(midcode[doneNum].result[0])==1)
    {
        asmfile <<"\t\tli\t$v0,"<<midcode[doneNum].result
                <<"\t\t\t#return number"<<endl;
    }
    else
    {
        int addr = find_addr(midcode[doneNum].result);
        if(addr==-1)
        {
            asmfile << "\t\tla\t$t0,\t"<<midcode[doneNum].result<<endl;
            asmfile <<"\t\tlw\t$v0,($t0)"<<endl;
        }
        else
        {
            if (addr <= paraNum)
            {
                addr = addr - paraNum - 1;
            }
            else
            {
                addr = addr - paraNum + 2;
            }
            asmfile << "\t\tlw\t$v0," << -4 * addr << "($fp)"
                    << "\t\t\t#return var" << endl;
        }
    }
    asmfile << "\t\tlw\t$ra,-4($fp)"
            << "\t\t\t#restore return address" << endl; //restore return address
    asmfile << "\t\tmove\t$sp,$fp"
            << "\t\t\t#pop frame" << endl;
    asmfile << "\t\tlw\t$fp,($fp)"
            << "\t\t\t#restore old frame pointer" << endl;
    asmfile << "\t\tjr\t$ra"
            << "\t\t\t#return" << endl;
    asmfile <<"#函数返回结束"<<endl;
}

/**
 * 函数定义结束
 */
void end_mips()
{
    asmfile<<"#function define end"<<endl;
}

/**
 * 加法的汇编代码
 */
void add_mips()
{
    int addr3;
    load_num();
    asmfile << "\t\tadd\t$t2,$t0,$t1"<<endl;
    addr3 = find_addr(midcode[doneNum].result);
    if(addr3 == -1)
    {
        asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$t2,($t0)" << endl;
    }
    else
    {
        if (addr3 <= paraNum)
        {
            addr3 = addr3 - paraNum - 1;
        }
        else
        {
            addr3 = addr3 - paraNum + 2;
        }
        asmfile << "\t\tsw\t$t2," << -4 * addr3 << "($fp)" << endl;
    }
}

/**
 * 减法的汇编代码
 */
void sub_mips()
{
    int addr3;
    load_num();
    asmfile << "\t\tsub\t$t2,$t0,$t1"<<endl;
    addr3 = find_addr(midcode[doneNum].result);
    if(addr3 == -1)
    {
        asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$t2,($t0)" << endl;
    }
    else
    {
        if (addr3 <= paraNum)
        {
            addr3 = addr3 - paraNum - 1;
        }
        else
        {
            addr3 = addr3 - paraNum + 2;
        }
        asmfile << "\t\tsw\t$t2," << -4 * addr3 << "($fp)" << endl;
    }
}

/**
 * 乘法的汇编代码
 */
void mul_mips()
{
    int addr3;
    load_num();
    asmfile << "\t\tmul\t$t2,$t0,$t1"<<endl;
    addr3 = find_addr(midcode[doneNum].result);
    if(addr3 == -1)
    {
        asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$t2,($t0)" << endl;
    }
    else
    {
        if (addr3 <= paraNum)
        {
            addr3 = addr3 - paraNum - 1;
        }
        else
        {
            addr3 = addr3 - paraNum + 2;
        }
        asmfile << "\t\tsw\t$t2," << -4 * addr3 << "($fp)" << endl;
    }
}

/**
 * 除法的汇编代码
 */
void div_mips()
{
    int addr3;
    load_num();
    asmfile << "\t\tdiv\t$t2,$t0,$t1"<<endl;
    addr3 = find_addr(midcode[doneNum].result);
    if(addr3 == -1)
    {
        asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$t2,($t0)" << endl;
    }
    else
    {
        if (addr3 <= paraNum)
        {
            addr3 = addr3 - paraNum - 1;
        }
        else
        {
            addr3 = addr3 - paraNum + 2;
        }
        asmfile << "\t\tsw\t$t2," << -4 * addr3 << "($fp)" << endl;
    }
}

/**
 * TODO 字符的输出还没写
 */
void prtf_mips()
{
    if(strcmp(midcode[doneNum].arg1, SPACE)!=0)
    {
        asmfile << "\t\tla\t$a0, "<<midcode[doneNum].arg1<<endl;//将要打印的字符串的地址赋值给 $a0
        asmfile << "\t\tli\t$v0,4"<<endl;//4号系统调用，输出字符串
        asmfile <<"\t\tsyscall"<<endl;
    }
    if(strcmp(midcode[doneNum].arg2, SPACE)==0)
    {
        asmfile << "\t\tla\t$a0, \'\\n\'"<<endl;
        asmfile << "\t\tli\t$v0,11"<<endl;
        asmfile <<"\t\tsyscall"<<endl;
        return;
    }
    if(is_num(midcode[doneNum].arg2[0])==1||midcode[doneNum].arg2[0]=='-')
    {
        asmfile << "\t\tli\t$a0,\t" << midcode[doneNum].arg2 //将要打印的整型赋值给 $a0
                << "\t\t\t#load number" << endl;
        asmfile << "\t\tli\t$v0,1" << endl; //1号系统调用，输出整型
        asmfile << "\t\tsyscall" << endl;
    }
    else//寻找变量
    {
        int addr = find_addr(midcode[doneNum].arg2);
        if (addr == -1)
        {
            asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].arg2 << endl;
            asmfile << "\t\tlw\t$a0,($t0)" << endl;
        }
        else //局部变量
        {
            if (addr <= paraNum)
            {
                addr = addr - paraNum - 1;
            }
            else
            {
                addr = addr - paraNum + 2;
            }
            asmfile << "\t\tlw\t$a0," << -4 * addr << "($fp)"
                    << "\t\t\t#return var" << endl;
        }
        //int i=search_symbol(midcode[doneNum].arg2, TYPE_VAR);
        if(strcmp(midcode[doneNum].result, "int")==0)
        {
            asmfile << "\t\tli\t$v0,1" << endl;
        } else
        {
            asmfile << "\t\tli\t$v0,11" << endl;
        }
//        if(symbolTable.elements[i].type==TYPE_CONST_INT)
//        {
//
//        } else if (symbolTable.elements[i].type==TYPE_CONST_CHAR)
//        {
//
//        } else if(symbolTable.elements[i].type==TYPE_VAR)
//        {
//            if(symbolTable.elements[i].value==VALUE_VAR_INT)
//            {
//                asmfile << "\t\tli\t$v0,1" << endl;
//            }
//            else
//            {
//                asmfile << "\t\tli\t$v0,11" << endl;
//            }
//        }
        //asmfile << "\t\tli\t$v0,1" << endl;
        asmfile << "\t\tsyscall" << endl;
    }
    asmfile << "\t\tla\t$a0, \'\\n\'"<<endl;
    asmfile << "\t\tli\t$v0,11"<<endl;
    asmfile <<"\t\tsyscall"<<endl;
}

/**
 * 主函数的汇编代码
 */
void main_mips()
{
    preFunc = search_symbol(midcode[doneNum].result, TYPE_FUNC);
    paraNum = symbolTable.elements[symbolTable.funcIndex[preFunc]].para;
    asmfile << "#main function" <<endl;
    asmfile << midcode[doneNum].result << ":"<<endl;
    asmfile << "\t\tsw\t$fp, ($sp)"
            << "\t\t\t#push old frame pointer(dynamic link)"<<endl;
    asmfile << "\t\tmove\t$fp, $sp"
            << "\t\t\t#frame pointer now points to the top of stack" << endl; //frame pointer now points to the top of stack
    asmfile << "\t\tsubu\t$sp, $sp, 500"
            << "\t\t\t#allocate say 500 bytes in the stack" << endl; // allocate say 100 bytes in the stack
    asmfile << "\t\tsw\t$ra,-4($fp)"
            << "\t\t\t#save return address in frame" << endl; //save return address in frame
    asmfile << "\t\tsw\t$v0,-8($fp)"
            << "\t\t\t#save static link in frame" << endl; //save static link in frame
    asmfile << "#func fours end" << endl;
}

/**
 * 调用函数的指令 TODO 看到这里
 */
void call_mips()
{
    asmfile << "\t\tmove\t$v0,$fp"
            << "\t\t\t#load static link in $v0" << endl;
    // #$ra=PC+8，jump
    asmfile << "\t\tjal\t" << midcode[doneNum].arg1
            << "\t\t\t#call procedure func" << endl;
    asmfile << "\t\tnop" << endl;
    asmfile << "\t\taddu\t$sp,$sp,8"
            << "\t\t\t#pop stack" << endl; //弹栈
    if (strcmp(midcode[doneNum].result, "no") != 0)//有返回值的调用
    {
        int addr1;
        addr1 = find_addr(midcode[doneNum].result);
        if (addr1 == -1)
        {
            asmfile << "\t\tla\t$t1,\t" << midcode[doneNum].result << endl;
            asmfile << "\t\tsw\t$v0,($t1)" << endl; //把返回值存入t1指向的地址
        }
        else
        {
            if (addr1 <= paraNum)
            {
                addr1 = addr1 - paraNum - 1;
            }
            else
            {
                addr1 = addr1 - paraNum + 2;
            }
            asmfile << "\t\tsw\t$v0," << -4 * addr1 << "($fp)"
                    << "\t\t\t#store retuen value" << endl;
        }
    }
}

/**
 * 输入语句的汇编 TODO 字符的输入还没写
 */
void scf_mips()
{
    int addr;
    if(strcmp(midcode[doneNum].arg1, "int")==0)
    {
        asmfile << "\t\tli\t$v0,5" << endl;
    } else{
        asmfile << "\t\tli\t$v0,12" << endl;
    }
    asmfile << "\t\tsyscall" << endl;
    addr = find_addr(midcode[doneNum].result);
    if (addr == -1)
    {
        asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$v0,($t0)" << endl;
    }
    else
    {
        //cout<<"addr:"<<addr<<endl;
        //cout<<"paraNum:"<<paraNum<<endl;
        if (addr <= paraNum)
        {
            addr = addr - paraNum - 1;
        }
        else
        {
            addr = addr - paraNum + 2;
        }
        asmfile << "\t\tsw\t$v0," << -4 * addr << "($fp)" << endl;
    }
}

/**
 * 生成标签的汇编代码
 */
void lab_mips()
{
    asmfile <<midcode[doneNum].result<<":"<<endl;
}

/**
 * 调用函数传参的汇编
 */
void calpara_mips()
{
    if(is_num(midcode[doneNum].result[0]) || midcode[doneNum].result[0]=='-') {
        asmfile << "\t\tli\t$t0,\t" << midcode[doneNum].result << endl;
    }
//    } else if (strlen(midcode[doneNum].result)==1 && is_letter(midcode[doneNum].result[0]))
//    {
//        asmfile << "\t\tli\t$t0,\t" << midcode[doneNum].result << endl;
//    }
    else
    {
        int addr = find_addr(midcode[doneNum].result);
        if (addr == -1)
        {
            asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
            asmfile << "\t\tlw\t$t0,($t0)" << endl;
        }
        else
        {
            if (addr <= paraNum)
            {
                addr = addr - paraNum - 1;
            }
            else
            {
                addr = addr - paraNum + 2;
            }
            asmfile << "\t\tlw\t$t0," << -4 * addr << "($fp)" << endl;
        }
    }

    asmfile << "\t\tsw\t$t0,($sp)" << endl;
    asmfile << "\t\tsubu\t$sp,$sp,4" << endl;
}

/**
 * 强制跳转
 */
void jmp_mips()
{
    asmfile << "\t\tj\t" << midcode[doneNum].result << endl;
}

/**
 * 条件跳转
 */
void jne_mips()
{
    asmfile << midcode[doneNum].result << endl;
}

/**
 * 赋值语句
 */
void assign_mips()
{
    int addr1;
    int addr2;
    if ((is_num(midcode[doneNum].arg1[0]) == 1) || midcode[doneNum].arg1[0] == '-')
    {
        asmfile << "\t\tli\t$t0," << midcode[doneNum].arg1 << endl;
    }else if(strlen(midcode[doneNum].arg1)==1 && is_letter(midcode[doneNum].arg1[0]))
    {
        asmfile << "\t\tli\t$t0,\'" << midcode[doneNum].arg1 <<'\''<< endl;
    }

    else
    {
        addr1 = find_addr(midcode[doneNum].arg1);
        if (addr1 == -1)
        {
            asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].arg1 << endl;
            asmfile << "\t\tlw\t$t0,($t0)" << endl;
        }
        else
        {
            if (addr1 <= paraNum)
            {
                addr1 = addr1 - paraNum - 1;
            }
            else
            {
                addr1 = addr1 - paraNum + 2;
            }
            asmfile << "\t\tlw\t$t0," << -4 * addr1 << "($fp)" << endl;
        }
    }
    addr2 = find_addr(midcode[doneNum].result);
    if (addr2 == -1)
    {
        asmfile << "\t\tla\t$t1,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$t0,($t1)" << endl;
    }
    else
    {
        if (addr2 <= paraNum)
        {
            addr2 = addr2 - paraNum - 1;
        }
        else
        {
            addr2 = addr2 - paraNum + 2;
        }
        asmfile << "\t\tsw\t$t0," << -4 * addr2 << "($fp)" << endl;
    }
}

/**
 * TODO 好像错了
 */
void bt_mips()
{
    load_num();
    asmfile << "\t\tble\t$t0,$t1\t";
}
void st_mips()
{
    load_num();
    asmfile << "\t\tbge\t$t0,$t1\t";
}
void eql_mips()
{
    load_num();
    asmfile << "\t\tbne\t$t0,$t1\t";
}
void neq_mips()
{
    load_num();
    asmfile << "\t\tbeq\t$t0,$t1\t";
}
void nbt_mips()
{
    load_num();
    asmfile << "\t\tbgt\t$t0,$t1\t";
}
void nst_mips()
{
    load_num();
    asmfile << "\t\tblt\t$t0,$t1\t";
}

/**
 * 数组元素在右侧的赋值语句
 * temp=num[j];
 *
 * array ,j     ,num    ,_ld0
 * =     ,ld0   ,       ,temp
 */
void array_mips()
{
    int addr1;
    int addr2;
    int addr3;
    if(is_num(midcode[doneNum].arg1[0])==1)//当j是数字时
    {
        asmfile << "\t\tli\t$t0,\t"<<midcode[doneNum].arg1<<endl;
        asmfile<<"\t\tmul\t$t0,$t0,4"<<endl;
    }
    else
    {
        addr1 = find_addr(midcode[doneNum].arg1);
        if(addr1 == -1)//如果符号表里没找到，那么是全局变量，从内存中取
        {
            asmfile << "\t\tla\t$t0,\t"<<midcode[doneNum].arg1<<endl;
            asmfile << "\t\tlw\t$t0,($t0)"<<endl;
        }
        else
        {
            //paraNum：当前函数的参数总数
            if(addr1 <= paraNum)//如果地址小于当前函数的参数总数，说明这个变量是函数的参数，往fp上找
            {
                addr1 = addr1 - paraNum -1;
            }
            else//这个变量是函数内部的，但是不是传进来的参数，所以往fp下面找
            {
                addr1 = addr1 - paraNum +2;
            }
            asmfile << "\t\tlw\t$t0,"<<-4*addr1<<"($fp)"<<endl;
        }
        asmfile<<"\t\tmul\t$t0,$t0,4"<<endl;
    }
    addr2=find_addr(midcode[doneNum].arg2);
    if(addr2==-1)//全局的数组变量
    {
        asmfile <<"\t\tla\t$t1,\t"<<midcode[doneNum].arg2<<endl;//数组首地址放进t1
        asmfile <<"\t\taddu\t$t1,$t1,$t0"<<endl;//首地址加偏移地址，放到t1里
        asmfile <<"\t\tlw\t$t1,($t1)"<<endl;//取出数组元素放到t1
    }
    else
    {
        if(addr2 <= paraNum)
        {
            addr2 = addr2 - paraNum -1;
        }
        else
        {
            addr2 = addr2 - paraNum + 2;
        }
        asmfile<<"\t\taddi\t$t0,$t0,"<<-4*addr2<<endl;//计算出数组首地址加上偏移地址相对于fp的地址
        asmfile<<"\t\taddu\t$t0,$t0,$fp"<<endl;//计算出数组元素的绝对地址,放t0
        asmfile<<"\t\tlw\t$t1,($t0)"<<endl;//取出来放在t1
    }
    addr3 = find_addr(midcode[doneNum].result);
    if(addr3 == -1)//result是个全局的变量
    {// TODO 感觉t0，t1弄反了
        asmfile << "\t\tla\t$t0,\t"<<midcode[doneNum].result<<endl;//取出这个变量的地址，放到t1
        asmfile << "\t\tsw\t$t1,($t0)"<<endl;//把数组元素的值存到t0所指向的地址里
    }
    else
    {
        if(addr3 <= paraNum)
        {
            addr3 = addr3 - paraNum -1;
        }
        else
        {
            addr3 = addr3 - paraNum + 2;
        }
        asmfile<<"\t\tsw\t$t1,"<<-4*addr3<<"($fp)"<<endl;
        asmfile<<"#array end"<<endl;
    }
}

/**
 * num[i]=a;
 * arrayl,a,i,num
 */
void arrayl_mips()
{
    int addrArg1;
    int addrArg2;
    int addrResult;
    if(is_num(midcode[doneNum].arg1[0])==1 || midcode[doneNum].arg1[0]=='-')
    {
        asmfile <<"\t\tli\t$t0,"<<midcode[doneNum].arg1<<endl;//当a是立即数时，直接取出来放到t0
    }
    else
    {
        addrArg1 = find_addr(midcode[doneNum].arg1);
        if(addrArg1 == -1)//全局变量
        {
            asmfile<<"\t\tla\t$t0,"<<midcode[doneNum].arg1<<endl;//把变量a的地址取出来放进t0
            asmfile<<"\t\tlw\t$t0,($t0)"<<endl;//把t0所指向的数据取出来放进t0
        }
        else//局部变量
        {
            if(addrArg1 <= paraNum)
            {
                addrArg1 = addrArg1 - paraNum -1;
            }
            else
            {
                addrArg1 = addrArg1 - paraNum +2;
            }
            asmfile<<"\t\tlw\t$t0,"<<-4*addrArg1<<"($fp)"<<endl;//根据偏移地址取出来放进t0
        }
    }
    if(is_num(midcode[doneNum].arg2[0])==1||midcode[doneNum].arg2[0]=='-')
    {
        asmfile << "\t\tli\t$t1,\t" << midcode[doneNum].arg2 << endl;//偏移地址是立即数，取出来放进t1
        asmfile << "\t\tmul\t$t1,$t1,4" << endl;//偏移地址乘4
    }
    else
    {
        addrArg2 = find_addr(midcode[doneNum].arg2);
        if(addrArg2==-1)//i是全局变量
        {
            asmfile<<"\t\tla\t$t1,"<<midcode[doneNum].arg2<<endl;//把变量i的地址取出来放在t1
            asmfile<<"\t\tlw\t$t1,($t1)"<<endl;//把变量i的值取出来放在t1里
        }
        else//i是局部变量
        {
            if(addrArg2 <= paraNum)
            {
                addrArg2 = addrArg2 - paraNum -1;
            }
            else
            {
                addrArg2 = addrArg2 - paraNum +2;
            }
            asmfile<<"\t\tlw\t$t1,"<<-4*addrArg2<<"($fp)"<<endl; // 取出i的值放进t1
        }
        asmfile<<"\t\tmul\t$t1,$t1,4"<<endl;//i的值乘4
    }
    addrResult = find_addr(midcode[doneNum].result);
    if(addrResult == -1) //num是全局的数组变量
    {
        asmfile<<"\t\tla\t$t2,\t"<<midcode[doneNum].result<<endl;//数组首地址取出来放进t2
        asmfile<<"\t\taddu\t$t2,$t2,$t1"<<endl;//计算出数组首地址加偏移地址存入t2
        asmfile<<"\t\tsw\t$t0,($t2)"<<endl;//把t0中存的a的值存入数组元素num[i]中
    }
    else
    {
        if(addrResult <= paraNum)
        {
            addrResult = addrResult - paraNum -1;
        }
        else
        {
            addrResult = addrResult - paraNum +2;
        }
        asmfile<<"\t\taddi\t$t1,$t1,"<<-4*addrResult<<endl;//数组首地址加上i
        asmfile<<"\t\taddu\t$t1,$t1,($fp)"<<endl;//num[i]的真实地址
        asmfile<<"\t\tsw\t$t0, ($t1)"<<endl;
    }
}

/**
 * 生成汇编的主入口
 */
void gen_asm()
{
    initasm();
    while (doneNum < codeNum) {
        if(strcmp(midcode[doneNum].op, SPACE)!=0 && strcmp(midcode[doneNum].op, "jne")!=0){
            asmfile <<"#"<<midcode[doneNum].op
                    <<","<<midcode[doneNum].arg1
                    <<","<<midcode[doneNum].arg2
                    <<","<<midcode[doneNum].result
                    <<endl;
        }
        if (strcmp(midcode[doneNum].op, "funct") == 0) {
            func_mips();
        } else if (strcmp(midcode[doneNum].op, "func") == 0) {
            func_mips();
        } else if (strcmp(midcode[doneNum].op, "prtf") == 0) {
            prtf_mips();
        } else if (strcmp(midcode[doneNum].op, "scanf") == 0) {
            scf_mips();
        } else if (strcmp(midcode[doneNum].op, "ret") == 0) {
            ret_mips();
        } else if (strcmp(midcode[doneNum].op, "reta") == 0) {
            reta_mips();
        } else if (strcmp(midcode[doneNum].op, "end") == 0) {
            end_mips();
        } else if (strcmp(midcode[doneNum].op, "MAIN") == 0) {
            main_mips();
        } else if (strcmp(midcode[doneNum].op, "call") == 0) {
            call_mips();
        } else if (strcmp(midcode[doneNum].op, "+") == 0) {
            add_mips();
        } else if (strcmp(midcode[doneNum].op, "-") == 0) {
            sub_mips();
        } else if (strcmp(midcode[doneNum].op, "*") == 0) {
            mul_mips();
        } else if (strcmp(midcode[doneNum].op, "/") == 0) {
            div_mips();
        } else if (strcmp(midcode[doneNum].op, "int") == 0) {
            int_mips();
        } else if (strcmp(midcode[doneNum].op, "char") == 0) {
            char_mips();
        } else if (strcmp(midcode[doneNum].op, "para") == 0) {
            para_mips();
        } else if (strcmp(midcode[doneNum].op, "cpara") == 0) {
            calpara_mips();
        } else if (strcmp(midcode[doneNum].op, "jne") == 0) {
            jne_mips();
        } else if (strcmp(midcode[doneNum].op, "jmp") == 0) {
            jmp_mips();
        } else if (strcmp(midcode[doneNum].op, "=") == 0) {
            assign_mips();
        } else if (strcmp(midcode[doneNum].op, ">") == 0) {
            bt_mips();
        } else if (strcmp(midcode[doneNum].op, "<") == 0) {
            st_mips();
        } else if (strcmp(midcode[doneNum].op, "==") == 0) {
            eql_mips();
        } else if (strcmp(midcode[doneNum].op, "!=") == 0) {
            neq_mips();
        } else if (strcmp(midcode[doneNum].op, "<=") == 0) {
            nbt_mips();
        } else if (strcmp(midcode[doneNum].op, ">=") == 0) {
            nst_mips();
        } else if (strcmp(midcode[doneNum].op, "array") == 0) {
            array_mips();
        } else if (strcmp(midcode[doneNum].op, "arrayl") == 0) {
            arrayl_mips();
        } else if (strcmp(midcode[doneNum].op, "lab") == 0) {
            lab_mips();
        }
        doneNum++;
    }
}