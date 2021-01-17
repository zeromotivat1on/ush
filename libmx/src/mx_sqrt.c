#include "../inc/libmx.h"

int mx_sqrt(int x){
    float temp = 0;
    float sqrt = x / 2;
    if (x <= 0) return 0;
    while (sqrt != temp) {
        temp = sqrt;
        sqrt = (x / temp + temp) / 2;
    }
    return (sqrt - (int)sqrt == 0) ? sqrt : 0;
}
