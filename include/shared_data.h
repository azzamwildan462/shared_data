/**
 * @author Okke Hendriks - Tech United Eindhoven
 * @author Azzam Wildan Maulana - IRIS ITS
 */
#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h> /* For O_* constants */

#define NAME_MAX_BYTES 16

    // #define DEBUG_SHARED_MEM // Enabled or disable printf's

    typedef struct
    {
        // Global semapohered shared mem variables
        key_t key;
        sem_t *sem;
        int shmid;
        void *segptr;
        int semid;
        int createdSegment;
        uint8_t r_blocking_flag;
        uint8_t w_blocking_flag;
        uint8_t r_unlock_flag;
        uint8_t w_lock_flag;

    } semShm_t;
    void *shmOpen(int key, int size, int *shmid, sem_t **sem, int *newSegment);
    int shmRemove(int shmid, void *segptr);
    int lockSemaphore(sem_t *sem, int blocking);
    int shmWrite(void *object, int size, int shmid, void *segptr, sem_t *sem, int blocking, int lock);
    int shmRead(void *object, int size, int shmid, void *segptr, sem_t *sem, int blocking, int unlock);
    int shmReadWrite(void *object, int size, int shmid, void *segptr, sem_t *sem, int blocking, int (*f)(void *object));
    int openSemForShm(int shmid, sem_t **sem);
    int closeSemForShm(int shmid);
    int getSemVal(sem_t *sem);

    int8_t init_shared_mem(semShm_t *shm, int8_t key, uint16_t size);
    int8_t shm_remove(semShm_t *shm);
    int8_t shm_write(semShm_t *shm, void *data, uint16_t size);
    int8_t shm_read(semShm_t *shm, void *data, uint16_t size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // SHARED_DATA_H