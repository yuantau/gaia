#pragma once
#include "def.h"
#include "list.h"
#include "lexer.h"
#include "ICode.h"
#include "runtime.h"

namespace Gaia
{
class Parse;
class FunctionNode;

//记录循环的位置反倒是
class LoopLocation
{
  public:
    LoopLocation(int start, int end)
        : startTarget_(start), endTarget_(end)
    {
    }
    int startTarget_; //开始
    int endTarget_;   //结束
};

class Parse
{
  public:
    Parse(void);
    ~Parse(void);

  public:
    //打印中间代码
    void printICode(const char *name);

    //设置错误
    void setError(int code);

    //设置词法解析器
    void setLexer(Lexer *pLexer);

    //获取编译后的ByteCode
    Stream * getStream();

    //语法解析
    void parse(Runtime *pRunTime);

    //获取全局数据大小
    int getGlobalVarSize();

    //获取指令条数
    int getInstrSize();

    //获取函数总数
    int getFunctionCount();

    //根据索引获取函数
    FunctionNode *getFunction(int index);

    Stream *getByteCode();

  private:
    //预解析，把所有function提取出来
    void preParse();

    //初始化操作
    void clear();

    //解析源码段
    void parseStatement();

    //解析块
    void parseBlock();

    //解析函数
    void parseFunction();

    //解析变量声明
    void parseVar();

    //解析函数调用
    void parseFunctionCall();

    //解析变量赋值
    void parseAssign();

    //解析对象方法调用
    void parseObjMethodCall();

    //解析表达式
    void parseExpression();

    //解析关系运算符的项
    void parseRelationalExpression();

    //解析子表达式
    void parseSubExpression();

    //解析项
    void parseTerm();

    //解析因式
    void parseFactor();

    //解析函数返回值
    void parseReturn();

    void parseInc_Dec();

    //解析前置自增自减运算符
    void parseFrontInc_Dec();

    //解析while循环
    void parseWhile();

    //解析break, continue语句
    void parseControlCommands(int ctrl);

    //解析if else 语句
    void parseIf();

    //解析for循环
    void parseFor();

    //解析DICT对象
    void parseObj();

    //解析数组
    void parseArray();

    //解析属性赋值
    void parsePropAssign();

    //解析.访问运算符
    void parseDotAccess(BOOL saveKey);

    //解析数组访问运算符
    void parseArrayAccess(BOOL saveKey);

    //添加字符串
    int addString(char *str);

    //添加变量
    SymbolNode *addSymbol(char *name, int scope, int type);

    //获取变量
    SymbolNode *findSymbol(char *name, int scope);

    //根据索引获取变量
    SymbolNode *getSymbol(int index);

    //添加函数
    int addFunction(char *name);

    //根据名字搜索函数
    FunctionNode *findFunction(char *name);

    //获取跳转目标地址
    int getNextJumpTargetIndex();

  private:
    Lexer *pLexer_;
    List<SymbolNode *> symbolTable_;
    List<FunctionNode *> functionTable_;
    List<char *> stringTable_;
    List<LoopLocation *> loopLocation_;

    int currScope; //当前作用域
    SymbolNode *incSymbols[256];
    SymbolNode *decSymbols[256];
    int incIndex;
    int decIndex;

    ICode iCode;
    Runtime *pRuntime_;
};

} // namespace Gaia
