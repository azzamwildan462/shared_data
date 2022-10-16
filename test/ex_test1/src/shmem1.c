#include "shared_data/shared_data.h"
#include <stdint.h>

int main()
{
    semShm_t shm_ex1;
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
    uint8_t data1 = 12;
    int16_t data2 = -2353;
    uint16_t data3 = 3456;

    // Additional data for buffer1
    memcpy(buffer1 + 8, &data1, 1);
    memcpy(buffer1 + 9, &data2, 2);
    memcpy(buffer1 + 11, &data3, 2);

    uint8_t bytes_to_send = 13;
    printf("Writing %s (%d) to shared memory\n", buffer1, bytes_to_send);
    shm_write(&shm_ex1, buffer1, 8);

    char buffer2[64];
    uint8_t read_bytes = 13;
    shm_read(&shm_ex1, buffer2, read_bytes);
    printf("Reading %s (%d) from shared memory\n", buffer2, read_bytes);
    printf("Data: \n");

    shm_remove(&shm_ex1);

    return 0;
}