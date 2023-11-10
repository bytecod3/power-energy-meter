#include <stdio.h>

void substring(char[], char[], int, int);

int main(){
    char string[1000], sub[1000];
    int position, length, c=0;

    printf("string:\n");
    gets(string);

    printf("position, length of substring\n");
    scanf("%d%d", &position, &length);

    substring(string, sub, position, length);

    printf("Substring is: %s \n", sub);

    return 0;
    
}

void substring(char string[], char substring[], int p, int length){
    int c = 0;

    while (c< length)
    {   
        substring[c] = string[p + c - 1];
        c++;
    }

    substring[c] = '\0';
    
}
