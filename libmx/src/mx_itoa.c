#include "../inc/libmx.h"

char *mx_itoa(int number){
    int len = 12;
    long n = number;
    char *str = mx_strnew(len);
    int sign, i = 0;
    if(number == 0) return "0";
    if((sign = number) < 0) n *= -1;
    while(n != 0){
        str[i] = n % 10 + '0'; 
        n /= 10; 
        ++i;
    }
    if(sign < 0) str[i] = '-';
    mx_str_reverse(str);
    str[i + 1] = (char)NULL;
    return str;
}
