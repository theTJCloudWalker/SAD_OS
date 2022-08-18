#ifndef __FS_GROUP_H
#define __FS_GROUP_H
#include "stdint.h"
#include "ide.h"

#define MAX_BLOCK_PER_GROUP 100 

/*空闲盘块号数组*/
struct free_group
{
    uint32_t inside_pointer;//当前数组下标
    uint32_t free_array[MAX_BLOCK_PER_GROUP];//100个空闲盘块地址

    uint8_t  pad[108];//凑够512字节,108=512-4-400
} __attribute__ ((packed));


/*目前还没有改动，需要在partition里面把block_bitmap换成一个freegroup*/

/*fs.c/mount_partition中把block_bitmap换为freegroup*/

/*还需要在超级块中更改block_bitmap信息为freegroup*/
/*还需要将其他操作里对应的block_bitmap替换掉*/

extern struct partition* cur_part;
void group_init(struct free_group* fgroup);//初始化一个free_group
int group_allocate_default(struct free_group* fgroup);//分配一个盘块并返回盘块地址
int group_allocate(struct partition* part,struct free_group* fgroup);//分配一个盘块并返回盘块地址
int group_recycle_default(struct free_group* fgroup,uint32_t addr);//回收一个盘块
int group_recycle(struct partition* part,struct free_group* fgroup,uint32_t addr);//回收一个盘块
int group_sync_default(struct free_group* fgroup);//把内存中的盘块写入磁盘中固定位置
int group_sync(struct partition* part,struct free_group* fgroup);//把内存中的盘块写入磁盘中固定位置
int group_load_default(struct free_group* fgroup);//从磁盘中固定位置获取空闲组到内存
int group_load(struct partition* part,struct free_group* fgroup);//从磁盘中固定位置获取空闲组到内存
void group_disk_init_default(struct free_group* fgroup,uint32_t file_begin_addr,uint32_t total_size);//进行磁盘初始化,应被fs.c/partition_format()调用
void group_disk_init(struct partition* part,struct free_group* fgroup,uint32_t group_default_addr,uint32_t file_begin_addr,uint32_t total_size);//进行磁盘初始化,应被fs.c/partition_format()调用
#endif


