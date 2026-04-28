/**
 *   \file model_data.h
 *   \brief 模板数据
 */
#ifndef _MODEL_DATA_H_
#define _MODEL_DATA_H_


/* 模板数据 */
typedef struct _MODEL_DATA
{
    char *name;
    char *proto;

    void *priv_data;
}MODEL_DATA;

MODEL_DATA *match_model_data(char *name);
void *model_private_data(char *name);

int model_data_register(MODEL_DATA *pmodel);
int model_data_init(MODEL_DATA *pmodel, char *name);
int model_data_exit(MODEL_DATA *pmodel);

#endif /* _MODEL_DATA_H_ */
