#include <stack>
#include "lex.cpp"
#include "symbol.cpp"
#include "genmidcode.cpp"

/**保存现场用的**/
int preFilePoint;   //暂时记录文件指针
int preLineIndex;   //暂时记录文件行号
Token preToken;     //暂时记录token

/**插入符号表用的临时变量**/
char name[512];
int type;
int value;
int address;
int paranum;
int arrsize;
int num = 0;

/**中间代码生成**/
char op[8];
char arg1[128];
char arg2[128];
char result[128];

char switchlabel[16];
char switchvalue[16];
char lastlab[16];
stack<string> tempstack;

//保存现场
void save_scene()
{
    preFilePoint = filePoint;
    preLineIndex = lineIndex;
    preToken.id = token.id;
    preToken.name = token.name;
    preToken.nameid = token.nameid;
    preToken.value = token.value;
}

//还原现场
void restore_scene()
{
    filePoint = preFilePoint;
    lineIndex = preLineIndex;
    token.id = preToken.id;
    token.name = preToken.name;
    token.nameid = preToken.nameid;
    token.value = preToken.value;
}

/**递归下降语法分析**/
void _program();         //程序
void _const_dec();       //常量声明 
void _var_dec();         //变量声明
void _val_func_def();    //有返回值函数定义
void _void_func_def();   //无返回值函数定义
void _main_func();       //主函数
void _const_def();       //常量定义
void _var_def();         //变量定义
void _dec_head();        //声明头部
void _para_list();       //参数表
void _comp_state();      //复合语句
void _state_list();      //语句列
void _if_state();        //条件语句
void _state();           //语句
void _condition();       //条件
//void rela_op();         //关系运算符
void _loop_state();      //循环语句
void _switch_state();    //情况语句
void _situation_list();  //情况表
void _case_state();      //情况子语句
void _val_fun_call();    //有返回值函数调用语句1
void _val_fun_call2();   //有返回值函数调用语句2
void _void_fun_call();   //无返回值函数调用语句
void _val_para_list();   //值参数表
void _assign_state();    //赋值语句
void _read_state();      //读语句
void _write_state();     //写语句
void _return_state();    //返回语句
void _expr();            //表达式
void _term();            //项
void _factor();          //因子


//各部分递归子程序
void _program() //程序
{
    if(token.nameid == CONST){//常量声明
        _const_dec();
    }
    while(token.nameid == INT || token.nameid == CHAR) //变量声明部分
    {
        save_scene();   //因为int或者char开头的可能是变量声明或者函数声明，那种情况需要恢复现场
        getnext();
        if(token.nameid != ID){
            error(ID_ERROR, lineIndex);
            exit(0);
        }
        getnext();
        if(token.nameid == LPAREN){ // 检测到小括号，确定是函数声明
            restore_scene(); //还原现场，跳出循环，进入函数
            break;
        }
        else if(token.nameid == SEMIC || token.nameid == COMMA || token.nameid == LMPAREN)
        {//进入变量声明
            restore_scene();
            strcpy(op, token.id.c_str());
            _var_def();// TODO 可能是_var_dec();
            if(token.nameid !=SEMIC){
                error(SEMI_ERROR, lineIndex);
                exit(0);
            }else{
                getnext();
            }
        }else{
            error(ID_DEC_ERROR, lineIndex);
            exit(0);
        }
    }
    while(token.nameid == INT || token.nameid == CHAR || token.nameid == VOID)
    {
        if(token.nameid == INT || token.nameid == CHAR){
            _val_func_def();
            genMidcode("end", "func", SPACE, SPACE);
        }else{
            save_scene();
            getnext();
            if(token.nameid == MAIN){
                restore_scene();
                break;
            }else{
                restore_scene();
                _void_func_def();
                genMidcode("end", "func", SPACE, SPACE);
            }
        }
    }
    _main_func();
    genMidcode("end", "func", SPACE, SPACE);
    printf("<程序>\n");
}

void _var_dec()
{
    while(token.nameid == INT || token.nameid == CHAR){
        strcpy(op, token.id.c_str());
        _var_def();
        if(token.nameid == SEMIC){
            printf("<变量声明>:=<变量定义>;\n");
            getnext();
        }else{
            error(SEMI_ERROR, lineIndex);
            exit(0);
        }
    }
    printf("This is a variable declaration!\n");
}

void _const_dec(){
    while(token.nameid==CONST){
        getnext();
        printf("<常量声明>:=<常量定义>\n");
        _const_def();
        if(token.nameid != SEMIC){
            error(SEMI_ERROR, lineIndex);
            exit(0);
        }
        else{
            getnext();
        }
    }
    printf("This is a constant declaration\n");
}

void _val_func_def()
{
    _dec_head();
    if(token.nameid==LPAREN){
        getnext();
        _para_list();
        if(token.nameid==RPAREN){
            getnext();
            if(token.nameid==LBPAREN){
                getnext();
                _comp_state();
                if(token.nameid == RBPAREN){
                    getnext();
                    printf("This is a value function defination!\n");
                }
                else{
                    error(RBRAS_ERROR, lineIndex);
                    exit(0);
                }
            }else{
                error(LBRAS_ERROR, lineIndex);
                exit(0);
            }
        }else{
            error(RPAR_ERROR, lineIndex);
            exit(0);
        }
    }else{
        error(LPAR_ERROR, lineIndex);
        exit(0);
    }

}

void _void_func_def()
{
    getnext();
    if(token.nameid == ID){
        strcpy(op, "funct");
        strcpy(name, token.id.c_str());
        genMidcode(op, SPACE, SPACE, name);
        address = 0;
        insert_symbol(name, TYPE_FUNC, VALUE_FUNC_VOID, 0, -1);
        getnext();
        if(token.nameid==LPAREN){
            getnext();
            _para_list();
            if(token.nameid==RPAREN){
                getnext();
                if(token.nameid==LBPAREN){
                    getnext();
                    _comp_state();
                    if(token.nameid == RBPAREN){
                        getnext();
                        printf("This is a void function defination!\n");
                    }else{
                        error(RBRAS_ERROR, lineIndex);
                        exit(0);
                    }
                }else{
                    error(LBRAS_ERROR, lineIndex);
                    exit(0);
                }
            }else{
                error(RPAR_ERROR, lineIndex);
                exit(0);
            }
        }else{
            error(RPAR_ERROR, lineIndex);
            exit(0);
        }
    }else{
        error(ID_ERROR, lineIndex);
        exit(0);
    }
}

void _main_func()
{
    getnext();
    if(token.nameid==MAIN){
        strcpy(name, token.id.c_str());
        strcpy(op, "MAIN");
        genMidcode(op, SPACE, SPACE, name);
        address = 0;
        insert_symbol(name, TYPE_FUNC, VALUE_FUNC_VOID, 0, 0);
        getnext();
        if(token.nameid==LPAREN){
            getnext();
            if(token.nameid==RPAREN){
                getnext();
                if(token.nameid==LBPAREN){
                    getnext();
                    _comp_state();
                    getnext();
                    if(token.nameid == RBPAREN){
                        printf("This is a main function defination!\n");
                    }else{
                        error(RBRAS_ERROR, lineIndex);
                        exit(0);
                    }
                }else{
                    error(LBRAS_ERROR, lineIndex);
                    exit(0);
                }
            }else{
                error(RPAR_ERROR, lineIndex);
                exit(0);
            }
        }else{
            error(RPAR_ERROR, lineIndex);
            exit(0);
        }
    }else{
        error(MAINSY_ERROR, lineIndex);
        exit(0);
    }
}

void _const_def()
{
    if(token.nameid==INT){
        do{
            getnext();
            if(token.nameid != ID){
                error(ID_ERROR, lineIndex);
                exit(0);
            }
            strcpy(name, token.id.c_str());
            getnext();
            if(token.nameid != EQUAL){
                error(CONST_DEF_ASSIGN_ERROR, lineIndex);
                exit(0);
            }
            getnext();
            int flag = 0;
            if(token.nameid == MINUS){
                flag=1;
                getnext();
            }
            if(token.nameid!=NUMBER){
                error(CONST_DEF_TYPE_ERROR, lineIndex);
                exit(0);
            }
            if(flag){
                value = 0-token.value;
            }else{
                value = token.value;
            }
            strcpy(op, "const");
            strcpy(arg1, "int");
            sprintf(arg2, "%d", value);
            genMidcode(op, arg1, arg2, name);
            insert_symbol(name, TYPE_CONST, value, ++address, -1);
            getnext();
        }while(token.nameid == COMMA);       
    }
    else if(token.nameid == CHAR)
    {
        do{
            getnext();
            if(token.nameid != ID){
                error(ID_ERROR, lineIndex);
                exit(0);
            }
            strcpy(name, token.id.c_str());
            getnext();
            if(token.nameid != EQUAL){
                error(CONST_DEF_ASSIGN_ERROR, lineIndex);
                exit(0);
            }
            getnext();
            if(token.nameid != CHARSYM){
                error(CONST_DEF_ASSIGN_ERROR, lineIndex);
                exit(0);
            }
            strcpy(op, "const");
            strcpy(arg1, "char");
            sprintf(arg2, "%d", value);
            genMidcode(op, arg1, arg2, name);
            insert_symbol(name, TYPE_CONST, token.value, ++address, -1);
            getnext();
        }while(token.nameid == COMMA);
    }
    else
    {
        error(CONST_DEC_TYPE_ERROR, lineIndex);
        exit(0);
    }    
}

void _var_def()
{
    do{
        getnext();
        if(token.nameid != ID){
            error(ID_ERROR, lineIndex);
            exit(0);
        }
        strcpy(name, token.id.c_str());
        getnext();
        if(token.nameid == LMPAREN){
            getnext();
            if(token.nameid != NUMBER){
                error(VAR_DEF_ARRAYINDEX_ERROR, lineIndex);
                exit(0);
            }
            arrsize=token.value;
            address+=arrsize;
            getnext();
            if(token.nameid != RMPAREN){
                error(RBRA_ERROR, lineIndex);
                exit(0);
            }
            strcpy(arg1, "array");
            sprintf(arg2, "%d", arrsize);
            genMidcode(op, arg1, arg2, name);
            insert_symbol(name, TYPE_ARRAY, -1, address, arrsize);
            getnext();
        }else{
            genMidcode(op, SPACE, SPACE, name);
            insert_symbol(name, TYPE_VAR, VALUE_VAR, ++address, -1);
        }

    }while(token.nameid == COMMA);
}

void _dec_head()
{
    if(token.nameid == INT || token.nameid == CHAR){
        strcpy(arg1, token.id.c_str());
        getnext();
        if(token.nameid == ID){
            strcpy(name, token.id.c_str());
            strcpy(result, token.id.c_str());
            strcpy(op,"func");
            genMidcode(op, arg1, SPACE,result);
            address = 0;
            insert_symbol(name, TYPE_FUNC, value, 0, -1);
            printf("This is a defination head!\n");
            getnext();        
        }else{
            error(ID_ERROR, lineIndex);
            exit(0); 
        }
    }
}

void _para_list()
{
    do{
        if(token.nameid == COMMA){
            getnext();
        }
        if(token.nameid == INT || token.nameid == CHAR){
            strcpy(arg1, token.id.c_str());
            getnext();
            strcpy(name, token.id.c_str());
            strcpy(op, "para");
            genMidcode(op, arg1, SPACE, name);
            insert_symbol(name, TYPE_PARA, -1, ++address, -1);
            paranum ++;
            if(token.nameid != ID){
                error(ID_ERROR, lineIndex);
                exit(0);
            }
            getnext();
        }
    }while(token.nameid == COMMA);
    insert_para(paranum);
}

void _comp_state()
{
    if(token.nameid == CONST){
        _const_dec();
    }
    if(token.nameid == INT || token.nameid ==CHAR){
        _var_dec();
    }
    _state_list();
    printf("This is a composite statement!\n");
}

void _state_list()
{
    while(token.nameid==IF || token.nameid==WHILE || token.nameid==LBPAREN || token.nameid==ID || token.nameid==SCANF || token.nameid==PRINTF || token.nameid==SEMIC || token.nameid==SWITCH || token.nameid == RETURN){
        _state();
    }
    printf("This is a statement list!\n");
}

void _state()
{
    if(token.nameid==IF){
        _if_state();
    }else if(token.nameid==WHILE){
        _loop_state();
    }else if(token.nameid==LBPAREN){
        getnext();
        _state_list();
        if(token.nameid==RBPAREN){
            getnext();
            cout<<"语句列"<<endl;
        }else{
            error(RBRAS_ERROR, lineIndex);
            exit(0);
        }
    }else if(token.nameid==ID){
        strcpy(arg1, token.id.c_str());
        tempstack.push(token.id);
        save_scene(); // 保存现场，查看下一个字符
        getnext();
        if(token.nameid == LPAREN){ //检测到小括号，函数调用
            restore_scene();
            //TODO
            // 辨别是否是有返回值函数的调用语句
            _val_fun_call();


            if(token.nameid==SEMIC){
                printf("函数调用\n");
                getnext();
            }else{
                error(SEMI_ERROR, lineIndex);
                exit(0);
            }
        }else if(token.nameid == EQUAL || token.nameid == LMPAREN){
            restore_scene();
            _assign_state();
            if(token.nameid==SEMIC){
                getnext();
            }else{
                error(SEMI_ERROR, lineIndex);
                exit(0);
            }
        }else{
            error(STATE_AFTER_ID_ERROR, lineIndex);
            exit(0);
        }
    }else if(token.nameid == SCANF){
        _read_state();
        if(token.nameid==SEMIC){
            getnext();
        }else{
            error(SEMI_ERROR, lineIndex);
            exit(0);
        }
    }else if(token.nameid == PRINTF){
        _write_state();
        if(token.nameid == SEMIC){
            getnext();
        }else{
            error(SEMI_ERROR, lineIndex);
            exit(0);
        }
    }else if(token.nameid == SWITCH){
        _switch_state();
    }else if(token.nameid == RETURN){
        _return_state();
        if(token.nameid == SEMIC){
            printf("Return结束\n");
            getnext();
        }else{
            error(SEMI_ERROR, lineIndex);
            exit(0);
        }
    }else if(token.nameid == SEMIC){
        getnext();
    }else{
        return;
    }
}

void _if_state()
{
    char label1[128];
    char label2[128];
    strcpy(label1, genLab());
    strcpy(label2, genLab());
    getnext();
    if(token.nameid == LPAREN){
        getnext();
        _condition();
        strcpy(op, "jne");
        genMidcode(op, SPACE, SPACE, label1);
        if(token.nameid == RPAREN){
            getnext();
            _state();
            strcpy(op, "jmp");
            genMidcode(op, SPACE, SPACE, label2);
            strcpy(op, "lab");
            genMidcode(op, SPACE, SPACE, label1);
            if(token.nameid == ELSE){
                getnext();
                _state();
            }
            strcpy(op, "lab");
            genMidcode(op, SPACE, SPACE, label2);
        }else{
            error(RPAR_ERROR, lineIndex);
            exit(0);
        }
    }else{
        error(LPAR_ERROR, lineIndex);
        exit(0);
    }
}

void _condition()//条件
{
    _expr();
    printf("This is a condition!\n");
    if(token.nameid==DOUEQUL ||token.nameid==BIGTH || token.nameid==SMALLTH || token.nameid==NOTBIG || token.nameid==NOTEQUL || token.nameid==NOTSAMLL){
        strcpy(op, token.id.c_str());
        getnext();
        _expr();
        strcpy(arg2, tempstack.top().c_str());
        tempstack.pop();
        strcpy(arg1, tempstack.top().c_str());
        tempstack.pop();
        genMidcode(op, arg1, arg2, SPACE);
    }else{
        strcpy(op, "!=");
        strcpy(arg2, "0");
        strcpy(arg1, tempstack.top().c_str());
        tempstack.pop();
        genMidcode(op, arg1, arg2, SPACE);
        return;
    }
}

void _loop_state()
{
    char label1[16];
    char label2[16];
    strcpy(label1, genLab());
    strcpy(label2, genLab());
    getnext();
    if(token.nameid == LPAREN){
        getnext();
        strcpy(op, "lab");
        genMidcode(op, SPACE, SPACE, label1);
        _condition();
        strcpy(op, "jne");
        genMidcode(op, SPACE, SPACE, label2);
        if(token.nameid == RPAREN){
            getnext();
            _state();
            strcpy(op, "jmp");
            genMidcode(op, SPACE, SPACE, label1);
            strcpy(op, "lab");
            genMidcode(op, SPACE, SPACE, label2);
        }else{
            error(RPAR_ERROR, lineIndex);
            exit(0);
        }
    }else{
        error(LPAREN, lineIndex);
        exit(0);
    }
}

void _switch_state()
{

    getnext();
    if(token.nameid == LPAREN){
        getnext();
        _expr();
        strcpy(switchvalue, tempstack.top().c_str());
        tempstack.pop();
        if(token.nameid == RPAREN){
            getnext();
            if(token.nameid == LBPAREN){
                getnext();
                strcpy(lastlab, genLab());
                _situation_list();
                genMidcode("lab", SPACE, SPACE, lastlab);

                if(token.nameid == RBPAREN){
                    getnext();
                }else{
                    error(RBRAS_ERROR, lineIndex);
                    exit(0);
                }
            }else{
                error(LBRAS_ERROR, lineIndex);
                exit(0);
            }
        }else{
            error(RPAR_ERROR, lineIndex);
            exit(0);
        }
    }else{
        error(LPAR_ERROR, lineIndex);
        exit(0);
    }
}

void _situation_list()
{
    do{
        strcpy(switchlabel, genLab());
        _case_state();
        genMidcode("lab", SPACE, SPACE, switchlabel);
    }while(token.nameid == CASE);
}

void _case_state()
{

    if(token.nameid == CASE){
        getnext();
        if(token.nameid == NUMBER || token.nameid == CHARSYM){
            sprintf(arg2, "%d", token.value);
            genMidcode("==", switchvalue, arg2, SPACE);
            genMidcode("jne",SPACE,SPACE,switchlabel);

            getnext();
            if(token.nameid == COLON){
                getnext();
                _state();
                genMidcode("jmp", SPACE, SPACE, lastlab);
            }else{
                error(COLON_ERROR, lineIndex);
                exit(0);
            }
        }else{
            error(CASE_CONST_ERROR, lineIndex);
            exit(0);
        }
    }else{
        error(CASE_ERROR, lineIndex);
        exit(0);
    }
}

void _val_fun_call()
{
    getnext();
    if(token.nameid == LPAREN){
        getnext();
        _val_para_list();
        strcpy(op, "call");
        strcpy(arg1, tempstack.top().c_str());
        tempstack.pop();
        tempstack.push(genVar());
        strcpy(result, tempstack.top().c_str());
        insert_symbol(result, TYPE_VAR, VALUE_VAR, ++address, -1);//向符号表中插入中间的临时变量
        genMidcode(op, arg1, SPACE, result);
        if(token.nameid == RPAREN){
            getnext();
            printf("有返回值函数调用\n");
        }else{
            error(RPAR_ERROR, lineIndex);
            exit(0);
        }
    }else{
        error(LPAR_ERROR, lineIndex);
        exit(0);
    }
}

void _void_fun_call()
{
    getnext();
    if(token.nameid == LPAREN){
        getnext();
        _val_para_list();
        strcpy(op, "call");
        strcpy(result, "no");
        strcpy(arg1, tempstack.top().c_str());
        tempstack.pop();
        genMidcode(op, arg1, SPACE, result);
        if(token.nameid == RPAREN){
            getnext();
            printf("无返回值函数调用\n");
        }else{
            error(RBRA_ERROR, lineIndex);
            exit(0);
        }
    }else{
        error(LPAR_ERROR, lineIndex);
        exit(0);
    }
}

void _val_para_list()
{
    if(token.nameid==RPAREN){
        printf("<值参数表>:=<空>\n");
    }else {
        do {
            if (token.nameid == COMMA) {
                getnext();
            }
            _expr();
            strcpy(op, "cpara");
            strcpy(result, tempstack.top().c_str());
            tempstack.pop();
            genMidcode(op, SPACE, SPACE, result);
        } while (token.nameid == COMMA);
        printf("值参数表\n");
    }
}

void _assign_state()
{
    getnext();
    if(token.nameid == EQUAL){
        getnext();
        _expr();
        strcpy(arg1, tempstack.top().c_str());
        tempstack.pop();
        strcpy(result, tempstack.top().c_str());
        genMidcode("=", arg1, SPACE, result);
        printf("普通变量赋值\n");
    }else if(token.nameid == LMPAREN){
        getnext();
        _expr();
        if(token.nameid == RMPAREN){
            getnext();
            if(token.nameid == EQUAL){
                getnext();
                _expr();
                strcpy(arg1, tempstack.top().c_str());
                tempstack.pop();
                strcpy(arg2, tempstack.top().c_str());
                tempstack.pop();
                strcpy(result, tempstack.top().c_str());
                tempstack.pop();
                genMidcode("arrayl", arg1, arg2, result);
            }else{
                error(ASSIGN_ERROR, lineIndex);
                exit(0);
            }
        }else{
            error(RBRAS_ERROR, lineIndex);
            exit(0);
        }
        printf("数组变量赋值\n");
    }else{
        error(ASSIGN_ERROR, lineIndex);
        exit(0);
    }
}

void _read_state()
{
    getnext();
    if(token.nameid == LPAREN){
        do{
            getnext();
            if(token.nameid == ID){
                strcpy(result, token.id.c_str());
                strcpy(op, "scanf");
                genMidcode(op, SPACE, SPACE, result);
                getnext();
            }else{
                error(ID_ERROR, lineIndex);
                exit(0);
            }
        }while(token.nameid == COMMA);
        if(token.nameid == RPAREN){
            getnext();
            printf("读语句\n");
        }else{
            error(RPAR_ERROR, lineIndex);
            exit(0);
        }
    }else{
        error(LPAR_ERROR, lineIndex);
        exit(0);
    }
}

void _write_state()//TODO
{
    getnext();
    if(token.nameid == LPAREN){
        getnext();
        if(token.nameid == STRING){
            tempstack.push(token.id);
            getnext();
            if(token.nameid == COMMA){
                getnext();
                _expr();
                if(token.nameid == RPAREN){
                    strcpy(arg2, tempstack.top().c_str());
                    tempstack.pop();
                    strcpy(arg1, tempstack.top().c_str());
                    tempstack.pop();
                    genMidcode("prtf", arg1, arg2, "sym");
                    printf("printf(字符串，表达式)\n");
                    getnext();
                }else{
                    error(RPAR_ERROR, lineIndex);
                    exit(0);
                }
            }else if(token.nameid == RPAREN){
                getnext();
                strcpy(arg1, tempstack.top().c_str());
                tempstack.pop();
                //cout<<tempstack.top()<<endl;
                genMidcode("prtf", arg1, SPACE, "string");
                printf("printf(字符串)\n");
            }else{
                error(RPAR_ERROR, lineIndex);
                exit(0);
            }
        }else{
            _expr();
            if(token.nameid == RPAREN){
                getnext();
                strcpy(arg2, tempstack.top().c_str());
                tempstack.pop();
                genMidcode("prtf", SPACE, arg2, "st");
                printf("printf(表达式)\n");
            }else{
                error(RPAR_ERROR, lineIndex);
                exit(0);
            }
        }
    }else{
        error(LPAR_ERROR, lineIndex);
        exit(0);
    }
}

void _return_state()
{
    getnext();

    if(token.nameid == LPAREN){
        getnext();
        _expr();
        if(token.nameid == RPAREN){
            getnext();
            strcpy(op, "reta");
            strcpy(result, tempstack.top().c_str());
            tempstack.pop();
            genMidcode(op, SPACE, SPACE, result);
            printf("return <表达式>\n");
        }else{
            error(RPAR_ERROR, lineIndex);
            exit(0);
        }
    }else if(token.nameid == SEMIC){
        printf("return\n");
        strcpy(op, "ret");
        genMidcode(op, SPACE, SPACE, SPACE);
    }else{
        printf("RETURN ERROR\n");
        exit(0);
    }
}

void _expr()
{
    char tempop[10];
    if(token.nameid == PLUS || token.nameid == MINUS){
        tempstack.push("0");
        strcpy(tempop, token.id.c_str());
        getnext();
        _term();
        strcpy(arg2, tempstack.top().c_str());
        tempstack.pop();
        strcpy(arg1, tempstack.top().c_str());
        tempstack.pop();
        strcpy(result, genVar());
        tempstack.push(result);
        insert_symbol(result, TYPE_VAR, VALUE_VAR, ++address, -1);
        genMidcode(tempop, arg1, arg2, result);
        while(token.nameid == PLUS || token.nameid == MINUS){
            strcpy(tempop, token.id.c_str());
            getnext();
            _term();
            strcpy(arg2, tempstack.top().c_str());
            tempstack.pop();
            strcpy(arg1, tempstack.top().c_str());
            tempstack.pop();
            strcpy(result, genVar());
            tempstack.push(result);
            insert_symbol(result, TYPE_VAR, VALUE_VAR, ++address, -1);
            genMidcode(tempop, arg1, arg2, result);
        }
    }else{
        _term();
        while(token.nameid == PLUS || token.nameid == MINUS){
            strcpy(tempop, token.id.c_str());
            getnext();
            _term();
            strcpy(arg2, tempstack.top().c_str());
            tempstack.pop();
            strcpy(arg1, tempstack.top().c_str());
            tempstack.pop();
            strcpy(result, genVar());
            tempstack.push(result);
            insert_symbol(result, TYPE_VAR, VALUE_VAR, ++address, -1);
            genMidcode(tempop, arg1, arg2, result);
        }
    }
}

void _term()
{
    _factor();
    while(token.nameid == MULT || token.nameid == DIV){
        char tempop[10];
        strcpy(tempop, token.id.c_str());
        getnext();
        _factor();
        strcpy(arg2, tempstack.top().c_str());
        tempstack.pop();
        strcpy(arg1, tempstack.top().c_str());
        tempstack.pop();
        strcpy(result, genVar());
        tempstack.push(result);
        insert_symbol(result, TYPE_VAR, VALUE_VAR, ++address, -1);
        genMidcode(tempop, arg1, arg2, result);
    }
}

void _factor()
{
    if(token.nameid == LPAREN){
        getnext();
        _expr();
        if(token.nameid == RPAREN){
            getnext();
            printf("<因子>:=<表达式>\n");
        }else{
            error(RPAR_ERROR, lineIndex);
            exit(0);
        }
    }else if(token.nameid == NUMBER){
        printf("<因子>:=<整数>\n");
        sprintf(result, "%d", token.value);
        tempstack.push(result);
        getnext();

    }else if(token.nameid == CHARSYM){
        printf("<因子>:=<字符>\n");
        tempstack.push(token.id);
        getnext();
    }else if(token.nameid == ID){
        save_scene();
        tempstack.push(token.id);
        //cout<<token.id<<endl;
        getnext();
        if(token.nameid == LPAREN){
            restore_scene();
            _val_fun_call();
            printf("<因子>:=<有返回值调用语句>\n");
        }else if (token.nameid == LMPAREN){
            getnext();
            _expr();
            strcpy(arg1, tempstack.top().c_str());
            tempstack.pop();
            strcpy(arg2, tempstack.top().c_str());
            tempstack.pop();
            tempstack.push(genVar());
            strcpy(result, tempstack.top().c_str());
            insert_symbol(result, TYPE_VAR, VALUE_VAR, ++address, -1);
            genMidcode("array", arg1, arg2, result);
            if(token.nameid == RMPAREN){
                printf("<因子>:=<标识符><表达式>\n");
                getnext();
            }else{
                error(RBRA_ERROR, lineIndex);
                exit(0);
            }
        }else{
            printf("<因子>:=<标识符>\n");
            //cout<<tempstack.top()<<endl;
            //cout<<token.id<<endl;
            //tempstack.push(token.id);
            //getnext();
        }
    }else{
        printf("因子分析错误\n");
        exit(0);
    }
}