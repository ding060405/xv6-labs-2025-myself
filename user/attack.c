#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"
int judge(char *p){
  if((*p<='9'&&*p>='0')||(*p<='z'&&*p>='a')||(*p<='Z'&&*p>='A')) return 1;
  return 0;
}
int
main(int argc, char *argv[])
{
  // Your code here.
  char *p=sbrk(8*PGSIZE);
  char *end=p+PGSIZE;
  while(p!=end){
    int flag=0;
    if(judge(p)){
      char *tmp=p;
      while(judge(p)&&p!=end){
        p++;
      }
      if(p[0]=='\0'){
        flag=1;
        printf("%s\n",tmp);     
      }
    }
    if(flag){
      break;
    }
    p++;
  }
  exit(1);
}
