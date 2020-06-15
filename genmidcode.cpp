#include <iostream>
#include <cstring>


ofstream four("./output/fourcode.txt");
typedef struct 
{
    char op[8];
    char arg1[128];
    char arg2[128];
    char result[128];
} FourCode;

int codeNum;
int labNum;
int varNum;

FourCode midcode[512];

void init_fourcode()
{
    codeNum=0;
}

int is_number(char *a, int len){
    for(int i=0;i<len;i++){
        if(a[i]<'0'||a[i]>'9')
            return 0;
    }
    return 1;
}

char *genLab()
{
    char *label = (char *)malloc(sizeof(char)*16);
    sprintf(label, "_LABEL_%d", labNum);
    labNum++;
    return label;
}

char *genVar()
{
    char *var = (char *)malloc(sizeof(char)*16);
    sprintf(var, "_ld%d", varNum);
    varNum++;
    return var;
}

void genMidcode(char *op, char *a, char *b, char *result)
{
    strcpy(midcode[codeNum].op, op);
    strcpy(midcode[codeNum].arg1, a);
    strcpy(midcode[codeNum].arg2, b);
    strcpy(midcode[codeNum].result, result);

    //four<<"op= "<<op<<"\tnum_a= "<<a<<"\tnum_b= "<<b<<"\tresult= "<<result<<endl;
    //左对齐，宽度为10输出
    four<<"op= "    <<setw(10)<<setiosflags(ios::left)<<op
        <<"num_a= " <<setw(10)<<setiosflags(ios::left)<<a
        <<"num_b= " <<setw(10)<<setiosflags(ios::left)<<b
        <<"result= "<<setw(10)<<setiosflags(ios::left)<<result
        <<endl;

    codeNum ++;
}




