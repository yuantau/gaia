#pragma once
#include "def.h"
#include "list.h"
#include "memorypool.h"
#include "stream.h"

namespace Gaia {
//跳转目标
typedef struct _Label_ {
  int index;
  int target;
} Label;

//操作数
class ICodeOp {
public:
  friend class Parse;
  friend class FunctionNode;

  int type_;
  union {
    int intLiteral;
    float floatLiteral;
    int stringIndex;
    int symbolIndex;
    int jumpTarget;
    int funcIndex;
    int reg;
  };
  int offset;
  int key; //键偏移
};

//指令
class ICodeNode {
public:
  int type;       //类型
  char *source;   //源码标注
  int jumpTarget; //跳转目标
  int opCode;     //操作码

  ICodeOp *addOp(int type);  //添加操作数
  ICodeOp *getOp(int index); //获取操作数
  List<ICodeOp *> opList;    //操作数链表
};

//符号类
class SymbolNode {
public:
  SymbolNode() : isParam(FALSE) {}
  friend class Parse;

public:
  int index;                 //索引
  char name[MAX_IDENT_SIZE]; //名字
  int size;                  //大小
  int scope;                 //作用于
  int type;                  //类型
  int stackIndex;            //堆栈索引
  BOOL isParam;              //是否参数
};

//函数类
class FunctionNode {
public:
  FunctionNode() : paramCount_(0), localDataSize_(0), returnSize_(0){};
  ~FunctionNode(){};
  friend class Parse;

  //添加源码注释到指令流
  void addSourceLine(char *source);

  //添加指令到指令流
  ICodeNode *addInstr(int opCode);

  //添加跳转目标
  void addJumpTarget(int targetIndex);

  void clear();

  char *getName();

  int getIcodeSize();

  ICodeNode *getIcode(int index);

  int getLocalDataSize() { return localDataSize_; }
  int getParamCount() { return paramCount_; }

  void setReturnSize(int size) { returnSize_ = size; }

  int getReturnSize() { return returnSize_; }

private:
  char temp[2048];
  int index_;                     //索引
  char name_[MAX_IDENT_SIZE];     //名字
  int paramCount_;                //参数数量
  List<ICodeNode *> icodeStream_; //中间代码
  int localDataSize_;
  int returnSize_;
};

class ICode {
public:
  ICode(void);
  ~ICode(void);
  void generateAsmFile();
  void generateByteCode(List<FunctionNode *> *pFuncTable,
                        List<SymbolNode *> *pSymbolTable,
                        List<char *> *pStringTable);
  Stream *getStream();
  void printStream();

private:
  void copyOp(ICodeOp *pDest, ICodeOp *pSource, FunctionNode *pFunc);

private:
  int globalVarSize_;
  List<Label *> labelTable_;
  List<Function *> funcTable_;

private:
  List<SymbolNode *> *pSymbolTable_;
  List<FunctionNode *> *pFuncTable_;
  List<char *> *pStringTable_;
  Stream byteCode_;
};
} // namespace Gaia
