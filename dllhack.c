#include<stdio.h>

void main(int argc, char **argv) {
        FILE *f = fopen("gamex86.dll", "r+b");
        char *buf = (char *)malloc(2048);
        int i;

        fseek(f, -2048, SEEK_END);
        fread(buf, 1, 2048, f);

        i = 2048;
        while(--i) {
                if(buf[i] == '_') {
                        if (!strcmp("_GetGameAPI", &buf[i])) {
                                printf("Found _GetGameAPI\n");
                                strcpy(&buf[i], "GetGameAPI");
                                break;
                        }
                }
        }
        fseek(f, -2048, SEEK_END);
        fwrite(buf, 1, 2048, f);
        fclose(f);
}

