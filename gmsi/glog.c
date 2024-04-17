#include "glog.h"
#include "utilities/util_debug.h"

const char *stringErrorMessage[21];

void gerr_Printf(int wErrorNum, char* pFile, int wLine)
{
    LOG_OUT("gerr_Printf:");
    if(wErrorNum>20)
        LOG_OUT("no such error number(n)");
    else
        LOG_OUT(stringErrorMessage[abs(wErrorNum)]);
    LOG_OUT("; in file: ");
    LOG_OUT(pFile);
    LOG_OUT("; in line: ");
    LOG_OUT(wLine);
    LOG_OUT(".\n");
}


void glog_Printf(const char *pString, char* pFile, int wLine)
{
    LOG_OUT("glog_Printf:");
    LOG_OUT(pString);
    LOG_OUT("; in file: ");
    LOG_OUT(pFile);
    LOG_OUT(", in line: ");
    LOG_OUT(wLine);
    LOG_OUT(".\n");
}

void gval_Printf(const char *pString, int wValue, char* pFile, int wLine)
{
    LOG_OUT("gval_Printf:");
    LOG_OUT(pString);
    LOG_OUT(": ");
    LOG_OUT(wValue);
    LOG_OUT("; in file: ");
    LOG_OUT(pFile);
    LOG_OUT(", in line: ");
    LOG_OUT(wLine);

    LOG_OUT(".\n");
}

const char *stringErrorMessage[21] = {
    "Success num(0)",
    "Operation not permitted num(1)",
    "No such file or directory num(2)",
    "No such process num(3)",
    "Interrupted system call num(4)",
    "I/O error num(5)",
    "No such device or address num(6)",
    "Arg list too long num(7)",
    "Exec format error num(8)",
    "Bad file number num(9)",
    "No child processes num(10)",
    "Try again num(11)",
    "Out of memory num(12)",
    "Permission denied num(13)",
    "Bad address num(14)",
    "Block device required num(15)",
    "Device or resource busy num(16)",
    "File exists num(17)",
    "Cross-device link num(18)",
    "No such device num(19)",
    "Not a directory num(20)"
};