#include "shared_data/shared_data.h"
#include <stdint.h>

int main()
{
    semShm_t shm_ex1;
    int8_t key = 0x000012;
    uint16_t shm_size = 64;
    uint8_t ret = init_shared_mem(&shm_ex1, key, shm_size);
    if (ret == -1)
    {
        printf("Failed open shared memory\n");
        return 0;
    }

    printf("Success open shared memory\n");

    // char buffer1[64] = "itssmasa";
    // printf("Writing %s (%d) to shared memory\n", buffer1, 8);
    // shm_write(&shm_ex1, buffer1, 8);

    char buffer2[64];
    uint8_t read_bytes = 5;
    shm_read(&shm_ex1, buffer2, read_bytes);
    printf("Reading %s (%d) from shared memory\n", buffer2, read_bytes);

    shm_remove(&shm_ex1);

    return 0;
}