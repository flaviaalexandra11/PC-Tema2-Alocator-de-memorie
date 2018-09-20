//Popescu Flavia-Alexandra 311CD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int N, k = 0;
unsigned char *v, p[65536];
uint32_t *t; 

/*
pentru fiecare octet din arena se va tine minte 
indexul blocului din care face parte, indexul urmator,
indexul anterior si dimensiunea sectiunii de date
*/

struct gest
{
    int next, last, crt, size;

}s[65536];

/*
initializam vectorul s si alocam memorie pentru v;
folosim inca un vector p pe care il initializam cu 0 pentru 
a tine minte portiunile de memorie ocupate
*/

void INITIALIZE(int x)
{
    int i;
    N = x;

    v = malloc(N*sizeof(unsigned char));

    for(i = 0; i < N; i++) 
    {
        v[i] = 0;
        p[i] = 0;

        s[i].next = 0;
        s[i].last = 0;
        s[i].crt = 0;
        s[i].size = 0;
    }
}


void DUMP()
{
    int i, k=1, l=0;

    for(i = 0; i < N-1 && N != 0; i++)
    {
        if(k==1)
        {
            printf("%08X\t", l);
            printf("%02X ", v[i]);
        }
        else if(k == 8) printf("%02X  ", v[i]);
            else if(k != 16) printf("%02X ", v[i]);
                else{
                        printf("%02X\n", v[i]);
                        k=0;
                        l+=16;
                    }
        k++;
    }

    if(k == 1 && N != 0)
    {
        printf("%08X\t", l);
        printf("%02X\n",v[N - 1]);
    }
    else if (N != 0) 
        {
            printf("%02X ",v[N - 1]);
            printf("\n");
        }
}


void FINALIZE()
{
    free(v);
}


void internalAlloc(int index, int x)
{
    int i;

    printf("%d\n", index+12);

    for(i = index; i < index + 12; i++)
    {
        p[i] = 'g';
    }
    for(i = index + 12; i < index + x + 12; i++) 
    {
        p[i] = 'd';
    }
    
    // caut daca exista un bloc dupa mine
    int nextIndex = -1, k;
    for(k = index + 12 + x; k < N && (nextIndex == -1); k++)
    {
        if(p[k] != 0)
        {
            nextIndex = k;
        }
    }

    // actualizez informatiile pentru blocul curent
    for(i = index; i < index + 12 + x; i++) 
    {
        s[i].crt = index;
        s[i].size = x + 12;
        if(index == 0)
            s[i].last = -1;
        else
            s[i].last = s[index - 1].crt;
        if(index > 0 && s[index - 1].size > 0)
        {
            s[i].next = s[index - 1].next;
        }
        else
        {
            if(nextIndex == -1)
                s[i].next = -1;
            else
                s[i].next = nextIndex;
        }
    }

    // actualizez next pentru blocul anterior daca exista
    if(index > 0 && s[index - 1].size > 0)
    {
        for(i = index - 1; i >= s[index - 1].crt; i--)
        {
            s[i].next = index;
        }
    }

    // actualizez last pentru blocul urmator daca exista
    if(nextIndex != -1)
    {
        for(i = nextIndex; i < s[nextIndex].crt + s[nextIndex].size; i++)
        {
            s[i].last = index;
        }
    }

    // modificam arena pentru blocul curent
    t = (uint32_t *) (v + index);
    if(s[index].next == -1)
        *t = 0;
    else
        *t = s[index].next;
    if(s[index].last == -1)
        *(t + 1) = 0;
    else
        *(t + 1) = s[index].last;
    *(t + 2) = s[index].size - 12;

    // modificam arena pentru blocul anterior, daca exista
    if(s[index].last != -1)
    {
        t = (uint32_t *) (v + s[index].last);
        *t = index;
    }

    // modificam arena pentru blocul urmator, daca exista
    if(s[index].next != -1)
    {
        t = (uint32_t *) (v + s[index].next + 4);
        *t = index;
    }
}


void ALLOC(int x)
{

    int index = -1, i, j, ok;

    for(i=0; i<N && index == -1; i++)
    {   
        if(i + x + 12 > N) break;
        ok = 1;
    
        for(j = i; j < i + x + 12 && j < N && ok; j++)
            if(p[j] != 0) ok = 0;
    
        if(ok) index = i;      
    }

    if(index == -1)
    {
        printf("%d\n", 0);
        return;
    }

    internalAlloc(index, x);
}


void FREE(int x)
{
    int index = x - 12;
    int size = s[index].size;
    int i;

    // actualizam next pentru vecinul anterior, daca exista
    if(s[index].last != -1)
    {
        for(i = s[index].last; i < s[index].last + s[s[index].last].size; i++)
        {
            s[i].next = s[index].next;
        }
        t = (uint32_t *) (v + s[index].last);
        if(s[index].next == -1)
            *t = 0;
        else
            *t = s[index].next;
    }

    // actualizam last pentur vecinul urmator, daca exista
    if(s[index].next != -1)
    {
        for(i = s[index].next; i < s[index].next + s[s[index].next].size; i++)
        {
            s[i].last = s[index].last;
        }
        t = (uint32_t *) (v + s[index].next);
        if(s[index].last == -1)
            *(t + 1) = 0;
        else
            *(t + 1) = s[index].last;
    }

    // stergem  blocul curent
    for(i = index; i < index + size; i++)
    {
        v[i] = 0;
        p[i] = 0;
        s[i].size = 0;
        s[i].crt = 0;
        s[i].next = 0;
        s[i].last = 0;
    }

}


void FILL(int index, int size, int value)
{
    int i = index;
    while(size > 0 && i < N)
    {
        if(p[i] == 'd')
        {
            v[i] = value;
            size--;
        }
        i++;
    }
} 


void ALLOCALIGNED(int x, int align)
{
    int index = -1, i, j, ok;

    for(i=0; i<N && index == -1; i++)
    {   
        if(i % align != 0)
            continue;

        if(i + x + 12 > N)
            break;
        ok = 1;
        for(j = i; j < i + x + 12 && j < N && ok; j++)
            if(p[j] != 0)
                ok = 0;
        if(ok)
        {
            index = i;
        }        
    }

    if(index == -1)
    {
        printf("%d\n", 0);
        return;
    }

    internalAlloc(index, x);
}


void parse_command(char* cmd)
{
    const char* delims = " \n";

    char* cmd_name = strtok(cmd, delims);
    if (!cmd_name) {
        goto invalid_command;
    }

    if (strcmp(cmd_name, "INITIALIZE") == 0) {
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        INITIALIZE(size);

    } else if (strcmp(cmd_name, "FINALIZE") == 0) {
        FINALIZE();

    } else if (strcmp(cmd_name, "DUMP") == 0) {
        DUMP();

    } else if (strcmp(cmd_name, "ALLOC") == 0) {
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        ALLOC(size);

    } else if (strcmp(cmd_name, "FREE") == 0) {
        char* index_str = strtok(NULL, delims);
        if (!index_str) {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        FREE(index);

    } else if (strcmp(cmd_name, "FILL") == 0) {
        char* index_str = strtok(NULL, delims);
        if (!index_str) {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        char* value_str = strtok(NULL, delims);
        if (!value_str) {
            goto invalid_command;
        }
        uint32_t value = atoi(value_str);
        FILL(index, size, value);

    } else if (strcmp(cmd_name, "ALLOCALIGNED") == 0) {
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        char* align_str = strtok(NULL, delims);
        if (!align_str) {
            goto invalid_command;
        }
        uint32_t align = atoi(align_str);
        ALLOCALIGNED(size, align);

    } else if (strcmp(cmd_name, "REALLOC") == 0) {
        printf("Found cmd REALLOC\n");
        char* index_str = strtok(NULL, delims);
        if (!index_str) {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        // TODO - REALLOC

    } else {
        goto invalid_command;
    }

    return;

invalid_command:
    printf("Invalid command: %s\n", cmd);
    exit(1);
}


int main(void)
{
    ssize_t read;
    char* line = NULL;
    size_t len;

    /* parse input line by line */
    while ((read = getline(&line, &len, stdin)) != -1) {
        /* print every command to stdout */
        printf("%s", line);

        parse_command(line);
    }

    free(line);

    return 0;
}