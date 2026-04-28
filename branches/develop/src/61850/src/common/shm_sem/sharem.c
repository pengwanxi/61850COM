#include "sharem.h"
#include "errno.h"
#include "string.h"

int sharem_create(SHAREM_DATA *pdata, key_t key, int size, int mode)
{
    pdata->sharem_id = shmget(key, size, IPC_CREAT | IPC_EXCL | mode);
    if (pdata->sharem_id < 0) {
        if (errno != EEXIST) {
            perror("shmget(IPC_CREAT) fail");
            return -1;
        }

        return sharem_open(pdata, key, size, mode);
    }

    printf("key=%d size=%d sharem_id=%d\n", pdata->sharem_id, key, size);
    sharem_init_var(pdata);
    pdata->size = size;
    return 0;
}

int sharem_open(SHAREM_DATA *pdata, key_t key, int size, int mode)
{
    pdata->sharem_id = shmget(key, size, mode);
    if (pdata->sharem_id < 0) {
		printf("key=%d size=%d mode=%d\n", key, size, mode);
        perror("shmget() fail");
        return -2;
    }

    return 0;
}

void sharem_remove(SHAREM_DATA *pdata)
{
    if (shmctl(pdata->sharem_id, IPC_RMID, NULL) < 0) {
        perror("shmctl(IPC_RMID) fail");
        return;
    }
}

void *sharem_attach(SHAREM_DATA *pdata)
{
    pdata->psharem = (char *)shmat(pdata->sharem_id, NULL, 0);
    printf(" shmid = %d sharem = %p \n", pdata->sharem_id, pdata->psharem);
    if ((long)pdata->psharem == -1) {
        perror("shmat fail");
        printf("get sharememory error\n");
		return NULL;
    }

    return pdata->psharem;
}

void sharem_deattach(SHAREM_DATA *pdata)
{
    if (pdata->psharem == NULL )
        return;

    if ((shmdt(pdata->psharem))) {
        perror("shmdt fail");
        return;
    }

    pdata->psharem = NULL;
}

void sharem_init_var(SHAREM_DATA *pdata)
{
    memset(pdata->psharem, 0, pdata->size);
}
