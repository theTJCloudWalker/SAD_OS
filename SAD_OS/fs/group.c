#include"group.h"
#include"string.h"
//#include"ide.h"
#include "super_block.h"



/*初始化一个free_group*/
void group_init(struct free_group* fgroup){

    fgroup->inside_pointer=-1;//初始下标为-1代表没有空闲块
    memset(fgroup->free_array,0,sizeof(uint32_t)*MAX_BLOCK_PER_GROUP);//把空闲块地址全部设置为0
    //fgroup->next_group=0;//下个组地址为0，代表没有下个组了
}


/*把内存中的空闲组写入磁盘中固定位置*/
int group_sync_default(struct free_group* fgroup){


    uint32_t fixed_lba=cur_part->sb->block_group_lba;//磁盘固定位置

    ide_write(cur_part->my_disk,fixed_lba,fgroup,1);

}

/*把内存中的空闲组写入磁盘中固定位置*/
int group_sync(struct partition* part,struct free_group* fgroup){

    uint32_t fixed_lba=cur_part->sb->block_group_lba;//磁盘固定位置

    ide_write(part->my_disk,fixed_lba,fgroup,1);

}


/*从磁盘中固定位置获取空闲组到内存*/
int group_load_default(struct free_group* fgroup){



    uint32_t fixed_lba=cur_part->sb->block_group_lba;//磁盘固定位置
    ide_read(cur_part->my_disk,fixed_lba,fgroup,1);

}

/*从磁盘中固定位置获取空闲组到内存*/
int group_load(struct partition* part,struct free_group* fgroup){



    uint32_t fixed_lba=cur_part->sb->block_group_lba;//磁盘固定位置
    ide_read(part->my_disk,fixed_lba,fgroup,1);

}


/*分配一个盘块并返回盘块地址*/
int group_allocate_default(struct free_group* fgroup){
    
    /*没有可分配的盘块了*/
    if(fgroup->free_array[0]==0&&fgroup->inside_pointer==-1){
        return -1;
    }

    /*返回的盘块地址*/
    uint32_t block_addr=-1;

    /*当前仅剩下一个盘块可分配，那就是指向的下一个空闲组*/
    if(fgroup->inside_pointer==0)
    {

        /*将指向的下一个空闲组复制进内存，空闲组所在的块被分配*/
        block_addr=fgroup->free_array[0];
        /*从内存中获取512字节到fgoup*/
        ide_read(cur_part->my_disk, fgroup->free_array[0], fgroup, 1);
    }
    /*空闲组中还有大于等于1个空闲盘块*/
    else
    {
        block_addr=fgroup->free_array[fgroup->inside_pointer--];
    }

    /*分配后把内存中的空闲组写入磁盘*/
    /*
    如果是多次分配的话，总共写入磁盘的耗时较大，
    建议提供另外的办法比如分配多个盘块后再统一进磁盘以提高效率
    */
    //TODO:待优化
    group_sync_default(fgroup);

    return block_addr;

}

/*分配一个盘块并返回盘块地址*/
int group_allocate(struct partition* part,struct free_group* fgroup){

/*没有可分配的盘块了*/
    if(fgroup->free_array[0]==0&&fgroup->inside_pointer==-1){
        return -1;
    }

    /*返回的盘块地址*/
    uint32_t block_addr=-1;

    /*当前仅剩下一个盘块可分配，那就是指向的下一个空闲组*/
    if(fgroup->inside_pointer==0)
    {

        /*将指向的下一个空闲组复制进内存，空闲组所在的块被分配*/
        block_addr=fgroup->free_array[0];
        /*从内存中获取512字节到fgoup*/
        ide_read(part->my_disk, fgroup->free_array[0], fgroup, 1);
    }
    /*空闲组中还有大于等于1个空闲盘块*/
    else
    {
        block_addr=fgroup->free_array[fgroup->inside_pointer--];
    }

    /*分配后把内存中的空闲组写入磁盘*/
    /*
    如果是多次分配的话，总共写入磁盘的耗时较大，
    建议提供另外的办法比如分配多个盘块后再统一进磁盘以提高效率
    */
    //TODO:待优化
    group_sync(part,fgroup);

    printk("allocate lba=%x\n",block_addr);

    return block_addr;

}



/*回收一个盘块
    返回值说明：返回-1说明回收失败*/
int group_recycle_default(struct free_group* fgroup,uint32_t addr){

    /*如果空闲块满了，需要把现有的内容写入地址为addr的盘块，
    当前块的下一组为地址为addr的盘块*/
    if(fgroup->inside_pointer==MAX_BLOCK_PER_GROUP-1)
    {

        ide_write(cur_part->my_disk,addr,fgroup,1);
        group_init(fgroup);
        fgroup->free_array[0]=addr;
    }
    /*空闲块没有满，可以进行回收*/
    else
    {
        fgroup->free_array[++(fgroup->inside_pointer)]=addr;
    }

    /*再进行写入磁盘*/
    group_sync_default(fgroup);

}

/*回收一个盘块
    返回值说明：返回-1说明回收失败*/
int group_recycle(struct partition* part,struct free_group* fgroup,uint32_t addr){

    /*如果空闲块满了，需要把现有的内容写入地址为addr的盘块，
    当前块的下一组为地址为addr的盘块*/
    if(fgroup->inside_pointer==MAX_BLOCK_PER_GROUP-1)
    {

        ide_write(part->my_disk,addr,fgroup,1);
        group_init(fgroup);
        fgroup->free_array[0]=addr;
    }
    /*空闲块没有满，可以进行回收*/
    else
    {
        fgroup->free_array[++(fgroup->inside_pointer)]=addr;
    }

    printk("resycle lba=%x\n",addr);

    /*再进行写入磁盘*/
    group_sync(part,fgroup);




}


/*进行磁盘初始化,应被fs.c/partition_format()调用
* 输入说明：空闲组 文件扇区起始地址 文件扇区数量
*/
void group_disk_init_default(struct free_group* fgroup,uint32_t file_begin_addr,uint32_t total_size){

    uint32_t end_addr=file_begin_addr+total_size;//文件区结束地址
    uint32_t cur_addr=file_begin_addr;//当前操作地址
    struct free_group tmp;
    
    /*每100个扇区执行操作：信息写入第一个块中*/
    for( ; cur_addr<end_addr ; cur_addr+=MAX_BLOCK_PER_GROUP)
    {
        group_init(&tmp);
        for(uint32_t i=1;i<= MAX_BLOCK_PER_GROUP;i++)
        {
            tmp.free_array[i-1]=cur_addr+MAX_BLOCK_PER_GROUP-i+1;
        }
        tmp.inside_pointer=MAX_BLOCK_PER_GROUP-1;


        ide_write(cur_part->my_disk,cur_addr,&tmp,1);

                /*把第一个区域放进磁盘特定区域*/
        if(cur_addr==file_begin_addr){

            ide_write(cur_part->my_disk,cur_part->sb->block_group_lba,&tmp,1);

        }
    }

    /*还剩下小于100的扇区*/
    uint32_t left_size=end_addr+MAX_BLOCK_PER_GROUP-cur_addr;
    if(left_size!=0)
    {
        group_init(&tmp);
        for(uint32_t i=1;i<=left_size;i++)
        {
            tmp.free_array[++tmp.inside_pointer]=cur_addr-MAX_BLOCK_PER_GROUP+i;
        }

        ide_write(cur_part->my_disk,cur_addr,&tmp,1);
    }

}


/*进行磁盘初始化,应被fs.c/partition_format()调用
* 输入说明：分区表指针 空闲组 磁盘中空闲组特殊位置 文件扇区起始地址 文件扇区数量
*/
void group_disk_init(struct partition* part,struct free_group* fgroup,uint32_t group_default_addr,uint32_t file_begin_addr,uint32_t total_size){

    uint32_t end_addr=file_begin_addr+total_size;//文件区结束地址
    uint32_t cur_addr=file_begin_addr;//当前操作地址
    struct free_group tmp;
    
    /*每100个扇区执行操作：信息写入第一个块中*/
    for( ; cur_addr<end_addr ; cur_addr+=MAX_BLOCK_PER_GROUP)
    {
        group_init(&tmp);
        for(uint32_t i=1;i<= MAX_BLOCK_PER_GROUP;i++)
        {
            tmp.free_array[i-1]=cur_addr+MAX_BLOCK_PER_GROUP-i+1;
        }
        tmp.inside_pointer=MAX_BLOCK_PER_GROUP-1;

        ide_write(part->my_disk,cur_addr,&tmp,1);

        /*把第一个区域放进磁盘特定区域*/
        if(cur_addr==file_begin_addr){
            printk("write the first block to specific sector %x\n",group_default_addr);
            ide_write(part->my_disk,group_default_addr,&tmp,1);
        }

    }

    printk("now is the left sectors below 100,cur_addr=%x\n",cur_addr);

    /*还剩下小于100的扇区*/
    uint32_t left_size=end_addr+MAX_BLOCK_PER_GROUP-cur_addr;
    if(left_size!=0)
    {
        group_init(&tmp);
        for(uint32_t i=1;i<=left_size;i++)
        {
            tmp.free_array[++tmp.inside_pointer]=cur_addr-MAX_BLOCK_PER_GROUP+i;
        }

        ide_write(part->my_disk,cur_addr-MAX_BLOCK_PER_GROUP,&tmp,1);
    }

    




}