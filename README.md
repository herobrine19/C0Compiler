# C0文法的小型编译器实现

[TOC]

## 一、C0文法

符号含义

| EBNF(扩展巴科斯范式)元符号 | 含义                  |
| -------------------------- | --------------------- |
| ::=                        | 定义为                |
| \|                         | 或                    |
| { }                        | 含0次在内任意多次重复 |
| [ ]                        | 含0次和1次            |
| < >                        | 非终结符              |
| ( )                        | 括号内看作一项        |

具体文法

| 非终结符               | 右部                                                         |
| ---------------------- | ------------------------------------------------------------ |
| <加法运算符>           | + \| -                                                       |
| <乘法运算符>           | * \| /                                                       |
| <关系运算符>           | < \| <= \| > \| >= \| != \| ==                               |
| <字母>                 | _\|a\|...\|z\|A\|...\|Z                                      |
| <数字>                 | 0\|<非零数字>                                                |
| <非零数字>             | 1\|...\|9                                                    |
| <字符>                 | '<加法运算符>'\|'<乘法运算符>'\|'<字母>'\|'<数字>'           |
| <字符串>               | "{十进制编码为32，33，35-126的ASCII字符}"                    |
| <程序>                 | [<常量说明>] [<变量说明>] {<有返回值函数定义>\|<无返回值函数定义>}<主函数> |
| <常量说明>             | const <常量定义> ; {const <常量定义> ;}                      |
| <常量定义>             | int <标识符> = <整数> {,int <标识符> = <整数>} \|char <标识符> = <字符> {,char <标识符> = <字符>} |
| <无符号整数>           | <非零数字> {<数字>}                                          |
| <整数>                 | [ +\|- ] <无符号整数> \| 0                                   |
| <标识符>               | <字母> {<字母> \| <数字>}                                    |
| <声明头部>             | int <标识符> \| char <标识符>                                |
| <变量说明>             | <变量定义> ; {<变量定义>;}                                   |
| <变量定义>             | <类型标识符> (<标识符>\|<标识符>'['<无符号整数>']'){,(<标识符>\|<标识符>'['<无符号整数>']')}   *举例：int a1,ax[2],ax3,ax[4]* |
| <常量>                 | <整数>\|<字符>                                               |
| <类型标识符>           | int\|char                                                    |
| <有返回值函数定义>     | <声明头部> '(' <参数> ')' '{' <复合语句> '}'  *举例：int function(int a,int b){xxxxx}* |
| <无返回值函数定义>     | void <标识符> '(' <参数> ')' '{' <复合语句> '}'              |
| <复合语句>             | [<常量说明>] [<变量说明>] <语句列>                           |
| <参数>                 | <参数表>                                                     |
| <参数表>               | <类型标识符> <标识符> {, <类型标识符> <标识符>} \| <空>      |
| <主函数>               | void main '(' ')' '{' <复合语句> '}'                         |
| <表达式>               | [ + \| - ] <项> {<加法运算符> <项>} *例：a+b, a\*b+c\*d, a+(b+c)* |
| <项>                   | <因子> {<乘法运算符><因子>} *例：a\*b, a, (a+b)\*c, a/c*     |
| <因子>                 | <标识符> \| <标识符> '[' <表达式> ']' \| <整数> \| <字符> \| <有返回值函数调用语句> \| '(' <表达式> ')' *例：(a+b), call(a)* |
| <语句>                 | <条件语句>\|<循环语句>\|'{' <语句列> '}' \| <有返回值函数调用语句>; \| <无返回值函数调用语句>; \| <赋值语句>; \| <读语句>; \| <写语句>; \| <空>; \| <情况语句> \| <返回语句>; |
| <赋值语句>             | <标识符> = <表达式> \| <标识符> '[' <表达式> ']' = <表达式>  |
| <条件语句>             | if '(' <条件> ')' <语句> [else <语句>]                       |
| <条件>                 | <表达式> <关系运算符> <表达式> \| <表达式>                   |
| <循环语句>             | while '(' <条件> ')' <语句>                                  |
| <情况语句>             | switch '(' <表达式> ')' '{' <情况表> '}'do                   |
| <情况表>               | <情况子语句> {<情况子语句>}                                  |
| <情况子语句>           | case <常量> : <语句>                                         |
| <有返回值函数调用语句> | <标识符> '(' <值参数表> ')'                                  |
| <无返回值函数调用语句> | <标识符> '(' <值参数表> ')'                                  |
| <值参数表>             | <表达式> {,<表达式>} \| <空>                                 |
| <语句列>               | {<语句>}                                                     |
| <读语句>               | scanf'(' <标识符> {, <标识符>}')' *例：scanf(a,b,c,d)*       |
| <写语句>               | printf '(' <字符串> , <表达式> ')' \| printf '(' <字符串> ')' \| printf '(' <表达式> ')   *例：printf("abc", 1+3), printf("abc"), printf(a+1)* |
| <返回语句>             | return ['(' <表达式> ')'] *例：return , return a+b, return 0* |



## 二、词法分析

### 1. 保留字表：

| 保留字 | 宏名   | 宏体 |
| ------ | ------ | ---- |
| int    | INT    | 1    |
| char   | CHAR   | 2    |
| void   | VOID   | 3    |
| const  | CONST  | 4    |
| main   | MAIN   | 5    |
| if     | IF     | 6    |
| else   | ELSE   | 7    |
| while  | WHILE  | 8    |
| switch | SWITCH | 9    |
| case   | CASE   | 10   |
| scanf  | SCANF  | 11   |
| printf | PRINTF | 12   |
| return | RETURN | 13   |

### 2. 特殊符号表：

| 特殊符号 | 宏名     | 宏体 |
| -------- | -------- | ---- |
| 标识符   | ID       | 14   |
| >        | BIGTH    | 15   |
| <        | SMALLTH  | 16   |
| \>=      | NOTSMALL | 17   |
| <=       | NOTBIG   | 18   |
| !=       | NOTEQUL  | 19   |
| ==       | DOUEQUL  | 20   |
| =        | EQUAL    | 21   |
| +        | PLUS     | 22   |
| -        | MINUS    | 23   |
| *        | MULT     | 24   |
| /        | DIV      | 25   |
| ,        | COMMA    | 26   |
| {        | LBPAREN  | 27   |
| }        | RBPAREN  | 28   |
| [        | LMPAREN  | 29   |
| ]        | RMPAREN  | 30   |
| (        | LPAREN   | 31   |
| )        | RPAREN   | 32   |
| :        | COLON    | 33   |
| ;        | SEMIC    | 34   |
| 字符串   | STRING   | 35   |
| 整数     | NUMBER   | 36   |
| 单个字符 | CHARSYM  | 37   |
| '        | SQUOTE   | 38   |
| "        | DQUOTE   | 39   |

### 3. token的定义

```cpp
typedef struct 
{
    string name;    // token名
    int nameid;     // token的id
    string id;      // 文件中的单词
    int value;      // 值
} Token;
```

### 4. 词法分析自动机

![](https://tva1.sinaimg.cn/large/007S8ZIlgy1gf70w7it2lj31180u0q4a.jpg)

### 5. 词法分析具体实现过程

1. 将源代码文件读取到一个字符串里
2. 按顺序读取每一个字符
3. 如果是\n(换行)、\r(回车)、\t(水平制表符)、\v(垂直制表符)、空格，就跳过这个字符，如果是换行符，要把行号加一，为后面的错误处理提供定位。
4. 如果这个字符是数字，继续向后读取，同时把这个字符串向int做转换，直到读到其他字符为止，同时要检查这个字符串是不是0或者有没有超过int范围。
5. 如果这个字符是字母，继续向后读取，把整个串读完，看一下是不是语言保留字，如果不是就当成标识符。
6. 如果是>或者<或者=，看一下后面一个字符，如果是=，就是>=或者<=或者==，否则是>或者<或者=。
7. 如果是!，看一下后面一个字符，如果是=，就是!=，否则就报错。
8. 如果是‘，要检测是否符合字符的文法定义。
9. 如果是“，要检测是否符合字符串的文法定义。
10. 剩下的都是特殊字符的识别与检测。



## 三、语法分析

### 1. 递归下降程序

整个语法分析过程就采用的是递归下降的方法，对着文法一条一条实现的。

```c++
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
void _loop_state();      //循环语句
void _switch_state();    //情况语句
void _situation_list();  //情况表
void _case_state();      //情况子语句
void _val_fun_call();    //有返回值函数调用语句1
void _val_para_list();   //值参数表
void _assign_state();    //赋值语句
void _read_state();      //读语句
void _write_state();     //写语句
void _return_state();    //返回语句
void _expr();            //表达式
void _term();            //项
void _factor();          //因子
```

整个过程就是一条一条读入token，解析是哪个文法，有部分文法需要向前多读一个token才能确定，所以有时候需要保存现场向前再读一个token，如果确定了是什么语句就恢复现场，进入那个语句的解析。

### 2. 符号表设计

​		在进行语法分析，语法错误检查时，还需要检查当前使用的标识符是否已经定义，同时要将定义的符号的值，名字，偏移地址等属性记录下来，供后面语义分析使用。同时在后面翻译为汇编代码时候需要查找标识符相对于函数栈基址的位置，因此还需要记录标识符的相对偏移信息。

#### 2.1 符号表的定义

```c++
//符号表中type字段
#define TYPE_CONST_INT  0
#define TYPE_CONST_CHAR 1
#define TYPE_FUNC   2
#define TYPE_PARA   3
#define TYPE_ARRAY  4
#define TYPE_STRING 5
#define TYPE_VAR    6

//符号表中 value字段
#define VALUE_VAR        -1
#define VALUE_FUNC_VOID   0
#define VALUE_FUNC_INT    1
#define VALUE_FUNC_CHAR   2
#define VALUE_VAR_INT     1
#define VALUE_VAR_CHAR    2

typedef struct
{
    char name[512]; //标识符的名字
    int type;       //0-int常量 1-char常量 2-函数 3-参数 4-数组 5-string 6-变量
    int value;      //常量的值，若type是函数，则0为void，1为int，2为char，若type是变量，1为int，2为char
    int address;    //标识符相对于函数基址的偏移量
    int para;       //函数参数个数或者数组大小
} Symbol;

/**
 * 这个表相当于是索引表，因为存在局部变量和全局变量的作用域问题，
 * 所以要记录每一个函数的作用域范围，其实只用记录每一个函数的首地
 * 址即可，因为一个函数的作用域范围就是在这个函数的首地址到下一个
 * 函数的首地址之间，全局变量放在符号表的最顶层。
 */
typedef struct
{
    Symbol elements[MAX_SYMBOL_NUM];    //符号表
    int top;                            //符号表栈顶指针
    int funcTotal;                      //当前符号表拥有的函数总数
    int funcIndex[MAX_FUNC_NUM];        //函数在表中的索引
} SymbolTable;
```

#### 2.2 符号表的插入与查找

1. 符号表的插入其实很简单：
   * 每次遇到一个函数定义时，向表的top处插入函数名，将其address属性设置为0；
   * 遇到一个新的变量定义时，插到表的top处，address属性+1；
   * 如果是数组的定义，插到表的top处，address属性加上数组的大小；
2. 每次往符号表里插入元素时都要检查一下这个新的元素是否出现过，如果有的话就是重复定义，没有的话才可以插入，检查待插入元素是否出现过需要查找一下符号表，先在当前函数内部找，如果没有的话就要去全局变量找，都没有的话就是没有定义过。





## 四、中间代码生成

四元式设计：

| 原始语句                 | op     | a        | b    | result   |
| ------------------------ | ------ | -------- | ---- | -------- |
| const int a = 98         | const  | int      | 98   | a        |
| const char b = 'a'       | const  | char     | 97   | b        |
| void sun()               | funct  |          |      | sun      |
| int fun()                | func   | int      |      | fun      |
| fun(int n)               | para   | int      |      | n        |
| n == a                   | ==     | n        | a    |          |
| n != a                   | !=     | n        | a    |          |
| n >= a                   | \>=    | n        | a    |          |
| n <= a                   | <=     | n        | a    |          |
| n > a                    | >      | n        | a    |          |
| n < a                    | <      | n        | a    |          |
| if(n == 1){ ...... }     | ==     | n        | 1    |          |
|                          | jne    |          |      | _LABEL_0 |
|                          | ...... |          |      |          |
|                          | jmp    |          |      | _LABEL_1 |
|                          | lab    |          |      | _LABEL_0 |
|                          | lab    |          |      | _LABEL_1 |
| return(1);               | reta   |          |      | 1        |
| return ;                 | ret    |          |      |          |
| fun(n-2)                 | -      | n        | 2    | _ld0     |
|                          | cpara  |          |      | _ld1     |
|                          | call   | fun      |      | _ld2     |
| printf("hello world\n"); | prtf   | string0  |      | string   |
| printf(a);               | prtf   |          | a    | int/char |
| print("aaa",a);          | prtf   | string1  | a    | int/char |
| void main()              | MAIN   |          |      | main     |
| int temp;                | int    |          |      | temp     |
| int num[5];              | int    | array    | 5    | num      |
| scanf(a);                | scanf  | int/char |      | a        |
| i = 0                    | =      | 0        |      | i        |
| num[i]=a;                | arrayl | a        | i    | num      |
| temp=num[j]              | array  | num      | j    | _ld3     |
|                          | =      | _ld3     |      | temp     |
| while(i<5){......}       | lab    |          |      | _LABEL_2 |
|                          | i      | <        | 5    |          |
|                          | jne    |          |      | _LABEL_3 |
|                          | ...... |          |      |          |
|                          | jmp    |          |      | _LABEL_2 |
|                          | lab    |          |      | _LABEL_3 |
| 函数末尾                 | end    | func     |      |          |
| switch(a){               |        |          |      |          |
| case 1: ... ;            | ==     | a        | 1    |          |
|                          | jne    |          |      | _LABEL_5 |
|                          | ...... |          |      |          |
|                          | jmp    |          |      | _LABEL_4 |
|                          | lab    |          |      | _LABEL_5 |
| case 2: ... ;            | ==     | a        | 2    |          |
|                          | jne    |          |      | _LABEL_6 |
|                          | jmp    |          |      | _LABEL_4 |
|                          | ...... |          |      |          |
|                          | lab    |          |      | _LABEL_6 |
| }                        | lab    |          |      | _LABEL_4 |
|                          |        |          |      |          |

在语法分析过程中生成四元式有个比较麻烦的地方就是变量的顺序问题,比如这里的赋值语句：

赋值语句的文法是<赋值语句>：=  <标识符> = <表达式> \| <标识符> '[' <表达式> ']' = <表达式>

因为要读到=号才能确定是赋值语句，并且要读完整条语句才能确定是谁赋值给谁，所以这里需要引入一个**栈**来保存每次解析到的变量，把变量从左到右压栈，所以生成中间代码的时候要从右到左出栈，注意顺序即可。

```c++
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
```





## 五、mips汇编生成

### 1. MIPS编译器寄存器使用约定

| REGISTER | NAME     | USAGE                                                        |
| -------- | -------- | ------------------------------------------------------------ |
| $0       | $zero    | 常量0(constant value 0)                                      |
| $1       | $at      | 保留给汇编器(Reserved for assembler)                         |
| \$2-$3   | \$v0-$v1 | 函数调用返回值(values for results and expression evaluation) |
| \$4-$7   | \$a0-$a3 | 函数调用参数(arguments)                                      |
| \$8-$15  | \$t0-$t7 | 暂时的(或随便用的)                                           |
| \$16-$23 | \$s0-$s7 | 保存的(或如果用，需要SAVE/RESTORE的)(saved)                  |
| \$24-$25 | \$t8-$t9 | 暂时的(或随便用的)                                           |
| $28      | $gp      | 全局指针(Global Pointer)                                     |
| $29      | $sp      | 堆栈指针(Stack Pointer)                                      |
| $30      | $fp      | 帧指针(Frame Pointer)                                        |
| $31      | $ra      | 返回地址(return address)                                     |



### 2. MIPS汇编程序框架

* .data伪指令

  ​		定义程序的数据段，程序的变量需要在该伪指令下定义，汇编程序会分配和初始化变量的存储空间。

* .text

  ​		定义程序的代码段。

* .global

  ​		声明一个符号全局的，全局符号可以被其他的文件引用，用该伪指令声明一个程序的main过程。

  

### 3. 用到的一些指令

1. 寻址类指令

   | 寻址方式 | 指令            | 含义                                                         |
   | -------- | --------------- | ------------------------------------------------------------ |
   | 直接寻址 | `la $t0,var1`   | 把var1在内存中的地址拷贝到寄存器t0。var1可以是标签地址       |
   | 间接寻址 | `lw $t2,($t0)`  | 把t0存放的地址处的字拷贝到t2中                               |
   | 间接寻址 | `sw $t2,($t0)`  | 把t2中的字存储到t0中地址所指向的主存位置                     |
   | 基址寻址 | `lw $t2,4($t0)` | 把t0中地址加4所得的地址所对应的主存中的字载入寄存器t2中，4为偏移量 |

2. 跳转类指令：

   | 指令        | 含义                                                      |
   | ----------- | --------------------------------------------------------- |
   | `jal label` | \$ra=PC+8,jump,在$ra中保存返回地址并跳转到label过程处执行 |
   | `jr $ra`    | 类似相对寻址，跳到该寄存器给出的地址处                    |
   | `j target`  | 绝对跳转指令，跳转到target                                |

3. 分支(if else系列)指令：

   | 指令                 | 含义                       |
   | -------------------- | -------------------------- |
   | `b target`           | 无条件跳转到target处       |
   | `beq $t0,$t1,target` | 若\$t0=\$t1，跳转到target  |
   | `blt $t0,$t1,target` | 若\$t0<\$t1，跳转到target  |
   | `ble $t0,$t1,target` | 若\$t0<=\$t1，跳转到target |
   | `bgt $t0,$t1,target` | 若\$t0>\$t1，跳转到target  |
   | `bge $t0,$t1,target` | 若\$t0>=\$t1，跳转到target |
   | `bne $t0,$t1,target` | 若\$t0!=\$t1，跳转到target |

4. 系统调用：

   * 指令：`syscall`
   * 参数所使用的寄存器：\$v0, $a0
   * 返回值使用：$v0

   下表给出了系统调用中对应功能，代码，参数和返回值

   | 功能           | 调用码   | 参数                     | 返回值             |
   | -------------- | -------- | ------------------------ | ------------------ |
   | 打印一个整型   | `$v0=1`  | $a0=要打印的整型值       |                    |
   | 打印一个字符串 | `$v0=2`  | $a0=要打印的字符串的地址 |                    |
   | 打印一个字符   | `$v0=11` | $a0=要打印的字符         |                    |
   | 读取一个int    | `$v0=5`  |                          | $v0=读取到的整型值 |
   | 读取一个char   | `$v0=12` |                          | $v0=读取到的字符   |

   *这里需要注意一下，对于读取整型的数据操作，系统会读取一整行，也就是说以换行符为结束标志。*

   *还有一点就是，暂时我还没有写字符的输入输出。*

5. 算数指令集：

   *这里要注意，操作数只能是寄存器，不能是地址*

   | 指令               | 含义              |
   | ------------------ | ----------------- |
   | `sub $t2,$t3,$t4`  | `$t2 = $t3 + $t4` |
   | `subu $t1,$t6,$t7` | 无符号减          |
   | `addi $t2,$t3,5`   | 加立即数          |
   | `addu $t1,$t6,$t7` | 无符号加          |
   | ` add $t1,$t6,$t7` | `$t1 = $t6 + $7`  |
   | `mul $t2,$t0,$t1`  | `$t2 = $t0 * $t1` |
   | `div $t2,$t0,$t1`  | `$t2 = $t0 * $t1` |

6. 其他：

   | 指令           | 含义                         |
   | -------------- | ---------------------------- |
   | `li $t0,1`     | 把立即数1存入寄存器t1        |
   | `nop`          | 空指令，占用一个时钟，延时用 |
   | `move $t1,$t2` | `$t1 = $t2`赋值语句          |

### 4.实现过程

1.  整体架构：

    先进行一些初始化的工作，然后根据四元式逐条生成汇编代码：

    ![image-20200707173722290](https://dajinge-1258413278.cos.ap-nanjing.myqcloud.com/blog/2020-07-07-093722.png)

2.  initasm函数主要是为了处理程序中出现的全局变量以及字符串，把这些定义在数据区，保证其可以被所有函数共享和正常输出，除了全局变量和全局常量的处理之外，这里主要做了对输出语句的识别，如果要输出字符串，即将字符串使用stringnum关键字替代四元式中的字符串本身，并将字符串使用.asciiz关键字定义在数据区，方便输出使用系统调用。

    ```c++
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
    ```

3.  使用.word来定义全局常量，就两种类型，char和int

4.  使用.space来为全局变量分配空间，如果是数组的话要分配数组大小乘4的空间

5.  剩下的汇编语句生成都是根据op以及两个操作数，对操作数在符号表中查找偏移量，以便于寻址使用。然后根据op选择合适的指令生成。其中操作数的寻址代码如下：

    ```c++
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
    ```



## 六、优化

### 1. 优化的分类：

* 机器无关优化
  * 针对中间代码
* 机器相关优化
  * 针对目标代码
* 局部代码优化
  * 单个基本块范围内的优化
* 全局代码优化
  * 面向多个基本块的优化

### 2. 常用的优化方法：

* 删除公共子表达式

* 删除无用代码

* 常量合并

  ​		如果在编译时刻推导出一个表达式的值是常量，就可以使用该常量来替代这个表达式。该技术被称为常量合并。

* 代码移动
* 强度削弱
* 删除归纳变量

### 3. 基本块的优化（局部优化）

### 4.我做的优化

由于时间问题我就做了两个小小的优化，一个是常数替换，还有一个是表达式求值。

1.  下面一个是常数替换的代码，把当前函数作用域内已知的一些变量的值直接换成常数，但是在给变量进行重新赋值或者跳出了当前这个作用域的话就不能再替换了。

    ```C++
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
    ```

    

2.  表达式求值的优化和常数替换相似，消除了很多中间变量，只是在替换之前计算一下表达式的值。

    例如对于这段代码：

    ```c++
    int a,b;
    a=1+2*3-2/2;
    b=6;
    ```

    优化之前的四元式是：

    ![image-20200707232546167](https://dajinge-1258413278.cos.ap-nanjing.myqcloud.com/blog/2020-07-07-152546.png)

    而优化后的=四元式是这样的：

    ![image-20200707232629541](https://dajinge-1258413278.cos.ap-nanjing.myqcloud.com/blog/2020-07-07-152630.png)

    中间的加减乘除计算语句全部能在编译的时候完成，大大减少了汇编代码的量。

    


### 七、测试用例

这里写了几个典型的测试用例：

1.  这个用来测试优化的

    ```c++
    void main()
    {
        int a,b;
        a=1+2*3-2/2-6;
        b=6;
        printf(a);
        printf(b);
    }
    ```

    **四元式（那些op为空的四元式是被优化删去的）：**

    ![image-20200714194052393](/Users/jin/Library/Application Support/typora-user-images/image-20200714194052393.png)

    **汇编：**

    ```assembly
    .data
    .text
    .global main
    		j	main
    #MAIN,,,main
    #main function
    main:
    		sw	$fp, ($sp)			#push old frame pointer(dynamic link)
    		move	$fp, $sp			#frame pointer now points to the top of stack
    		subu	$sp, $sp, 500			#allocate say 500 bytes in the stack
    		sw	$ra,-4($fp)			#save return address in frame
    		sw	$v0,-8($fp)			#save static link in frame
    #func fours end
    #int,,,a
    #int,,,b
    #=,0,,a
    		li	$t0,0
    		sw	$t0,-12($fp)
    #=,6,,b
    		li	$t0,6
    		sw	$t0,-16($fp)
    #prtf,,a,int
    		lw	$a0,-12($fp)			#return var
    		li	$v0,1
    		syscall
    		la	$a0, '\n'
    		li	$v0,11
    		syscall
    #prtf,,b,int
    		lw	$a0,-16($fp)			#return var
    		li	$v0,1
    		syscall
    		la	$a0, '\n'
    		li	$v0,11
    		syscall
    #end,func,,
    #function define end
    
    ```

    **运行结果：**可以看到运行结果是正确的

    ![image-20200714194658603](/Users/jin/Library/Application Support/typora-user-images/image-20200714194658603.png)

2.  这个是很经典的递归程序求阶乘：

    ```c++
    int factorial(int x)
    {
    	if(x<=0) return (0);
    	if(x==1) return (1);
    	return (x*factorial(x-1));
    }
    
    void main()
    {
    	int x,result;
    	while(1){
    	    scanf(x);
        	result = factorial(x);
        	printf(result);
    	}
    }
    ```

3.  这个是一个测试数组操作的：

    ```c++
    int change[10];
    void main(){
       change[0+1]=1;
       change[1+2]=6;
       change[2+3]=change[1]+change[3];
       if(change[1]<change[3])
       printf(change[5]);
       else 
       printf("false");
    }
    ```

4.  测试switch和输出字符串：

    ```c++
    void main(){
       int a;
       scanf(a);
       while(a<=10){
       switch(a){
                case 1:printf("a is 1");
                case 2:printf("a is 2");
                case 3:printf("a is 3");
                case 4:printf("a is 4");
                case 5:printf("a is 5");            
                case 6:printf("a is 6");
                case 7:printf("a is 7");
                case 8:printf("a is 8");
                case 9:printf("a is 8");
                case 10:printf("a is 10");
                }
            scanf(a);
         }
    }
    ```

    

## 八、过程中遇到的问题和解决方法

### 1. 词法分析

1. **问题描述**：扫描不是一个字符的token时，比如数字、关键字等，必须要扫描到空格或者其他非显示字符时才能停止，但是这个时候文件指针就已经指向了下一个字符，这时候很有可能跳过行末的换行符，导致记录行号出错。

   **解决方案**：扫描长字符时，扫描结束把文件指针后撤一格，由于我是先把文件读入到一个字符串里，所以模拟的文件指针回撤很方便。

   

2. **问题描述**：对于负数的识别，容易出错，比较麻烦。

   **解决方案**：把正负号和数字分开识别，只识别数字，正负数留在后期语法分析再判断。



### 2. 中间代码生成

1. **问题描述：**一开始没有在四元式里的printf和scanf语句中加上符号的类型，就是没分辨int和char，但是读语句和写语句的汇编代码对不同类型的变量的系统调用不一样，所以不能统一用读入整数或者打印整数来处理。

   **解决方法：**因为一开始符号表也没有记录变量和常量的类型，所以只能对符号表进行一点修改，加上常量和变量的类型字段，在生成四元式的时候判断一下这个变量的类型，就直接在符号表里找就行了，然后写到四元式里，最后汇编代码生成的时候就对着四元式里变量的类型决定系统调用号就行了。


## 九、自我评价

1.  用c++几乎在一个月内纯手工实现了一个C0文法的编译器，代码量接近4000行，在过程中遇到了无数的小bug和很多的问题，在参考前人的代码和网络上的资料下完成。整个过程实属不易。
2.  然而我也是仅仅初步实现了C0编译器，由于时间问题，做的优化很少，这也算是一个比较大的遗憾吧，没能仔细研究优化的模块。
3.  总而言之，这个编译器写下来收获还是很大的，既学习了编译原理的理论知识又锻炼了代码能力和修bug的能力。




