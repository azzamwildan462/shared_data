#include "shared_data/shared_data.h"
#include <stdint.h>
#include <signal.h>

semShm_t shm_ex1;

void sigint_handler(int sig)
{
    shm_remove(&shm_ex1);
    exit(0);
}

void delay(int milliseconds)
{
    long pause;
    clock_t now, then;

    pause = milliseconds * (CLOCKS_PER_SEC / 1000);
    now = then = clock();
    while ((now - then) < pause)
        now = clock();
}

int main()
{
    signal(0x02, sigint_handler);

    int8_t key = 0x12;
    uint16_t shm_size = 64;
    uint8_t ret = init_shared_mem(&shm_ex1, key, shm_size);
    if (ret == -1)
    {
        printf("Failed open shared memory\n");
        return 0;
    }

    printf("Success open shared memory\n");

    while (1)
    {
        delay(1000);
        char buffer2[64];
        uint8_t read_bytes = 13;
        shm_read(&shm_ex1, (void *)buffer2, read_bytes);
        printf("Reading %s (%d) from shared memory\n", buffer2, read_bytes);

        // Extract data
        char str[9];
        uint8_t data1_get;
        int16_t data2_get;
        uint16_t data3_get;
        memcpy(str, buffer2, 8);
        memcpy(&data1_get, buffer2 + 8, 1);
        memcpy(&data2_get, buffer2 + 9, 2);
        memcpy(&data3_get, buffer2 + 11, 2);

        printf("Extracted Data: \n");
        printf("str: %s\n", str);
        printf("data1: %d\n", data1_get);
        printf("data2: %d\n", data2_get);
        printf("data3: %d\n", data3_get);
    }

    return 0;
}