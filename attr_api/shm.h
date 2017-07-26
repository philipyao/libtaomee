#ifndef __SHM_H__
#define __SHM_H__

#include <sys/types.h>


/*
 * -----------------------------------------------------------
 * exported enums
 * -----------------------------------------------------------
 */
enum get_shm_info_result_e {
	/* shm ��û���� */
	get_shm_info_nonexist		= 0,
	/* shm �Ѿ����� (�Ѿ���ȡ��shmid, size ����Ϣ) */
	get_shm_info_exist			= 1,
};

enum create_noexist_opt_e {
	create_noexist_no	= 0,
	create_noexist_yes	= 1,
};

enum init_exist_opt_e {
	init_exist_no	= 0,
	init_exist_yes	= 1,
};


/*
 * -----------------------------------------------------------
 * exported structers
 * -----------------------------------------------------------
 */
struct shm_mgr_t {
	/*! 0: ��ʾ��Ч */
	key_t	key;
	int		shmid;
	size_t	size;
	void	*shm;
};


/*
 * -----------------------------------------------------------
 * helper functions
 * -----------------------------------------------------------
 */



/*
 * -----------------------------------------------------------
 * exported interface
 * -----------------------------------------------------------
 */

/*
 * @brief ���һ����ʼ���� 0 �� shm segment (Ҳ���Ѿ���������Ҫ����), ����init_buf��ʼ����;
 * ע��:
 * (1) ��� init_buf_len > size, ����� init_buf ��ǰ init_buf_len�ֽ� ��ʼ�����´����� shm ��;
 * (2) �������� IPC_PRIVATE ���͵� shm (key������0);
 *
 * @create_nonexist �� key �� shm ������ʱ, �Ǳ���(0), ���Ǵ����µ�(1);
 * @init_exist �Ƿ��ʼ���Ѿ����ڵ� shm: 0: ����ʼ��, 1: ��ʼ��;
 * @return -1: ʧ��, 0: �ɹ�
 */
int get_shm(key_t key, size_t size, int create_nonexist, int init_exist, void *init_buf, size_t init_buf_len);

/*
 * @brief ����һ����ʼ���� 0 �� shm, create_xxx ����������, �����´���,
 * ��: ����system-wide������ͬkey��shm, �򷵻ش���
 */
int create_shm(key_t key, size_t size);

/*
 * @brief ����һ����ʼ���� 0 �� shm, create_xxx ����������, �����´���,
 * ���� init_buf ����ʼ���´����� shm;
 * ע��:
 * 1. ��� init_buf_len > size,
 * 		����� init_buf ��ǰ init_buf_len�ֽ� ��ʼ�����´����� shm ��;
 *
 * 2. ��� key == IPC_PRIVATE (0); ���Ǵ���˽��shm
 * 		(���������޷�attach, ��ֻ��ͨ��shmid��ɾ��)
 *
 * 3. ����system-wide������ͬkey��shm, �򷵻ش���
 */
int create_shm_init(key_t key, size_t size, void *init_buf, size_t init_buf_len);

/**
 * @brief attach �� new_shmid �� shm ��, ͬʱ detach old_shmid �� shm;
 * ��:
 * (1) �� key ��ͬʱ, ��������: shmid, size, shm ��������key����һ��,
 * 		�������κ�һ�������� key �� shm_info �еĲ���ͬ, �� update ʧ��;
 * (2) ���� key ����ͬʱ, shmid ͨ���Ͳ�ͬ, ��ʱ�ͻ�� size, shm ��ͬ������key����һ��;
 * 	   
 * ע��: ���ڲ��������� IPC_PRIVATE ���͵� shm,
 * 		 ���Ե�һ�� update_attach ��ʱ�� (��ʱ: mgr->key == 0), Ҫ�� shm_mgr->key ��ֵ;
 *
 * @return -1: ʧ��, 0: �ɹ�
 */
int update_shm_attach(struct shm_mgr_t *shm_mgr);

/**
 * @brief ��ȡ system-wide �� key �� shm ����Ϣ
 *
 * @return -1: ����, 0: ����������� (������� exist ��);
 *
 * @exist:
 * 		0: key �� shm ��û����;
 * 		1: key �� shm �Ѿ�����;
 *
 * @elder_shmid: ���� shm ����, �� elder_shmid ָ�벻Ϊ NULL ʱ, �洢�ѽ����� shm id;
 * @elder_size: ���� shm ����, �� elder_size ָ�벻Ϊ NULL ʱ, �洢�ѽ����� shm ��С;
 */
int get_shm_info(key_t key, int *exist, int *shmid, size_t *size);


/*
 * -----------------------------------------------------------
 * inline interface functions
 * -----------------------------------------------------------
 */

inline void init_shm_mgr(struct shm_mgr_t *mgr, key_t key);
inline int get_shm_nocreate_noinitexist(key_t key, size_t size);
inline int get_shm_nocreate_initexist(key_t key, size_t size, void *init_buf, size_t init_buf_len);
inline int get_shm_create_noinitexist(key_t key, size_t size, void *init_buf, size_t init_buf_len);
inline int get_shm_create_initexist(key_t key, size_t size, void *init_buf, size_t init_buf_len);
inline int get_shm_nocreate(key_t key, size_t size, int init_exist, void *init_buf, size_t init_buf_len);
inline int get_shm_create(key_t key, size_t size, int init_exist, void *init_buf, size_t init_buf_len);
inline int get_shm_noinitexist(key_t key, size_t size, int create_exist, void *init_buf, size_t init_buf_len);
inline int get_shm_initexist(key_t key, size_t size, int create_exist, void *init_buf, size_t init_buf_len);

#endif /* __SHM_H__ */
