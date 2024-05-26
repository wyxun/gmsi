#include "glog.h"
#include "utilities/util_debug.h"

const char *stringErrorMessage[] = {
    "Success num(0)",
    "Operation not permitted num(1)",
    "No such file or directory num(2)",
    "No such process num(3)",
    "Interrupted system call num(4)",
    "I/O error num(5)",
    "No such device or address num(6)",
    "Arg list too long num(7)",
    "Exec format error val(8)",
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
    "EFail num(20)"
    "Not a directory num(21)"
};

/**
 * Function: gerr_Printf
 * ----------------------------
 * This function prints an error message, including the error message, the file where the error occurred, 
 * and the line number of the error.
 *
 * Parameters: 
 * wErrorNum: The error number.
 * pFile: The file where the error occurred.
 * wLine: The line number of the error.
 *
 * Returns: 
 * None
 */
void gerr_Printf(int wErrorNum, char* pFile, int wLine)
{
    LOG_OUT("gerr_Printf:");
    
    // Get the size of the stringErrorMessage array
    size_t size = sizeof(stringErrorMessage) / sizeof(stringErrorMessage[0]);

    // Check if the error number is within the range of the array
    if(wErrorNum >= size || wErrorNum < 0)
        LOG_OUT("no such error number(n)");
    else
        LOG_OUT(stringErrorMessage[wErrorNum]);
    
    LOG_OUT("; in file: ");
    LOG_OUT(pFile);
    LOG_OUT("; in line: ");
    LOG_OUT(wLine);
    LOG_OUT(".\n");
}

/**
 * Function: glog_Printf
 * ----------------------------
 * This function prints a log message, including the log message, the file where the log occurred, 
 * and the line number of the log.
 *
 * Parameters: 
 * pString: The log message.
 * pFile: The file where the log occurred.
 * wLine: The line number of the log.
 *
 * Returns: 
 * None
 */
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

/**
 * Function: gval_Printf
 * ----------------------------
 * This function prints a log message with a value, including the log message, the value, 
 * the file where the log occurred, and the line number of the log.
 *
 * Parameters: 
 * pString: The log message.
 * wValue: The value to be printed.
 * pFile: The file where the log occurred.
 * wLine: The line number of the log.
 *
 * Returns: 
 * None
 */
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

