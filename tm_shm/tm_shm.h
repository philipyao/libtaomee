#ifndef __TM_SHM_H__
#define __TM_SHM_H__

#include <sys/types.h>

#define FILE_PATH_LEN 4096

struct tm_shm_mgr_t {
	/*! 0: ��ʾ��Ч */
	key_t	key;
	int		shmid;
	size_t	size;
	void	*shm;
};


//-----------------------------------------------------------
// helper functions
//-----------------------------------------------------------

/* @brief ��ȷ������ú����� key ��ȫϵͳ��Χ�ǲ��ظ��� */
int do_tm_shm_create_init(key_t key, size_t size, void *init_buf, size_t init_buf_len);




//-----------------------------------------------------------
// exported interface functions
//-----------------------------------------------------------
/*
 * @brief ����һ����ʼ���� 0 �� shm segment
 * ע��: �������� IPC_PRIVATE ���͵� shm (key������0);
 */
int tm_shm_create(key_t key, size_t size);


/*
 * @brief ע��:
 * ��� init_buf_len > size,
 * 		����� init_buf ��ǰ init_buf_len�ֽ� ��ʼ�����´����� shm ��;
 * ע��: �������� IPC_PRIVATE ���͵� shm (key������0);
 */
int tm_shm_create_init(key_t key, size_t size, void *init_buf, size_t init_buf_len);

/*
 * @brief ����һ����ʼ���� 0 �� shm segment
 * ����system-wide������ͬkey��shm, �򷵻ش���
 */
int tm_shm_create_new(key_t key, size_t size);

/*
 * @brief ע��:
 * 1. ��� init_buf_len > size,
 * 		����� init_buf ��ǰ init_buf_len�ֽ� ��ʼ�����´����� shm ��;
 *
 * 2. ��� key == IPC_PRIVATE (0); ���Ǵ���˽��shm
 * 		(���������޷�attach, ��ֻ��ͨ��shmid��ɾ��)
 *
 * 3. ����system-wide������ͬkey��shm, �򷵻ش���
 */
int tm_shm_create_new_init(key_t key, size_t size, void *init_buf, size_t init_buf_len);

/**
 * @brief attach �� new_shmid �� shm ��, ͬʱ detach old_shmid �� shm;
 * ע��: ��һ�� update_attach ��ʱ��, Ҫ�� shm_mgr->key ��ֵ;
 */
int tm_shm_update_attach(struct tm_shm_mgr_t *shm_mgr);

/**
 * @brief ���� system-wide �Ƿ��� key �� shm
 * @return 0: key �� shm ��û����; 1: key �� shm �Ѿ�����; -1: ����
 */
int tm_shm_test_shm_exist(key_t key);

/*added by singku for mmap xml*/
typedef int (*call_back_func_t)(void* addr, const char* path);

typedef struct cfg_info{
    size_t  mem_size;   //�������ļ���ӦҪ������ڴ��С(ʵ�ʴ�С��2����mem_size����һ�����ڸ���)
    size_t  meta_size;  //�ڴ���ÿ������(Ԫ����)�Ĵ�С
    char    *cfg_mmap_file_path;  //�����ļ���Ӧ��mmap�ļ���·��(spirit, weapon, item...)
    char    *cfg_file_path;  //�����ļ���·��
    call_back_func_t call_back; //���������ļ��Ļص��ӿ�,���û��������ʹ������ڴ�.���뷵��0��ʾ�ɹ�-1��ʾʧ��
    char    *addr;      //mmap���뵽���ڴ��ַ
} cfg_info_t;

//==added for init on mmap
/**
 * @brief ��mmap�ϼ������ñ�
 * @param c_info: �����ļ���Ϣ��
 * @return 0  �ɹ� -1ʧ��.
 */
int init_cfg_on_mmap(cfg_info_t *c_info);

/**
 * @brief ��mmap���ͷ����ñ�.
 */
void destroy_cfg_from_mmap(cfg_info_t* c_info);

/**
 * @brief ��mmap�ϸ������ñ�
 */
int update_cfg_on_mmap(cfg_info_t *c_info);

int attach_cfg_to_mmap(cfg_info_t *c_info);
void detach_cfg_from_mmap(cfg_info_t *c_info);

/**/
#endif /* __TM_SHM_H__ */
