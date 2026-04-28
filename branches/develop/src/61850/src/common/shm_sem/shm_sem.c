#include "shm_sem.h"

#define SHM_LOG_WARN(fmt, ...)
#define SHM_LOG_ERR(fmt, ...)
#define SHM_LOG_INFO(fmt, ...)

int shm_sem_create(SHM_SEM_DATA *pdata, key_t key, int size, int mode)
{
    if (NULL == pdata)
        return -1;

    int shm_id;
    if ((shm_id = shmget(key, 0, 0)) == -1) {
        perror("shmget");
        /* printf("%s shm_get error\n", __FUNCTION__); */
        SHM_LOG_WARN("shmget error key=%d", key);
    }
    else {
        printf("shm_id=%d", shm_id);
        // 删除共享内存段
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("shmctl");
            printf("%s shmctl error\n", __FUNCTION__);
            SHM_LOG_ERR( "shmctl error key=%d", key);
            return -1;
        }
        else {
            printf("shm_id ctl ok");
        }
    }

    if (sharem_create(&pdata->sharem_data, key, size, mode) < 0) {
        SHM_LOG_ERR( "sharem_create error key=%d", key);
        return -1;
    }

    if (semph_create(&pdata->semph_data, key + 1, mode) < 0) {
        sharem_remove(&pdata->sharem_data);
        SHM_LOG_ERR( "semph_create error key=%d", key + 1);
        return -1;
    }

    SHM_LOG_INFO("ok");
    printf("%s ok\n", __FUNCTION__);
    return 0;
}

int shm_sem_open(SHM_SEM_DATA *pdata, key_t key, int size, int mode)
{
    if (sharem_open(&pdata->sharem_data, key, size, mode) < 0) {
        SHM_LOG_ERR( "sharem_open error");
        return -1;
    }

    if (semph_open(&pdata->semph_data, key + 1, mode) < 0) {
        SHM_LOG_ERR( "semph_open error");
        return -1;
    }

    printf("%s ok\n", __FUNCTION__);
    return 0;
}

void shm_sem_remove(SHM_SEM_DATA *pdata)
{
    sharem_remove(&pdata->sharem_data);
    semph_remove(&pdata->semph_data);
}

void *shm_sem_attach(SHM_SEM_DATA *pdata)
{
    if (pdata->nattach > 0) {
        return pdata->sharem_data.psharem;
    }

    void *p = sharem_attach(&pdata->sharem_data);
    if (NULL != p) {
        pdata->nattach++;
    }

    if (semph_open(&pdata->semph_data, pdata->key + 1, 0) < 0) {
        SHM_LOG_ERR( "semph_open error");
    }

    return p;
}

void shm_sem_deattach(SHM_SEM_DATA *pdata)
{
    sharem_deattach(&pdata->sharem_data);
}

void shm_sem_init_shmvar(SHM_SEM_DATA *pdata)
{
    sharem_init_var(&pdata->sharem_data);
}

bool shm_sem_lock(SHM_SEM_DATA *pdata)
{
    return semph_lock(&pdata->semph_data);
}

void shm_sem_unlock(SHM_SEM_DATA *pdata)
{
    semph_unlock(&pdata->semph_data);
}

void shm_sem_set_semval(SHM_SEM_DATA *pdata, int val)
{
    semph_setval(&pdata->semph_data, val);
}

int shm_sem_get_semval(SHM_SEM_DATA *pdata)
{
    return semph_getval(&pdata->semph_data);
}
