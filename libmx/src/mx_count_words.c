#include "../inc/libmx.h"

int mx_count_words(const char *str, char c) {
    bool state = true;
    int count = 0;
    if(!str) return -1;
    else for(int i = 0; str[i] != '\0'; ++i){
            if(str[i] == c) state = true;
            else if(state == true){state = false; count++;}
        }
    
    return count;
}
