//
// Created by jin on 2020/6/11.
//

#include "grammar.cpp"

using namespace std;

bool isNum(char a){
    return (a>='0' && a<='9');
}

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
void ass_mips();
void bt_mips();
void st_mips();
void eql_mips();
void neq_mips();
void nbt_mips();
void nst_mips();
void array_mips();
void arrayl_mips();


