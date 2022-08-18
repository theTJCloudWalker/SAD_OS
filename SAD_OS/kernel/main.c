#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "process.h"
#include "syscall-init.h"
#include "syscall.h"
#include "stdio.h"
#include "memory.h"
#include "dir.h"
#include "fs.h"
#include "assert.h"
#include "shell.h"

#include "ide.h"
#include "stdio-kernel.h"

void init(void);

int main(void) {
   put_str("I am kernel\n");
   init_all();

/*//文件创建写入读取的测试
   uint32_t fd = sys_open("/file1", O_CREAT|O_RDWR); 
   printf("fd:%d\n", fd); 
   sys_write(fd, "hello,world\nhello,world\n", 24);  
   sys_close(fd); 
   printf("%d closed now\n", fd);

   fd = sys_open("/file1", O_RDWR); 
   printf("open /file1,fd:%d\n", fd); 
   char buf[64] = {0}; 
   int read_bytes = sys_read(fd, buf, 18); 
   printf("1_ read %d bytes:\n%s\n", read_bytes, buf); 
   
   memset(buf, 0, 64); 
   read_bytes = sys_read(fd, buf, 6); 
   printf("2_ read %d bytes:\n%s", read_bytes, buf); 
   
   memset(buf, 0, 64); 
   read_bytes = sys_read(fd, buf, 6); 
   printf("3_ read %d bytes:\n%s", read_bytes, buf); 
   
   printf("________ close file1 and reopen ________\n"); 
   sys_close(fd); 
   fd = sys_open("/file1", O_RDWR); 
   memset(buf, 0, 64); 
   read_bytes = sys_read(fd, buf, 24); 
   printf("4_ read %d bytes:\n%s", read_bytes, buf); 
   
   sys_close(fd);
*/


//文件夹的创建测试

/*
   printf("/dir1/subdir1 create %s!\n", \
   sys_mkdir("/dir1/subdir1") == 0 ? "done" : "fail"); 
   printf("/dir1 create %s!\n", sys_mkdir("/dir1") == 0 ? "done" : "fail"); 
   printf("now, /dir1/subdir1 create %s!\n", \
   sys_mkdir("/dir1/subdir1") == 0 ? "done" : "fail"); 
   int fd = sys_open("/dir1/subdir1/file2", O_CREAT|O_RDWR); 
   if (fd != -1) { 
      printf("/dir1/subdir1/file2 create done!\n"); 
      sys_write(fd, "Catch me if you can!\n", 21); 
      sys_lseek(fd, 0, SEEK_SET); 
      char buf[32] = {0}; 
      sys_read(fd, buf, 21); 
      printf("/dir1/subdir1/file2 says:\n%s", buf); 
      sys_close(fd); 
   }
*/




/*************    写入应用程序    *************/
//   uint32_t file_size = 5698; 
//   uint32_t sec_cnt = DIV_ROUND_UP(file_size, 512);
//   struct disk* sda = &channels[0].devices[0];
//   void* prog_buf = sys_malloc(file_size);
//   ide_read(sda, 300, prog_buf, sec_cnt);
//   int32_t fd = sys_open("/cat", O_CREAT|O_RDWR);
//   if (fd != -1) {
//      if(sys_write(fd, prog_buf, file_size) == -1) {
//         printk("file write error!\n");
//         while(1);
//      }
//   }
/*************    写入应用程序结束   *************/
   cls_screen();
   console_put_str("[rabbit@localhost /]$ ");
   thread_exit(running_thread(), true);
   return 0;
}

/* init进程 */
void init(void) {
   uint32_t ret_pid = fork();
   if(ret_pid) {  // 父进程
      int status;
      int child_pid;
       /* init在此处不停的回收僵尸进程 */
       while(1) {
	  child_pid = wait(&status);
	  printf("I`m init, My pid is 1, I recieve a child, It`s pid is %d, status is %d\n", child_pid, status);
       }
   } else {	  // 子进程
      my_shell();
   }
   panic("init: should not be here");
}
