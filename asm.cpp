//
// Created by jin on 2020/6/11.
//

#include "grammar.cpp"

using namespace std;

ofstream asmfile("asm.txt");
typedef struct
{
    char name[128];
    char value[128];
} Glbconst;

Glbconst glbconst[128];
int glbNum = 0;
int doneNum = 0;
int prefunc = 0;
int prepara = 0;


int isNum(char a);
int findaddr(char *name);
void addrset();
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

void run();


/**以下是具体的函数定义**/

/**
 * 判断是否是数字
 * @param a 字符
 * @return
 */

/**
 * 判断字符是否是数字
 * @param a 字符
 * @return 1或0
 */
int isNum(char a)
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
 * @return 符号的address属性
 */
int findaddr(char *symbolName)
{
    int i = symbolTable.funcIndex[prefunc];
    int n=0;
    if(prefunc<symbolTable.funcTotal-1)
    {
        n=symbolTable.funcIndex[prefunc+1];
    }
    else
    {
        n=symbolTable.top;
    }
    for(;i<n;i++)
    {
        if(strcmp(symbolTable.elements[i].name, symbolName)==0)
        {
            break;
        }
    }
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
 * TODO
 */
void addrset()
{
    int addr1;
    int addr2;
    if((isNum(midcode[doneNum].arg1[0])==1)||midcode[doneNum].arg1[0]=='-')
    {
        asmfile<<"\t\tli\t$t0\t"<<midcode[doneNum].arg1<<"\t\t\t#load number"<<endl;
    }
    else
    {
        addr1=findaddr(midcode[doneNum].arg1);
        if(addr1==-1)
        {
            asmfile <<"\t\tla\t$t0,\t"<<midcode[doneNum].arg1<<midcode[doneNum].arg1
                    <<"\t\t\t#load static var"<<endl;
            asmfile <<"\t\tlw\t$t0,($t0)"<<endl;
        }
        else
        {
            if(addr1 <= prepara)
            {
                addr1 = addr1 - prepara -1;
            }
            else
            {
                addr1 = addr1 - prepara + 2;
            }
            asmfile <<"\t\tlw\t$t0,"<<-4*addr1<<"($fp)"
                    <<"\t\t\t#load local number"<<endl;
        }
    }
    if(isNum(midcode[doneNum].arg2[0])==1||midcode[doneNum].arg2[0]=='-')
    {
        asmfile <<"\t\tli\t$t1\t"<<midcode[doneNum].arg2
                <<"\t\t\t#load number"<<endl;
    }
    else
    {
        addr2 = findaddr(midcode[doneNum].arg2);
        if(addr2==-1)
        {
            asmfile <<"\t\tla\t$t1,\t"<<midcode[doneNum].arg2
                    <<"#\t\t\tload static number"<<endl;
            asmfile <<"\t\tlw\t$t1,($t1)"<<endl;
        }
        else
        {
            if(addr2<=prepara)
            {
                addr2 = addr2-prepara-1;
            }
            else
            {
                addr2 = addr2-prepara+2;
            }
            asmfile << "\t\tlw\t$t1,"<<-4*addr2<<"($fp)"
                    << "#\t\t\tload local number"<<endl;
        }
    }
}

/**
 * 生成常量的汇编代码
 */
void glbconstdef()
{
    int i = 0;
    while(strcmp(midcode[i].op, "const")==0)
    {
        strcpy(glbconst[glbNum].name, midcode[i].result);
        strcpy(glbconst[glbNum].value, midcode[i].arg2);
        asmfile<<midcode[i].result<<":\t.word\t"<<midcode[i].arg2<<endl;
        i++;
    }
}

/**
 * 生成全局变量的代码
 */
void glbvardef()
{
    int i=glbNum; //从全局常量下一个开始
    while(strcmp(midcode[i].op, "int")==0||strcmp(midcode[i].op, "char")==0)
    {
        if(strcmp(midcode[i].arg1, "array")==0)
        {
            strcpy(glbconst[glbNum].name, midcode[i].result);
            strcpy(glbconst[glbNum].value, " ");
            int step=atoi(midcode[i].arg2);
            asmfile<<midcode[i].result<<":\t.space\t"<<step*4<<endl;
            glbNum++;
            i++;
        }
        else
        {
            strcpy(glbconst[glbNum].name, midcode[i].result);
            strcpy(glbconst[glbNum].value, " ");
            asmfile<<midcode[i].result<<":\tspace\t"<<"4"<<endl;
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
    asmfile<<".data"<<endl;
    glbconstdef();//全局常量
    glbvardef();//全局变量
    doneNum = glbNum;
    int i=glbNum;//跳过全局常量查找printf语句
    int stringcnt = 0;
    char temp[128];
    while(i<codeNum)
    {
        if(strcmp(midcode[i].op, "prtf")==0){
            if(strcmp(midcode[i].arg1, " ")!=0){
                sprintf(temp, "string%d", stringcnt);
                strcpy(glbconst[glbNum].name, temp);
                strcpy(glbconst[glbNum].value, midcode[i].arg1);
                strcpy(midcode[i].arg1, temp);
                asmfile<<temp<<":\t.asciiz\t"<<"\""<<glbconst[glbNum].value<<"\""<<endl;
                glbNum++;
                stringcnt++;
            }
        }
        i++;
    }
    asmfile << ".text"<<endl;
    asmfile << ".globl main" << endl;
    asmfile <<"\t\tj\tmain"<<endl;
}

/**
 * 函数的汇编代码生成
 */
void func_mips()
{
    prefunc = search_symbol(midcode[doneNum].result, TYPE_FUNC);
    prepara = symbolTable.elements[symbolTable.funcIndex[prefunc]].para;
    asmfile << "#function declaration" << endl;
    asmfile << midcode[doneNum].result<<":"<<endl;
    asmfile << "\t\tsw\t$fp, ($sp)"
            << "\t\t\t#push old frame pointer(dynamic link)" <<endl;
    asmfile << "\t\tmove\t$fp, $sp"
            << "\t\t\t#frame pointer now points to the top of stack"<<endl;
    asmfile << "\t\tsubu\t$sp, $sp, 500"
            << "\t\t\t#allocate say 500 bytes in the stack"<<endl;
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
        asmfile<<"\t\tsubu\t$sp, $sp, "<<-4*n<<"\t\t\t#define array "<<midcode[doneNum].result<<endl;
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
        asmfile<<"\t\tsubu\t$sp, $sp, "<<-4*n<<"\t\t\t#define array "<<midcode[doneNum].result<<endl;
    }
}

/**
 * 无返回值函数返回语句汇编
 */
void ret_mips()
{
    asmfile <<"\t\tlw\t$ra, -4($fp)"
            <<"\t\t\t#restore return address"<<endl;
    asmfile <<"\t\tmove\t$sp, $fp"
            <<"\t\t\t#pop frame"<<endl;
    asmfile <<"\t\tlw\t$fp,($fp)"
            <<"\t\t\trestore old frame pointer"<<endl;
    asmfile <<"\t\tjr\t$ra"
            <<"\t\t\t#return"<<endl;
}

/**
 * 有返回值函数返回语句的汇编
 */
void reta_mips()
{
    //返回一个数字
    if(isNum(midcode[doneNum].result[0])==1)
    {
        asmfile <<"\t\tli\t$v0,"<<midcode[doneNum].result
                <<"\t\t\t#return number"<<endl;
    }
    else
    {
        int addr = findaddr(midcode[doneNum].result);
        if(addr==-1)
        {
            asmfile << "\t\tla\t$t0,\t"<<midcode[doneNum].result<<endl;
            asmfile <<"\t\tlw\t$v0,($t0)"<<endl;
        }
        else
        {
            if (addr <= prepara)
            {
                addr = addr - prepara - 1;
            }
            else
            {
                addr = addr - prepara + 2;
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
}

/**
 * 函数结束
 */
void end_mips()
{
    asmfile<<"#function end"<<endl;
}

/**
 * 加法的汇编代码
 */
void add_mips()
{
    int addr3;
    addrset();
    asmfile << "\t\tadd\t$t2,$t0,$t1"<<endl;
    addr3 = findaddr(midcode[doneNum].result);
    if(addr3 == -1)
    {
        asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$t2,($t0)" << endl;
    }
    else
    {
        if (addr3 <= prepara)
        {
            addr3 = addr3 - prepara - 1;
        }
        else
        {
            addr3 = addr3 - prepara + 2;
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
    addrset();
    asmfile << "\t\tsub\t$t2,$t0,$t1"<<endl;
    addr3 = findaddr(midcode[doneNum].result);
    if(addr3 == -1)
    {
        asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$t2,($t0)" << endl;
    }
    else
    {
        if (addr3 <= prepara)
        {
            addr3 = addr3 - prepara - 1;
        }
        else
        {
            addr3 = addr3 - prepara + 2;
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
    addrset();
    asmfile << "\t\tmul\t$t2,$t0,$t1"<<endl;
    addr3 = findaddr(midcode[doneNum].result);
    if(addr3 == -1)
    {
        asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$t2,($t0)" << endl;
    }
    else
    {
        if (addr3 <= prepara)
        {
            addr3 = addr3 - prepara - 1;
        }
        else
        {
            addr3 = addr3 - prepara + 2;
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
    addrset();
    asmfile << "\t\tdiv\t$t2,$t0,$t1"<<endl;
    addr3 = findaddr(midcode[doneNum].result);
    if(addr3 == -1)
    {
        asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$t2,($t0)" << endl;
    }
    else
    {
        if (addr3 <= prepara)
        {
            addr3 = addr3 - prepara - 1;
        }
        else
        {
            addr3 = addr3 - prepara + 2;
        }
        asmfile << "\t\tsw\t$t2," << -4 * addr3 << "($fp)" << endl;
    }
}

/**
 * TODO
 */
void prtf_mips()
{
    if(strcmp(midcode[doneNum].arg1, SPACE)!=0)
    {
        asmfile << "\t\tla\t$a0, "<<midcode[doneNum].arg1<<endl;
        asmfile << "\t\tli\t$v0,4"<<endl;//将要打印的字符串的地址赋值给 $a0
        asmfile <<"\t\tsyscall"<<endl;
    }
    if(strcmp(midcode[doneNum].arg2, SPACE)==0)
    {
        return;
    }
    if(isNum(midcode[doneNum].arg2[0])==1||midcode[doneNum].arg2[0]=='-')
    {
        asmfile << "\t\tli\t$a0\t" << midcode[doneNum].arg2 << "\t\t\t#load number" << endl;
        asmfile << "\t\tli\t$v0,1" << endl;//将要打印的整型赋值给 $a0
        asmfile << "\t\tsyscall" << endl;
    }
    else//TODO 不是很懂
    {
        int addr = findaddr(midcode[doneNum].arg2);
        if (addr == -1)
        {
            asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].arg2 << endl;
            asmfile << "\t\tlw\t$a0,($t0)" << endl;
        }
        else
        {
            if (addr <= prepara)
            {
                addr = addr - prepara - 1;
            }
            else
            {
                addr = addr - prepara + 2;
            }
            asmfile << "\t\tlw\t$a0," << -4 * addr << "($fp)"
                    << "\t\t\t#return var" << endl;
        }
        asmfile << "\t\tli\t$v0,1" << endl;
        asmfile << "\t\tsyscall" << endl;
    }
}

/**
 * 主函数的汇编代码
 */
void main_mips()
{
    prefunc = search_symbol(midcode[doneNum].result, TYPE_FUNC);
    prepara = symbolTable.elements[symbolTable.funcIndex[prefunc]].para;
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
 * 调用函数的指令
 */
void call_mips()
{
    asmfile << "\t\tmove\t$v0,$fp"
            << "\t\t\t#load static link in $v0" << endl;
    asmfile << "\t\tjal\t" << midcode[doneNum].arg1 << "\t\t\t#call procedure func" << endl;
    asmfile << "\t\tnop" << endl;
    asmfile << "\t\taddu\t$sp,$sp,8"
            << "\t\t\t#pop stack" << endl;
    if (strcmp(midcode[doneNum].result, "no") != 0)
    {
        int addr1;
        addr1 = findaddr(midcode[doneNum].result);
        if (addr1 == -1)
        {
            asmfile << "\t\tla\t$t1,\t" << midcode[doneNum].result << endl;
            asmfile << "\t\tsw\t$v0,($t1)" << endl;
        }
        else
        {
            if (addr1 <= prepara)
            {
                addr1 = addr1 - prepara - 1;
            }
            else
            {
                addr1 = addr1 - prepara + 2;
            }
            asmfile << "\t\tsw\t$v0," << -4 * addr1 << "($fp)"
                    << "\t\t\t#store retuen value" << endl;
        }
    }
}

/**
 * 输入语句的汇编
 */
void scf_mips()
{
    int addr;
    asmfile << "\t\tli\t$v0,5" << endl;
    asmfile << "\t\tsyscall" << endl;
    addr = findaddr(midcode[doneNum].result);
    if (addr == -1)
    {
        asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$v0,($t0)" << endl;
    }
    else
    {
        if (addr <= prepara)
        {
            addr = addr - prepara - 1;
        }
        else
        {
            addr = addr - prepara + 2;
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
    int addr = findaddr(midcode[doneNum].result);
    if (addr == -1)
    {
        asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tlw\t$t0,($t0)" << endl;
    }
    else
    {
        if (addr <= prepara)
        {
            addr = addr - prepara - 1;
        }
        else
        {
            addr = addr - prepara + 2;
        }
        asmfile << "\t\tlw\t$t0," << -4 * addr << "($fp)" << endl;
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
    if ((isNum(midcode[doneNum].arg1[0]) == 1) || midcode[doneNum].arg1[0] == '-')
    {
        asmfile << "\t\tli\t$t0\t" << midcode[doneNum].arg1 << endl;
    }
    else
    {
        addr1 = findaddr(midcode[doneNum].arg1);
        if (addr1 == -1)
        {
            asmfile << "\t\tla\t$t0,\t" << midcode[doneNum].arg1 << endl;
            asmfile << "\t\tlw\t$t0,($t0)" << endl;
        }
        else
        {
            if (addr1 <= prepara)
            {
                addr1 = addr1 - prepara - 1;
            }
            else
            {
                addr1 = addr1 - prepara + 2;
            }
            asmfile << "\t\tlw\t$t0," << -4 * addr1 << "($fp)" << endl;
        }
    }
    addr2 = findaddr(midcode[doneNum].result);
    if (addr2 == -1)
    {
        asmfile << "\t\tla\t$t1,\t" << midcode[doneNum].result << endl;
        asmfile << "\t\tsw\t$t0,($t1)" << endl;
    }
    else
    {
        if (addr2 <= prepara)
        {
            addr2 = addr2 - prepara - 1;
        }
        else
        {
            addr2 = addr2 - prepara + 2;
        }
        asmfile << "\t\tsw\t$t0," << -4 * addr2 << "($fp)" << endl;
    }
}

/**
 * TODO 好像错了
 */
void bt_mips()
{
    addrset();
    asmfile << "\t\tble\t$t0,$t1\t";
}
void st_mips()
{
    addrset();
    asmfile << "\t\tbge\t$t0,$t1\t";
}
void eql_mips()
{
    addrset();
    asmfile << "\t\tbne\t$t0,$t1\t";
}
void neq_mips()
{
    addrset();
    asmfile << "\t\tbeq\t$t0,$t1\t";
}
void nbt_mips()
{
    addrset();
    asmfile << "\t\tbgt\t$t0,$t1\t";
}
void nst_mips()
{
    addrset();
    asmfile << "\t\tblt\t$t0,$t1\t";
}

void array_mips()
{
    //TODO
    return;
}

void arrayl_mips()
{
    //TODO
    return;
}


/**
 * 生成汇编的主入口
 */
void run()
{
    initasm();
    while (doneNum < codeNum)
    {
        if(strcmp(midcode[doneNum].op, "funct")==0){
            func_mips();
        } else if (strcmp(midcode[doneNum].op, "func")==0){
            func_mips();
        } else if (strcmp(midcode[doneNum].op, "prtf")==0){
            prtf_mips();
        } else if (strcmp(midcode[doneNum].op, "scanf")==0){
            scf_mips();
        } else if (strcmp(midcode[doneNum].op, "ret")==0){
            ret_mips();
        } else if (strcmp(midcode[doneNum].op, "reta")==0){
            reta_mips();
        } else if (strcmp(midcode[doneNum].op, "end")==0){
            end_mips();
        } else if (strcmp(midcode[doneNum].op, "MAIN")==0){
            main_mips();
        } else if (strcmp(midcode[doneNum].op, "call")==0){
            call_mips();
        } else if (strcmp(midcode[doneNum].op, "+")==0){
            add_mips();
        } else if (strcmp(midcode[doneNum].op, "-")==0){
            sub_mips();
        } else if (strcmp(midcode[doneNum].op, "*")==0){
            mul_mips();
        } else if (strcmp(midcode[doneNum].op, "/")==0){
            div_mips();
        } else if (strcmp(midcode[doneNum].op, "int")==0){
            int_mips();
        } else if (strcmp(midcode[doneNum].op, "char")==0){
            char_mips();
        } else if (strcmp(midcode[doneNum].op, "para")==0){
            para_mips();
        }else if (strcmp(midcode[doneNum].op, "cpara")==0){
            calpara_mips();
        } else if (strcmp(midcode[doneNum].op, "jne")==0){
            jne_mips();
        } else if (strcmp(midcode[doneNum].op, "jmp")==0){
            jmp_mips();
        } else if (strcmp(midcode[doneNum].op, "=")==0){
            assign_mips();
        } else if (strcmp(midcode[doneNum].op, ">")==0){
            bt_mips();
        } else if (strcmp(midcode[doneNum].op, "<")==0){
            st_mips();
        } else if (strcmp(midcode[doneNum].op, "==")==0){
            eql_mips();
        } else if (strcmp(midcode[doneNum].op, "!=")==0){
            neq_mips();
        } else if (strcmp(midcode[doneNum].op, "<=")==0){
            nbt_mips();
        } else if (strcmp(midcode[doneNum].op, ">=")==0){
            nst_mips();
        } else if (strcmp(midcode[doneNum].op, "array")==0){
            array_mips();
        } else if (strcmp(midcode[doneNum].op, "arrayl")==0){
            arrayl_mips();
        } else if (strcmp(midcode[doneNum].op, "lab")==0){
            lab_mips();
        }
        doneNum++;
    }
}