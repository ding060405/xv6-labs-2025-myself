# trap_design
## backtrace
### to do
implement backtrace for debuging
### idea
using stack frame to trace which function is invoked
### struct 
none
### function
```cpp
// get now stack frame pointer
static inline uint64
r_fp()
{
  uint64 x;
  asm volatile("mv %0, s0" : "=r" (x) );
  return x;
}
```
### corresponding knowledge
返回栈指针-8 ---> 函数地址
返回栈指针-16 ---> 下一个函数栈帧的地址
os为stack分配一个page，利用page边界判断是否回溯到栈底
### process
get stack pointer
|
v
set bround b=PGROUNDDOWN
loop:
    *(sp-8)----> function address
    *(sp-16)----> next stack pointer
until b!=PGROUNDDOWN(sp) // 超出栈

## alarm
### to do
implement alarm
sigalarm : 设置执行周期，执行函数 
sigreturn : 恢复中断前的状态
### kernel process
sigalarm
|
v
usertrap
|
v
sigreturn
### idea
- 在proc中增加中断函数func，间隔周期ticks，和先前状态tpf_store,以及当前周期now_ticks
### process
#### sigalarm
get argument
|
set ticks and func of proc
#### sigreturn
restore trapframe
#### usertrap
if ticks==now_ticks
else now_ticks++
|
v
store previous trapframe
|
v
update trapframe epc to func
|
v
now_ticks to 0


