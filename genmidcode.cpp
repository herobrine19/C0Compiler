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
char SPACE[128];

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

/**
 * 把从b开始的所有result这个变量都用a值来替换（当前函数作用域）
 * @param result
 * @param a
 * @param b
 */
void replace(char *result, char *a, int b)
{
    for (int i = b +1; i < codeNum; i++)
    {
        if (strcmp(midcode[i].op, "func") == 0 || strcmp(midcode[i].op, "funct") == 0 || strcmp(midcode[i].op, "MAIN") == 0)
        {
            break;
        }
        else
        {
            if (strcmp(midcode[i].arg1, result) == 0)
            {
                strcpy(midcode[i].arg1, a);
            }
            if (strcmp(midcode[i].arg2, result) == 0)
            {
                strcpy(midcode[i].arg2, a);
            }
        }
    }
}

/**
 * 常数替换
 * @param result
 * @param a
 * @param b
 */
void replace2(char *result, char *a, int b)
{
    for(int i=b+1;i<codeNum;i++)
    {
        if(strcmp(midcode[i].op, "func")==0||strcmp(midcode[i].op, "funct")==0||strcmp(midcode[i].op, "MAIN")==0||strcmp(midcode[i].op, "lab")==0)
        {
            //只能在一个模块里进行替换,如果进入下一个模块就退出
            break;
        }
        else if (strcmp(midcode[i].op, "=")==0 && strcmp(midcode[i].result, result) == 0)
        {
            //如果对result这个变量进行了二次赋值，就不能再常数替换了，break；
            break;
        }
        else
        {
            if(strcmp(midcode[i].arg1, result)==0)
            {
                strcmp(midcode[i].arg1, a);
            }
            if(strcmp(midcode[i].arg2, result)==0)
            {
                strcmp(midcode[i].arg2, a);
            }
        }
    }
}

/**
 * 两个优化，常量运算和常量赋值
 */
void optimize()
{
    int i=0;
    while(i<codeNum)
    {
        if(strcmp(midcode[i].op, "+")==0 && is_number(midcode[i].arg1, strlen(midcode[i].arg1)) && is_number(midcode[i].arg2, strlen(midcode[i].arg2)))
        {
            int a = atoi(midcode[i].arg1)+atoi(midcode[i].arg2);
            char tempResult[64];
            char c[64];
            sprintf(c,"%d",a);
            strcpy(tempResult, midcode[i].result);
            strcpy(midcode[i].op, SPACE);
            strcpy(midcode[i].arg1, SPACE);
            strcpy(midcode[i].arg2, SPACE);
            strcpy(midcode[i].result, SPACE);
            replace(tempResult, c, i);
        }
        else if(strcmp(midcode[i].op, "-")==0 && is_number(midcode[i].arg1, strlen(midcode[i].arg1)) && is_number(midcode[i].arg2, strlen(midcode[i].arg2)))
        {
            int a = atoi(midcode[i].arg1)-atoi(midcode[i].arg2);
            char tempResult[64];
            char c[64];
            sprintf(c,"%d",a);
            strcpy(tempResult, midcode[i].result);
            strcpy(midcode[i].op, SPACE);
            strcpy(midcode[i].arg1, SPACE);
            strcpy(midcode[i].arg2, SPACE);
            strcpy(midcode[i].result, SPACE);
            replace(tempResult, c, i);
        }
        else if(strcmp(midcode[i].op, "*")==0 && is_number(midcode[i].arg1, strlen(midcode[i].arg1)) && is_number(midcode[i].arg2, strlen(midcode[i].arg2)))
        {
            int a = atoi(midcode[i].arg1) * atoi(midcode[i].arg2);
            char tempResult[64];
            char c[64];
            sprintf(c,"%d",a);
            strcpy(tempResult, midcode[i].result);
            strcpy(midcode[i].op, SPACE);
            strcpy(midcode[i].arg1, SPACE);
            strcpy(midcode[i].arg2, SPACE);
            strcpy(midcode[i].result, SPACE);
            replace(tempResult, c, i);
        }
        else if(strcmp(midcode[i].op, "/")==0 && is_number(midcode[i].arg1, strlen(midcode[i].arg1)) && is_number(midcode[i].arg2, strlen(midcode[i].arg2)))
        {
            int a = atoi(midcode[i].arg1) / atoi(midcode[i].arg2);
            char tempResult[64];
            char c[64];
            sprintf(c,"%d",a);
            strcpy(tempResult, midcode[i].result);
            strcpy(midcode[i].op, SPACE);
            strcpy(midcode[i].arg1, SPACE);
            strcpy(midcode[i].arg2, SPACE);
            strcpy(midcode[i].result, SPACE);
            replace(tempResult, c, i);
        }
        else if (strcmp(midcode[i].op, "=")==0 && is_number(midcode[i].arg1, strlen(midcode[i].arg1)))
        {
            char tempResult[64];
            strcpy(tempResult, midcode[i].result);
            replace2(tempResult, midcode[i].arg1, i);
        }
        i++;
    }
}



