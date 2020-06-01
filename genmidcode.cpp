#include <iostream>
#include <cstring>

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
char op[12];
FourCode midcode[512];

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
    codeNum ++;
}



