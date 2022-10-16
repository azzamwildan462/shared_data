#include "unistd.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

typedef struct config_tag
{
    uint16_t port;
    char multicast_addr[20];
} config_t;

int8_t load_config(config_t *cfg)
{
    char *filename = "config/multicast.cfg";
    FILE *f = fopen(filename, "r");

    char buffer[BUFSIZ];
    char *line;
    int i;
    while (fgets(buffer, sizeof(buffer), f) != NULL)
    {
        for (i = 0; i < strlen(buffer); i++)
        { // iterate through the chars in a line
            if (buffer[i] == '\n' || buffer[i] == NULL)
            { // if char is a #, stop processing chars on this line
                break;
            }
            else if (buffer[i] == ' ')
            { // if char is whitespace, continue until something is found
                continue;
            }
            else
            {
                if (buffer[0] == 'a' && buffer[1] == 'd' && buffer[2] == 'd' && buffer[3] == 'r')
                    sscanf(buffer, "addr: %s", cfg->multicast_addr);
                else if (buffer[0] == 'p' && buffer[1] == 'o' && buffer[2] == 'r' && buffer[3] == 't')
                    sscanf(buffer, "port: %d", &cfg->port);
                break;
            }
        }
    }

    fclose(f);
}

int main()
{
    config_t cfg;
    load_config(&cfg);
    printf("addr: %s\n", cfg.multicast_addr);
    printf("port: %d\n", cfg.port);
    return 0;
}