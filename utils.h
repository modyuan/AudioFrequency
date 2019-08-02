
#ifndef TESTCPP_UTILS_H
#define TESTCPP_UTILS_H


#include <stdio.h>

#define CAVERR(obj,msg) fprintf(stderr,"[ERROR] %s: %s\n",(obj),(msg))
#define CAVINFO(obj,msg) fprintf(stdout,"[INFO] %s: %s\n",(obj),(msg))

#define CAVERRX(obj,msg, ...) fprintf(stderr,"[ERROR] " obj ": " msg, __VA_ARGS__)
#define CAVINFOX(obj,msg, ...) fprintf(stdout,"[INFO] " obj ": " msg, __VA_ARGS__)


void InitConsole();

#endif //TESTCPP_UTILS_H
