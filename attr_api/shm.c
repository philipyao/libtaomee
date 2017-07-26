#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "shm.h"




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
int get_shm(key_t key, size_t size, int create_nonexist, int init_exist, void *init_buf, size_t init_buf_len)
{
	void *shm = 0;
	int init_size = 0;
	int exist = 0;
	int elder_shmid = 0;
	size_t elder_size = 0;

	if (key == 0) {
		printf("error - forbid to create IPC_PRIVATE shm\n");
		return -1;
	}

	/* ���Ի�ȡ֮ǰ�Ѿ�����ͬ��key����������shm */
	get_shm_info(key, &exist, &elder_shmid, &elder_size);

	if (exist == get_shm_info_nonexist) {
		if (create_nonexist == create_noexist_no) {
			return -1;
		} else {
			/* key �� shm ֮ǰ������, ����´���һ�� */
			return create_shm_init(key, size, init_buf, init_buf_len);
		}
	}

	/* exist == get_shm_info_exist */
	if (size != elder_size) {
		/* key �� shm ֮ǰ����, ����С������Ҫ��, �򷵻ش��� */
		printf("error - cannot create newsize=%zd against elder_size=%zd\n",
				size, elder_size);
		return -1;
	}

	/* key �� shm ֮ǰ����, �Ҵ�С����Ҫ�� */

	/* Ҫ��Ҫ��ʼ���Ѵ��ڵ�shm */
	if (init_exist == init_exist_no) return 0;

	/* Ҫ���ʼ���Ѵ��ڵ�shm */
	shm = shmat(elder_shmid, 0, 0);
	if (shm == (void *)-1) {
		perror("error - attach elder shm failed: ");
		return -1;
	}

	if (init_buf) {
		init_size = (size < init_buf_len) ? size : init_buf_len;
		memcpy(shm, init_buf, init_size);
	} else {
		memset(shm, 0, size);
	}
	shmdt(shm);

	return 0;
}

/*
 * @brief ����һ����ʼ���� 0 �� shm, create_xxx ����������, �����´���,
 * ��: ����system-wide������ͬkey��shm, �򷵻ش���
 */
int create_shm(key_t key, size_t size)
{
	return create_shm_init(key, size, 0, 0);
}

/*
 * @brief ����һ����ʼ���� 0 �� shm, create_xxx ����������, �����´���,
 * ���� init_buf ����ʼ���´����� shm;
 * ע��:
 * 1. ������Ҫ����ȷ��ϵͳ��Χ��û�� key �� shm ����;
 * 2. ��� init_buf_len > size,
 * 		����� init_buf ��ǰ init_buf_len�ֽ� ��ʼ�����´����� shm ��;
 *
 * 3. ��� key == IPC_PRIVATE (0); ���Ǵ���˽��shm
 * 		(���������޷�attach, ��ֻ��ͨ��shmid��ɾ��)
 *
 * 4. ����system-wide������ͬkey��shm, �򷵻ش���
 */
int create_shm_init(key_t key, size_t size, void *init_buf, size_t init_buf_len)
{
	int shmid = -1;
	void *shm = 0;
	size_t init_size = 0;

	shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | 0600);
	/* 0600: S_IRUSR | S_IWUSR: �ɶ���д */
	if (shmid == -1) {
		fprintf(stderr, "error - do create shm failed, err(%d): %s\n",
				errno, strerror(errno));
		return -1;
	}

	shm = shmat(shmid, 0, 0);
	if (shm == (void *)-1) {
		fprintf(stderr, "error - attach shm failed, err(%d): %s\n",
				errno, strerror(errno));
		return -1;
	}

	if (init_buf) {
		init_size = (size < init_buf_len) ? size : init_buf_len;
		memcpy(shm, init_buf, init_size);
	}

	shmdt(shm);

	return 0;
}

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
int update_shm_attach(struct shm_mgr_t *shm_mgr)
{
	int shmid = 0;
	size_t size = 0;
	int exist = 0;
	void *shm = NULL;

	/* �˴����ܼ�� shm_mgr->size, ��Ϊ��1�� attach ʱ, shm_mgr->sizeΪ 0 */
	if (!shm_mgr || !shm_mgr->key) {
		printf("error - update shm: shm_mgr=%p, key=0x%x\n",
				shm_mgr, shm_mgr ? shm_mgr->key : 0);
		return -1;
	}

	get_shm_info(shm_mgr->key, &exist, &shmid, &size);
	if (exist == get_shm_info_nonexist) {
		printf("error - update shm: nonexist-shm, key=0x%x\n", shm_mgr->key);
		return -1;
	}

	if (shm_mgr->shmid == shmid) {
		if (shm_mgr->size == size) {
			/* key �� shm �Ĳ��� shmid �� size ��û�б仯, ���ֱ�ӷ��سɹ� */
			return 0;
		} else { /* key �� shm �Ĳ���(size)�б�, ����ʧ�� */
			printf("error - update same shmid(%d) but diff shm_segsz"
					"key=%#x, shm_mgr->size=%zd, exist_size=%zd\n",
					shm_mgr->shmid, shm_mgr->key, shm_mgr->size, size);
			return -1;
		}
	}

	/* �� shmid (��Ϊ���� key): ������³�ʼ�� shm_info */
	shm = shmat(shmid, 0, 0);
	if (shm == (void *)-1) {
		printf("error - update new shmid(%d) of key(%#x), but shmat failed\n",
				shmid, shm_mgr->key);
		return -1;
	}

	/*
	 * ����һ�� update_attach ʱ,
	 * ���� shm_mgr->shm == 0, �ᵼ�� shmdt() ���� -1,
	 * ���ⲻ�Ǵ���, ��˺��Դ˴���
	 */
	shmdt(shm_mgr->shm);

	shm_mgr->shmid = shmid;
	shm_mgr->size = size;
	shm_mgr->shm = shm;

	return 0;
}

/**
 * @brief ��ȡ system-wide �� key �� shm ����Ϣ
 *
 * @exist:
 * 		0: key �� shm ��û����;
 * 		1: key �� shm �Ѿ�����;
 *
 * @shmid: ���� shm ����, �� shmid ָ�벻Ϊ NULL ʱ, �洢�ѽ����� shm �� id;
 * @size: ���� shm ����, �� size ָ�벻Ϊ NULL ʱ, �洢�ѽ����� shm �� size;
 *
 * @return -1: ����, 0: ����������� (������� exist ��);
 */
int get_shm_info(key_t key, int *exist, int *shmid, size_t *size)
{
	struct shmid_ds shm_stat;

	/* ���Ի�ȡ֮ǰ�Ѿ�����ͬ��key����������shm */
	int id = shmget(key, 0, 0);
	if (id == -1) {
		/* �޷���ȡshmid, shm������ */
		*exist = get_shm_info_nonexist;
		goto out;
	}

	if (shmctl(id, IPC_STAT, &shm_stat) == -1) {
		/* �޷���ȡshm_stat, shmҲ������ */
		*exist = get_shm_info_nonexist;
		goto out;
	}

	*exist = get_shm_info_exist;

	/* ��� shm �Ѵ���, �ڻ�ȡһЩ��Ϣ */
	if (shmid) {
		*shmid = id;
	}

	if (size) {
		*size = shm_stat.shm_segsz;
	}

out:
	return 0;
}


/*
 * -----------------------------------------------------------
 * inline interface functions
 * -----------------------------------------------------------
 */

inline void init_shm_mgr(struct shm_mgr_t *mgr, key_t key)
{
	mgr->key = key;
	mgr->shmid = -1;
	mgr->size = 0;
	mgr->shm = NULL;
}

inline int get_shm_nocreate_noinitexist(key_t key, size_t size)
{
	return get_shm(key, size, create_noexist_no, init_exist_no, NULL, 0);
}

inline int get_shm_nocreate_initexist(key_t key, size_t size, void *init_buf, size_t init_buf_len)
{
	return get_shm(key, size, create_noexist_no, init_exist_yes, init_buf, init_buf_len);
}

inline int get_shm_create_noinitexist(key_t key, size_t size, void *init_buf, size_t init_buf_len)
{
	return get_shm(key, size, create_noexist_yes, init_exist_no, init_buf, init_buf_len);
}

inline int get_shm_create_initexist(key_t key, size_t size, void *init_buf, size_t init_buf_len)
{
	return get_shm(key, size, create_noexist_yes, init_exist_yes, init_buf, init_buf_len);
}

inline int get_shm_nocreate(key_t key, size_t size, int init_exist, void *init_buf, size_t init_buf_len)
{
	return get_shm(key, size, create_noexist_no, init_exist, init_buf, init_buf_len);
}

inline int get_shm_create(key_t key, size_t size, int init_exist, void *init_buf, size_t init_buf_len)
{
	return get_shm(key, size, create_noexist_yes, init_exist, init_buf, init_buf_len);
}

inline int get_shm_noinitexist(key_t key, size_t size, int create_exist, void *init_buf, size_t init_buf_len)
{
	return get_shm(key, size, create_exist, init_exist_no, init_buf, init_buf_len);
}

inline int get_shm_initexist(key_t key, size_t size, int create_exist, void *init_buf, size_t init_buf_len)
{
	return get_shm(key, size, create_exist, init_exist_yes, init_buf, init_buf_len);
}
