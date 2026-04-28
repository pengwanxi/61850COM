/**
 *   \file ied_modify_cfg.h
 *   \brief 加载前根据装置处理modify
 */
#ifndef _IED_MODIFY_CFG_H_
#define _IED_MODIFY_CFG_H_

/**
 *  \brief 将原始文件处理为更改文件
 *  \param raw_file 原始文件路径
 *  \param cfg_file 更改后文件路径
 *  \return ERR_OK-成功 其他-失败
 */
int ied_modify_cfg(const char *raw_file, const char *cfg_file);

#endif /* _IED_MODIFY_CFG_H_ */
