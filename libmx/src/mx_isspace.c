#include "../inc/libmx.h"

bool mx_isspace(char c){
    return (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' ? 1 : 0);
}
