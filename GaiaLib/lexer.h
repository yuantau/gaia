#pragma once
#include "def.h"
#include "list.h"

namespace Gaia
{
//运算符状态
typedef struct OpState_
{
    char c;
    int type;
    int subStateCount;
    char subStateChar[10];
} OpState;

typedef struct Token_
{
    int startIndex;               //在源代码行中开始的位置
    int line;                     //在源代码中的行数
    int value;                    //Token值
    int op;                       //操作符
    char lexeme[MAX_LEXEME_SIZE]; //单词的值
    BOOL isRelationalOp()
    {
        if (op == OP_TYPE_EQUAL ||
            op == OP_TYPE_NOT_EQUAL ||
            op == OP_TYPE_LESS ||
            op == OP_TYPE_GREATER ||
            op == OP_TYPE_LESS_EQUAL ||
            op == OP_TYPE_GREATER_EQUAL)
            return TRUE;
        return FALSE;
    }
    BOOL isLogicalOp()
    {
        if (op == OP_TYPE_LOGICAL_AND ||
            op == OP_TYPE_LOGICAL_OR ||
            op == OP_TYPE_LOGICAL_NOT)
            return TRUE;
        return FALSE;
    }
} Token;

class Lexer
{
  public:
    Lexer(void);
    ~Lexer(void);

  public:
    //分析源代码
    void analyze(char *pSource);

    //获取源代码行
    char *getSourceLine(int index);

    //读取当前Token
    Token *getPrevToken();

    //读取下一个Token
    Token *getNextToken();

    //回退一次
    void rewindToken();

    //检查下一个Token是否置顶的值
    Token *checkToken(int val);

    //向前查看一个字符
    char lookAhead();

    //重置
    void reset();

  private:
    //检查字符是否是空白字符
    BOOL isWhitespace(char c);

    //检查字符是否是一个数字
    BOOL isNumberic(char c);

    //检查字符是否是一个分隔符
    int checkDelim(char c);

    //检查字符是否是操作符
    BOOL checkOp(OpState *pState, char c);

    //检查字符是否是标识符
    BOOL isIdent(char c);

    //读取下一个字符
    char readChar();

    //读取下一个Token
    int readToken();

    //检查标识符是否保留字,如果是,返回保留字Token
    int checkReservedword(char *ident);

    //预处理
    void preprocess(char *pSource);

    //发生错误时设置错误消息
    void setErrorMsg(int line, int index, char *msg);

  private:
    List<char *> sourceList_;
    ListNode<char *> *pCurrSourceNode_;

    List<Token *> tokenList_;

    int lineIndex_;

    //当前源码行长度
    int currSourceLineSize_;

    //指示当前单词的开始，结束位置
    int startIndex_;
    int endIndex_;

    //当前单词
    char currLexeme_[MAX_LEXEME_SIZE];

    int currTokenValue_;
    int currOp_;

    int readIndex_;
};
} // namespace Gaia
