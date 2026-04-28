#include "semph.h"
#include "errno.h"
#include "stdio.h"

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
    int val; /* value for SETVAL */
    struct semid_ds *buf; /* buffer for IPC_STAT, IPC_SET */
    unsigned short int *array; /* array for GETALL, SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
};
#endif

int semph_create(SEMPH_DATA *pdata, key_t key, int mode)
{
    pdata->semph_id = semget(key, 1, IPC_CREAT | IPC_EXCL | mode);
    if (pdata->semph_id < 0) {
        if (errno != EEXIST) {
            perror("semget(IPC_CREAT) fail");
            return -1;
        }

        return semph_open(pdata, key, mode);
    }

    printf("key=%d sharem_id=%d\n", pdata->semph_id, key);
    semph_setval(pdata, 1);

    return 0;
}

int semph_open(SEMPH_DATA *pdata, key_t key, int mode)
{
    pdata->semph_id = semget(key, 1, mode);
    if (pdata->semph_id < 0) {
        perror("semget() fail");
        return -2;
    }

    return 0;
}

void semph_remove(SEMPH_DATA *pdata)
{
    union semun unSem;
    semctl(pdata->semph_id, 0, IPC_RMID, unSem);
    pdata->semph_id = -1;
}

bool semph_lock(SEMPH_DATA *pdata)
{
    struct sembuf semb;
    semb.sem_num = 0;
    semb.sem_op = -1;
    semb.sem_flg = SEM_UNDO;
    if (semop(pdata->semph_id, &semb, 1) < 0) {
        if (errno == EAGAIN || errno == EINTR) {
            return false;
        }
        return false;
    }
    return true;
}

void semph_unlock(SEMPH_DATA *pdata)
{
    struct sembuf semb;
    semb.sem_num = 0;
    semb.sem_op = 1;
    semb.sem_flg = SEM_UNDO;
    semop(pdata->semph_id, &semb, 1);
}

void semph_setval(SEMPH_DATA *pdata, int val)
{
    union semun unsem;
    unsem.val = val;
    semctl(pdata->semph_id, 0, SETVAL, unsem);
}

int semph_getval(SEMPH_DATA *pdata)
{
    union semun unsem;
    int val = semctl(pdata->semph_id, 0, SETVAL, unsem);

    return val;
}
