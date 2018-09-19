#include "gaia.h"
using namespace Gaia;

Error error;

Script::Script(void)
{
    runtime_.pushLocalFunction("print", gaia_print);
    runtime_.pushLocalFunction("toString", gaia_tostring);
    runtime_.pushLocalFunction("toInt", gaia_toInt);
    runtime_.pushLocalFunction("toNumber", gaia_toNumber);
    runtime_.pushLocalFunction("getTimer", gaia_getTimer);
    runtime_.pushLocalFunction("typeof", gaia_typeof);
    runtime_.pushLocalFunction("toLowerCase", gaia_toLowerCase);
    runtime_.pushLocalFunction("toUpperCase", gaia_toUpperCase);
}

Script::~Script(void)
{
}

void Script::formatErrorMessage(char *szMsg, int code)
{

    switch (code)
    {
    case ERROR_IVALID_SOURCE_FILE:
        sprintf(szMsg, "INVALID_SOURCE_FILE");
        break;
    case GERROR_INVALID_TOKEN:
        sprintf(szMsg, "ERRPR_INVALID_TOKEN");
        break;

    case ERROR_UNEXPECTED_END_OF_FILE:
        sprintf(szMsg, "UNEXPECTED_END_OF_FILE");
        break;

    case ERROR_UNEXPECTED_INPUT:
        sprintf(szMsg, "UNEXPECTED_INPUT");
        break;

    case ERROR_ILLEGAL_ASSIGN_OP:
        sprintf(szMsg, "ILLEGAL_ASSIGN_OP");
        break;

    case ERROR_UNEXPECTED_END_OF_LINE:
        sprintf(szMsg, "UNEXPECTED_END_OF_LINE");
        break;

    case ERROR_NESTED_FUNCTION_ILLEGAL:
        sprintf(szMsg, "NESTED_FUNCTION_ILLEGAL");
        break;
    case ERROR_UNEXPECTED_BLOCK:
        sprintf(szMsg, "UNEXPECTED_BLOCK");
        break;
    case GERROR_UNEXPECTED_RETURN:
        sprintf(szMsg, "UNEXPECTED_RETURN");
        break;
    case GERROR_UNDEFINED_FUNCTION_CALL:
        sprintf(szMsg, "UNDEFINED_FUNCTION_CALL");
        break;
    case GERROR_MORE_PARAM:
        sprintf(szMsg, "MORE_PARAM");
        break;
    case GERROR_FEW_PARAM:
        sprintf(szMsg, "FEW_PARAM");
        break;
    case GERROR_INVALID_INPUT:
        sprintf(szMsg, "INVALID_INPUT");
        break;
    case GERROR_UNDEFINED_VAR:
        sprintf(szMsg, "UNDEFINED_VAR");
        break;
    case GERROR_UNEXPECTED_BREAK:
        sprintf(szMsg, "UNEXPECTED_BREAK");
        break;

    case GERROR_INVALID_LEFT_HAND_SIDE_ASSGIN:
        sprintf(szMsg, "INVALID_LEFT_HAND_SIDE_ASSGIN");
        break;

    default:
        break;
    }
}

void Script::execFile(const char *filename)
{
    FILE *pFile;
    if (!(pFile = fopen(filename, "rb")))
    {
        error.code = ERROR_IVALID_SOURCE_FILE;
        return;
    }

    fseek(pFile, 0, SEEK_END);
    int size = ftell(pFile);

    char *pSource = new char[size + 1];

    fseek(pFile, 0, SEEK_SET);
    fread(pSource, 1, size, pFile);
    fclose(pFile);
    pSource[size] = '\0';

    compile(pSource);

    delete[] pSource;
}

void Script::execString(const char *str)
{
    compile((char *)str);
    runtime_.begin(parse_.getByteCode(), this);
}

void Script::compile(char *pSource)
{
    lexer_.analyze(pSource);

    parse_.setLexer(&lexer_);

    parse_.parse(&runtime_);

	printICode("d:/2.txt");

    runtime_.begin(parse_.getByteCode(), this);
}

void Script::printICode(const char *filename)
{
    parse_.printICode(filename);
}

BOOL Script::call(char *name)
{
    return runtime_.call(name);
}

BOOL Script::getGlobal(char *name)
{
    return runtime_.getGlobal(name);
}

BOOL Script::isInt()
{
    return runtime_.isInt();
}

BOOL Script::IsNumber()
{
    return runtime_.IsNumber();
}

BOOL Script::isString()
{
    return runtime_.isString();
}

int Script::toInt()
{
    return runtime_.toInt();
}

float Script::toNumber()
{
    return runtime_.toNumber();
}

char *Script::toString()
{
    return runtime_.toString();
}

GaiaValue Script::popValue()
{
    return runtime_.pop();
}

void Script::pushInt(int val)
{
    runtime_.pushInt(val);
}

void Script::GC()
{
    runtime_.GC();
}

void Script::pushNumber(float val)
{
    runtime_.pushNumber(val);
}

void Script::pushString(char *str)
{
    runtime_.pushString(str);
}

void Script::getType(char *buf)
{
    GaiaValue val = runtime_.pop();
    if (val.type == GAIA_OP_INT)
    {
        strcpy(buf, "int");
    }
    else if (val.type == GAIA_OP_NUMBER)
    {
        strcpy(buf, "number");
    }
    else if (val.type == GAIA_OP_STRING)
    {
        strcpy(buf, "string");
    }
}
