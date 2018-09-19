#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int BOOL;
#define TRUE 1
#define FALSE 0

#define ERROR_MSG_SIZE 1024        //定义错误消息的最大长度
#define ERROR_IVALID_SOURCE_FILE 0 //源码文件错误
#define GERROR_INVALID_TOKEN 1     // token错误

#define ERROR_UNEXPECTED_END_OF_FILE 2          //源码非正常结束
#define ERROR_UNEXPECTED_INPUT 3                //错误的输入
#define ERROR_ILLEGAL_ASSIGN_OP 4               //非法的赋值运算符
#define ERROR_UNEXPECTED_END_OF_LINE 5          //语句未结束
#define ERROR_NESTED_FUNCTION_ILLEGAL 6         //错误的函数嵌套
#define ERROR_UNEXPECTED_BLOCK 7                //错误的语句块
#define GERROR_UNEXPECTED_RETURN 8              //错误的return语句
#define GERROR_UNDEFINED_FUNCTION_CALL 9        //调用未定义的函数
#define GERROR_MORE_PARAM 10                    //太多参数
#define GERROR_FEW_PARAM 11                     //太少参数
#define GERROR_INVALID_INPUT 12                 //错误的输入
#define GERROR_UNDEFINED_VAR 13                 //未定义的变量
#define GERROR_UNEXPECTED_BREAK 14              //意外的break语句
#define GERROR_INVALID_LEFT_HAND_SIDE_ASSGIN 15 //给函数返回值赋值

#define MAX_LEXEME_SIZE 1024
#define MAX_DELIM_COUNT 8
#define MAX_OP_LENGTH 33
#define MAX_IDENT_SIZE 256

#define LEX_STATE_START 0
#define LEX_STATE_INT 1
#define LEX_STATE_FLOAT 2
#define LEX_STATE_DOT 3
#define LEX_STATE_IDENT 5
#define LEX_STATE_OP 6
#define LEX_STATE_DELIM 7
#define LEX_STATE_STRING 8
#define LEX_STATE_STRING_ESCAPE 9
#define LEX_STATE_STRING_CLOSE_QUOTE 10
#define LEX_STATE_UNKNOW 11
#define LEX_STATE_NEWLINE 12

#define TOKEN_TYPE_INVALID 0
#define TOKEN_TYPE_END_STREAM 1
#define TOKEN_TYPE_INT 2
#define TOKEN_TYPE_FLOAT 3
#define TOKEN_TYPE_IDENT 4
/*=======================保留字======================*/
#define TOKEN_TYPE_RSRVD_VAR 5
#define TOKEN_TYPE_RSRVD_TRUE 6
#define TOKEN_TYPE_RSRVD_FALSE 7
#define TOKEN_TYPE_RSRVD_IF 8
#define TOKEN_TYPE_RSRVD_ELSE 9
#define TOKEN_TYPE_RSRVD_BREAK 10
#define TOKEN_TYPE_RSRVD_CONTINUE 11
#define TOKEN_TYPE_RSRVD_FOR 12
#define TOKEN_TYPE_RSRVD_WHILE 13
#define TOKEN_TYPE_RSRVD_FUNCTION 14
#define TOKEN_TYPE_RSRVD_RETURN 15
#define TOKEN_TYPE_OP 16
/*=======================分隔符======================*/
#define TOKEN_TYPE_DELIM_COMMA 17             // ,
#define TOKEN_TYPE_DELIM_OPEN_PAREN 18        // (
#define TOKEN_TYPE_DELIM_CLOSE_PAREN 19       // )
#define TOKEN_TYPE_DELIM_OPEN_BRACE 20        // [
#define TOKEN_TYPE_DELIM_CLOSE_BRACE 21       // ]
#define TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE 22  // {
#define TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE 23 // }
#define TOKEN_TYPE_DELIM_SEMICOLON 24         // ;
#define TOKEN_TYPE_DELIM_DOT 25               // .
#define TOKEN_TYPE_DELIM_TYPE 26              // :
#define TOKEN_TYPE_DELIM_NEWLINE 27           // \n

#define TOKEN_TYPE_STRING 28 // string

/*===================================运算符类型==================================*/
#define OP_TYPE_ADD 0                  // +
#define OP_TYPE_SUB 1                  // -
#define OP_TYPE_MUL 2                  // *
#define OP_TYPE_DIV 3                  // /
#define OP_TYPE_MOD 4                  // %
#define OP_TYPE_BITWISE_XOR 5          // ^
#define OP_TYPE_BITWISE_AND 6          // &
#define OP_TYPE_BITWISE_OR 7           // |
#define OP_TYPE_BITWISE_NOT 8          // ~
#define OP_TYPE_LESS 9                 // <
#define OP_TYPE_GREATER 10             // >
#define OP_TYPE_LOGICAL_NOT 11         // !
#define OP_TYPE_ASSIGN 12              // =
#define OP_TYPE_INC 13                 // ++
#define OP_TYPE_ASSIGN_ADD 14          // +=
#define OP_TYPE_DEC 15                 // --
#define OP_TYPE_ASSIGN_SUB 16          // -=
#define OP_TYPE_ASSIGN_MUL 17          // *=
#define OP_TYPE_ASSIGN_DIV 18          // /=
#define OP_TYPE_ASSIGN_MOD 19          // %=
#define OP_TYPE_ASSIGN_XOR 20          // ^=
#define OP_TYPE_ASSIGN_AND 21          // &=
#define OP_TYPE_LOGICAL_AND 22         // &&
#define OP_TYPE_ASSIGN_OR 23           // |=
#define OP_TYPE_LOGICAL_OR 24          // ||
#define OP_TYPE_BITWISE_SHIFT_LEFT 25  // <<
#define OP_TYPE_LESS_EQUAL 26          // <=
#define OP_TYPE_BITWISE_SHIFT_RIGHT 27 // >>
#define OP_TYPE_GREATER_EQUAL 28       // >=
#define OP_TYPE_NOT_EQUAL 29           // !=
#define OP_TYPE_EQUAL 30               // ==
#define OP_TYPE_ASSIGN_SHIFT_LEFT 31   // <<=
#define OP_TYPE_ASSIGN_SHIFT_RIGHT 32  // >>=

#define SYMBOL_TYPE_VAR 0
#define SYMBOL_TYPE_PARAM 1

#define ICODE_INSTR 0
#define ICODE_SOURCE_LINE 1
#define ICODE_JUMP_TARGET 2

#define INSTR_MOV 0 // mov a, 2
#define INSTR_ADD 1 // add a, 2
#define INSTR_SUB 2 // sub a, 2
#define INSTR_MUL 3 // mul a, 2
#define INSTR_DIV 4
#define INSTR_MOD 5
#define INSTR_EXP 6
#define INSTR_NEG 7
#define INSTR_INC 8
#define INSTR_DEC 9
#define INSTR_AND 10
#define INSTR_OR 11
#define INSTR_XOR 12
#define INSTR_NOT 13
#define INSTR_SHL 14
#define INSTR_SHR 15
#define INSTR_CONCAT 16
#define INSTR_GETCHAR 17
#define INSTR_SETCHAR 18
#define INSTR_JMP 19
#define INSTR_JE 20
#define INSTR_JNE 21
#define INSTR_JG 22
#define INSTR_JL 23
#define INSTR_JGE 24
#define INSTR_JLE 25
#define INSTR_PUSH 26
#define INSTR_POP 27
#define INSTR_CALL 28
#define INSTR_RET 29
#define INSTR_PAUSE 30
#define INSTR_EXIT 31

#define INSTR_NEW_OBJ 32
#define INSTR_SAVE_FIELD 33
#define INSTR_SET_FIELD 34
#define INSTR_GET_FIELD 35
#define INSTR_NEW_ARRAY 36
#define INSTR_OCALL 37
#define INSTR_ADD_FIELD 38
#define INSTR_SUB_FIELD 39
#define INSTR_MUL_FIELD 40
#define INSTR_DIV_FIELD 41
#define INSTR_MOD_FIELD 42

#define MAX_OP_COUNT 3

#define GAIA_OP_STRING 0
#define GAIA_OP_INT 1
#define GAIA_OP_NUMBER 2
#define GAIA_OP_STACK_INDEX 3
#define GAIA_OP_EAX 4
#define GAIA_OP_EBX 5
#define GAIA_OP_ECX 6
#define GAIA_OP_INSTR 7
#define GAIA_OP_NULL 8
#define GAIA_OP_FUNC_INDEX 9
#define GAIA_OP_VAR 10
#define GAIA_OP_JUMP_TARGET_INDEX 11
#define GAIA_OP_OBJECT 12
#define GAIA_OP_ARRAY 13

namespace Gaia {
typedef struct Error_ {
  int code;                 //错误号
  int line;                 //发生错误的源代码行
  char source[4096];        //源代码
  int index;                //源代码行发生错误的位置
  char msg[ERROR_MSG_SIZE]; //错误消息
} Error;

typedef struct Reservedword_ {
  int token;
  char ident[MAX_IDENT_SIZE];
} Reservedword;

class Script;
typedef int (*pLocalFn)(Script *gaia);

//运行时函数
typedef struct _Function_ {
  int size;
  int entryPoint;
  int paramCount;
  int localSize;
  int hashVal;
  int returnSize;
  pLocalFn pfn;
  int index;

} Function;

char *getMnemonics(int type);

//字符串hash函数
int hash(char *source);

char *getValueType(int p);
} // namespace Gaia

extern "C" Gaia::Error error;

#define CLEAR_ERROR error.code = -1