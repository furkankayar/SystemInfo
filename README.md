Furkan Kayar

arch/x86/entry/syscalls/syscall_64.tbl :<br/>
  335&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;common&nbsp;&nbsp;systeminfo&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;__x64_sys_systeminfo


include/linux/syscalls.h :<br/>
  asmlinkage long sys_systeminfo(char __user *, char __user *, char __user *, char __user *);
