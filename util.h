#include <string.h>
#include <stdbool.h>

bool is_space(char c){
    return c == ' ';
}


// Trims string from the beginning and end.
void trim(char *str){

    size_t len = strlen(str);

    while (is_space(str[0])){
        str = str + 1;
    }

    int endp = len - 1;
    while (is_space(str[endp])){
       endp--;
    }

    char *new_str = malloc(sizeof(char) * (endp + 1));
    int index = 0;
    while(index < endp + 1){
        new_str[index] = str[index];
        index += 1;
    }

    str = new_str;

}