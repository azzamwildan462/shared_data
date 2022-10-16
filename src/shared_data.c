/**
 *
 * @author Okke Hendriks - Tech United Eindhoven
 * @author Azzam Wildan Maulana - IRIS ITS
 *
 * Shared memory creation, sending and receiving.
 * Can be used for inter process (or thread) communication.
 *
 * Uses semaphores to prevent race conditions.
 *
 *
 */

#include "shared_data.h"

/**
 *
 * @brief Create (or open if it already exists) an shared memory segment (shms)
 * and map it to the current process. Always call shmRemove after the
 * process is finished with the shms.
 *
 * @param key:     	An unique key to identify the shms (use ftok(...) to ensure a
 *          	unique key.
 * @param size:    	The size of the shared memory segment.
 * @param *shmid:		Return the obtained shared mem id
 * @param *sem:		Return the obtained semaphore used to guard this mem segment
 * @param *newSegment:	Return if a new segment has been created or attached to an existing segment
 *
 * @return segptr   	- if a shared memory segment is attached ||
 *  	    (void*)-1	- if an error occured
 */
void *shmOpen(int key, int size, int *shmid, sem_t **sem, int *newSegment)
{
#ifdef DEBUG_SHARED_MEM
    printf("key: %d | size: %d\n", key, size);
#endif
    /* Open the shared memory segment - create if necessary */
    if ((*shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | 0666)) == -1)
    {
#ifdef DEBUG_SHARED_MEM
        printf("Shared memory segment exists - opening as client.\n");
#endif
        /* Segment probably already exists - try as a client */
        if ((*shmid = shmget(key, size, 0)) == -1)
        {
            printf("shared_mem.c:shmget(%d, %d, 0)\n", key, size);
            perror("shared_mem.c:shmget()");
            printf("Try running sudo /dev/Okke/scripts/kill_ipcs.sh\nMake sure to run make_all_install."); // Temporary FIX, occurs after a number of force quits of simulator!
            *newSegment = -1;
            return (void *)-1;
        }
        *newSegment = 0;
    }
    else
    {
#ifdef DEBUG_SHARED_MEM
        printf("Shared memory segment does not exist - Creating new shared memory segment (%d, %d).\n", key, size);
#endif
        *newSegment = 1;
    }

    /* Attach (map) the shared memory segment into the current process */
    void *segptr = shmat(*shmid, 0, 0);
    if (segptr == (void *)-1)
    {
        perror("shmat");
        return (void *)-1;
    }

    // Create or open the named semaphore for this sms
    if (openSemForShm(*shmid, sem) == -1)
        return (void *)-1;

    return segptr;
}

/**
 *
 * @brief Mark a shared memory segment for deletion. It will be delete as soon
 * as all processes detached from the memory segment.
 *
 * @param shmid:     	The id of a previously aquired shared memory segment.
 * @param *segptr:		Pointer to the shared memory segment
 *
 * @return -1      - If an error occured ||
 *          0	   - If success
 */
int shmRemove(int shmid, void *segptr)
{
    // Detach shared memory segment
    if (shmdt(segptr) == -1)
    {
        perror("shmdt");
        return -1;
    }

#ifdef DEBUG_SHARED_MEM
    printf("################### %d END #########################\n", getpid());
    printf("Shared memory segment detached.\n");
#endif

    /* Obtain the shm struct */
    struct shmid_ds shmInfo;
    if (shmctl(shmid, IPC_STAT, &shmInfo) == -1)
        perror("shmctl (obtaining key of shmid)");

    int nattach = shmInfo.shm_nattch;

    //#ifdef DEBUG_SHARED_MEM
    printf("Number of processes still attached to shared mem segment: %d\n", nattach);
    //#endif

    if (nattach == 0)
    {
        // Close the used semaphore
        closeSemForShm(shmid);

        //#ifdef DEBUG_SHARED_MEM
        printf("No processes attached to shared mem segment, removing...\n");
        //#endif

        // Remove shared mem segment
        shmctl(shmid, IPC_RMID, 0);
    }

    return 0;
}

/**
 *
 * @brief Obtain a lock on a semaphore, either in a blocking or non blocking manner.
 *
 * @param shmid:     	The id of a previously aquired shared memory segment.
 * @param blocking:    Indicate blocking or non-blocking mode
 *
 * @return -2      - If a lock was not obtained (when in non-blocking mode) ||
 * 			-1 		- if sem_wait returned an error ||
 *			0		- If succes
 */
int lockSemaphore(sem_t *sem, int blocking)
{
#ifdef DEBUG_SHARED_MEM
    printf("Sem Value: %d\n", getSemVal(sem));
#endif

    if (blocking)
    {
        // Lock the semaphore. BLOCKS until semaphore can be taken!
        if (sem_wait(sem) == -1)
        {
            perror("sem_wait");
            return -1;
        }
    }
    else if (sem_trywait(sem) == -1)
    {
        // Lock could not be taken because another process holds it
        // Skip the write because this in non-waiting mode
        return -2;
    }
    return 0;
}

/**
 *
 * @brief Write an object to a shared memory segment, using the corressponding semaphore to
 * prevent race conditions
 *
 * @param *object		The object to write to the shm
 * @param shmid:     	The id of a previously aquired shared memory segment.
 * @param *segptr:		Pointer to the shared memory segment
 * @param *sem:		Pointer to the previously obtained semaphore that goes the this segptr
 * @param blocking:    Indicate blocking or non-blocking mode
 * @param lock:       Indicate if before writing the semaphore should be lock, used in case of custom critical read write operation (i.c.w. unlock parameter of read function)
 *
 * @return -1 		- If an error occured ||
 *			0		- If succes
 */
int shmWrite(void *object, int size, int shmid, void *segptr, sem_t *sem, int blocking, int lock)
{
#ifdef DEBUG_SHARED_MEM
    printf("shmWrite, blocking: %d, locking: %d\n", blocking, lock);
#endif
    if (lock)
    {
        // Lock semaphore, either blocking or non-blocking
        if (lockSemaphore(sem, blocking) == -1)
            return -1;
    }

// At this momement a lock is obtained, so we can write the memory segment
#ifdef DEBUG_SHARED_MEM
    printf("Writing, memcpy to address: %lu | size: %d...\n", (unsigned long int)segptr, size);
#endif
    memcpy(segptr, object, size);

    // After this critical section we release the lock
    sem_post(sem);
    return 0;
}

/**
 *
 * @brief Read an object from a shared memory segment, using the corressponding semaphore to
 * prevent race conditions
 *
 * @param *object		The object to read from the shm
 * @param shmid:     	The id of a previously aquired shared memory segment.
 * @param *segptr:		Pointer to the shared memory segment
 * @param *sem:		Pointer to the previously obtained semaphore that goes with the segptr
 * @param blocking:    Indicate blocking or non-blocking mode
 * @param unlock:      Indicate if after locking and writing the semaphore should be unlocked, used in case of custom critical read write operation
 *
 * @return -1 		- If an error occured ||
 *			0		- If succes
 */
int shmRead(void *object, int size, int shmid, void *segptr, sem_t *sem, int blocking, int unlock)
{
#ifdef DEBUG_SHARED_MEM
    printf("shmRead, blocking: %d, unlocking: %d\n", blocking, unlock);
#endif
    // Lock semaphore, either blocking or non-blocking
    if (lockSemaphore(sem, blocking) == -1)
        return -1;

#ifdef DEBUG_SHARED_MEM
    printf("Reading, mempcy from address: %lu | size: %d...\n", (unsigned long int)segptr, size);
#endif

    // At this momement a lock is obtained, so we can read the memory segment
    memcpy(object, segptr, size);

    if (unlock)
    {
        // After this critical section we release the lock
        sem_post(sem);
    }
    return 0;
}

/**
 *
 * @brief Read an object from a shared memory segment, execute a critical section function that acts on this object
 * and write the object back to shared memory. Using the corressponding semaphore to prevent race conditions.
 *
 * @param *object				The object to read from the shm
 * @param shmid:     			The id of a previously aquired shared memory segment.
 * @param *segptr:				Pointer to the shared memory segment
 * @param *sem:				Pointer to the previously obtained semaphore that goes with the segptr
 * @param blocking:    		Indicate blocking or non-blocking mode
 * @param (*f)(void* object):  Pointer to a function that is execute as the critical section
 *
 * 						Critical section function example:
 *
 * 						int criticalFunction(void* object){
 * 							yourType foo = (yourType)object;
 * 							foo.value = foo.value + 1;
 * 						}
 *
 *
 * @return -1 		- If an error occured ||
 *			0		- If succes
 */
int shmReadWrite(void *object, int size, int shmid, void *segptr, sem_t *sem, int blocking, int (*f)(void *object))
{
    // Lock semaphore, either blocking or non-blocking
    if (lockSemaphore(sem, blocking) == -1)
        return -1;

    // At this momement a lock is obtained, so we can read the shared memory segment
    memcpy(object, segptr, size);

    // The critical section function is called with the object
    f(object);

    // And the object is written back to the shared memory segement
    memcpy(segptr, object, size);

    // After the critical section we release the lock
    sem_post(sem);
    return 0;
}

/**
 *
 * @brief Attaches to a semaphore, creates it if it does not yet exist.
 *
 * @param shmid:     	The id of a previously aquired shared memory segment.
 * @param **sem:		Returns the sem_t *
 *
 * @return -1 		- If an error occured ||
 *			0		- If succes
 */
int openSemForShm(int shmid, sem_t **sem)
{
    struct shmid_ds shmInfo;
    int key;
    char name[NAME_MAX_BYTES];

    /* Obtain the key used for this shared memory segment in order to generate a name for the named semaphore*/
    if (shmctl(shmid, IPC_STAT, &shmInfo) == -1)
    {
        perror("shmctl (obtaining key of shmid)");
        return -1;
    }

    key = shmInfo.shm_perm.__key;
    sprintf(name, "/%d", key);

#ifdef DEBUG_SHARED_MEM
    printf("Obtained key: %d of shmid: %d --> name: %s\n", key, shmid, name);
#endif

    // Try to open the semaphore, initialize to 0 (locked)
    *sem = sem_open(name, O_CREAT, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH, 0);
    if (*sem == SEM_FAILED)
    {
        perror("sem_open");
        return -1;
    }

#ifdef DEBUG_SHARED_MEM
    printf("Attached to semaphore: %s, value: %d\n\n", name, getSemVal(*sem));
#endif

    return 0;
}

/**
 *
 * @brief Detaches from semaphore, by shared memory id
 *
 * @param shmid:     	The id of a previously aquired shared memory segment.
 *
 * @return -1 		- If an error occured ||
 *			0		- If succes
 */
int closeSemForShm(int shmid)
{
    struct shmid_ds shmInfo;
    int key;
    char name[NAME_MAX_BYTES];

    /* Obtain the key used for this shared memory segment to get name of the semaphore */
    if (shmctl(shmid, IPC_STAT, &shmInfo) == -1)
    {
        perror("shmctl (obtaining key of shmid)");
        return -1;
    }

    key = shmInfo.shm_perm.__key;
    sprintf(name, "/%d", key);

    // Perform the unlinking
    sem_unlink(name);
    return 0;
}

/**
 *
 * @brief Return the number of attached processes to a semaphore
 *
 * @param *sem:	The pointer to the semaphore
 *
 * @return -1 		- If an error occured ||
 *			>= 0	- The number of attached processes
 */
int getSemVal(sem_t *sem)
{
    int sem_value;
    if (sem_getvalue(sem, &sem_value) == -1)
    {
        perror("sem_getvalue");
        return -1;
    }
    return sem_value;
}

/**
 *
 * @brief Init shared memory for IPC (Inter Process Communication)
 * with a fixed size and an unique key
 *
 * @param *shm: Pointer to shared memory struct (semShm_t)
 * @param key: unique key that can be obtained from ftok()
 * @param size: shared memory size (in bytes)
 *
 * @return -1 if an error occured ||
 * 			0 if success
 * */
int8_t init_shared_mem(semShm_t *shm, int8_t key, uint16_t size)
{
    shm->key = key;
    if ((shm->segptr = shmOpen(shm->key, size, &(shm->shmid), &(shm->sem), &(shm->createdSegment))) == (void *)-1)
    {
        printf("FATAL ERROR, failed to open shared memory segment, exiting.\n Use cmnds ipcs and ipcrm to remove shared memory segment with permissions 666.\nOr reboot.\n");
        return -1;
        exit(1);
    }
    else
    {
        sem_post(shm->sem);
    }

    shm->w_blocking_flag = 1;
    shm->w_lock_flag = 0;
    shm->r_blocking_flag = 0;
    shm->r_unlock_flag = 0;

    return 0;
}

/**
 *
 * @brief Mark a shared memory segment for deletion. It will be delete as soon
 * as all processes detached from the memory segment.
 *
 * @param *shm: 		Pointer to shared memory struct (semShm_t)
 *
 * @return -1      - If an error occured ||
 *          0	   - If success
 */
int8_t shm_remove(semShm_t *shm)
{
    shmRemove(shm->shmid, shm->segptr);
}

/**
 *
 * @brief Write an object to a shared memory segment, using the corressponding semaphore to
 * prevent race conditions
 *
 * @param *shm: 		Pointer to shared memory struct (semShm_t)
 * @param *data:        Data to be written to shared memory
 * @param size:         How many data to write (in Bytes)
 *
 * @return -1 		- If an error occured ||
 *			0		- If succes
 */
int8_t shm_write(semShm_t *shm, void *data, uint16_t size)
{
    return shmWrite(data, size, shm->shmid, shm->segptr, shm->sem, shm->w_blocking_flag, shm->w_lock_flag);
}

/**
 *
 * @brief Read an object from a shared memory segment, using the corressponding semaphore to
 * prevent race conditions
 *
 * @param *shm: 		Pointer to shared memory struct (semShm_t)
 * @param *data:        Pointer to buffer to save the data that read from shared memory
 * @param size:         How many data to read (in Bytes)
 *
 * @return -1 		- If an error occured ||
 *			0		- If succes
 */
int8_t shm_read(semShm_t *shm, void *data, uint16_t size)
{
    return shmRead(data, size, shm->shmid, shm->segptr, shm->sem, shm->r_blocking_flag, shm->r_unlock_flag);
}
