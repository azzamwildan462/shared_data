#include "shared_data/shared_data.h"
#include <stdint.h>
#include <signal.h>
#include <time.h>

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

    // Prepare data to send
    char buffer1[64] = "itssmasa";
    char itssmasa[9] = "itssmasa";
    char smasaits[9] = "smasaits";
    uint8_t data1 = 12;
    int16_t data2 = -2353;
    uint16_t data3 = 3456;

    // Additional data for buffer1
    memcpy(buffer1 + 8, &data1, 1);
    memcpy(buffer1 + 9, &data2, 2);
    memcpy(buffer1 + 11, &data3, 2);

    uint8_t bytes_to_send = 13;
    printf("Writing %s (%d) to shared memory\n", buffer1, bytes_to_send);
    shm_write(&shm_ex1, (void *)buffer1, bytes_to_send);

    uint64_t cnt = 0;
    while (1)
    {
        delay(1000);
        data1++;
        data2--;
        data3 += 2;

        if (!(cnt & 1))
            memcpy(buffer1, itssmasa, 8);
        else
            memcpy(buffer1, smasaits, 8);
        memcpy(buffer1 + 8, &data1, 1);
        memcpy(buffer1 + 9, &data2, 2);
        memcpy(buffer1 + 11, &data3, 2);

        uint8_t bytes_to_send = 13;
        printf("Writing %s (%d) to shared memory\n", buffer1, bytes_to_send);
        shm_write(&shm_ex1, (void *)buffer1, bytes_to_send);

        cnt++;
    }

    return 0;
}