#ifndef __FS_INODE_H
#define __FS_INODE_H
#include "stdint.h"
#include "list.h"
#include "ide.h"


#define SECTOR_FILE_SIZE 508

//TODO:文件存储的一页的文件结构，文件存储变成链式，最后4字节为下一个逻辑区块号！
struct sector_file{
   uint8_t  pad[SECTOR_FILE_SIZE];//存储的数据
   uint32_t next_lba;//下一页的逻辑区块号
}__attribute__ ((packed));





/* inode结构 */
struct inode {
   uint32_t i_no;    // inode编号

/* 当此inode是文件时,i_size是指文件大小,
若此inode是目录,i_size是指该目录下所有目录项大小之和*/
   uint32_t i_size;

   uint32_t i_open_cnts;   // 记录此文件被打开的次数
   bool write_deny;	   // 写文件不能并行,进程写文件前检查此标识




/* i_sectors[0-11]是直接块, i_sectors[12]用来存储一级间接块指针 */
   //uint32_t i_sectors[13];

//TODO:存储了首个逻辑区块号和总扇区大小
/*进行链式的修改，存储首个逻辑区块号和总共的扇区大小*/
   uint32_t first_sector_lba;//首个逻辑区块号
   uint32_t total_sector_num;//总共的扇区大小


   struct list_elem inode_tag;
};

struct inode* inode_open(struct partition* part, uint32_t inode_no);
void inode_sync(struct partition* part, struct inode* inode, void* io_buf);
void inode_init(uint32_t inode_no, struct inode* new_inode);
void inode_close(struct inode* inode);
void inode_release(struct partition* part, uint32_t inode_no);
void inode_delete(struct partition* part, uint32_t inode_no, void* io_buf);
#endif
