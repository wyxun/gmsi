#ifndef __GLOG_H__
#define __GLOG_H__

void glog_Printf(const char *pString, char* pFile, int wLine);
void gerr_Printf(int wErrorNum, char* pFile, int wLine);
void gval_Printf(const char *pString, int wValue, char* pFile, int wLine);

#define GLOG_PRINTF(S)      glog_Printf(S, __FILE__, __LINE__)
#define GERR_PRINTF(S)      gerr_Printf(S, __FILE__, __LINE__)
#define GVAL_PRINTF(V)      gval_Printf(#V, V, __FILE__, __LINE__)
#endif