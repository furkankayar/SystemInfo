/*
 *
 *   systeminfo/2016510110.c
 *
 *
 *   arch/x86/entry/syscalls/syscall_64.tbl :
 *      335     common  systeminfo              __x64_sys_systeminfo
 *
 *   include/linux/syscalls.h :
 *      asmlinkage long sys_systeminfo(char __user *, char __user *, char __user *, char __user *);
 *
 *
 *   Function: get_part
 *   ------------------
 *   Returns lines of a string between given interval.
 *
 *
 *   Function: read_file
 *   -------------------
 *   Reads file that exists on given path up to given buffer size and returns the result.
 *
 *
 *   Function: split
 *   ---------------
 *   Splits the given string by using given delimiter and returns the result as string array.
 *
 *
 *
 *   2019, by Furkan Kayar
 *
*/

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <asm/uaccess.h>


#define RETURN_BUFFER_LEN 800
#define STATUS_BUFFER_LEN 1000
#define CPUINFO_BUFFER_LEN 300
#define UPTIME_BUFFER_LEN 50
#define LOADAVG_BUFFER_LEN 50
#define PARAM_LEN 10

char* get_part(char *string, int start_line, int end_line, int len){

    int i;
    char *result = (char*)kmalloc(sizeof(char) * len, GFP_KERNEL);
    int position = 0;
    int cur_line = 0;
    for(i=0;i<len;i++){

        if(string[i] == '\n'){
            cur_line++;
            if(cur_line == start_line){
                result[position++] = '\t';
                result[position++] = '\t';
            }
            else if(cur_line > start_line && cur_line <= end_line){
                result[position++] = '\n';
                result[position++] = '\t';
                result[position++] = '\t';
            }
            else if(cur_line > end_line){
                result[position++] = '\n';
            }
        }
        else{

            if(cur_line >= start_line && cur_line <= end_line){
                result[position++] = string[i];
            }
            else if(cur_line >= start_line)
                result[position++] = '\0';
        }
    }

    return result;
}

char* read_file(char* path, int buffer_size){

    char *buffer = (char*)kmalloc(sizeof(char) * buffer_size, GFP_KERNEL);
    int i;
    struct file *file_ptr;
    mm_segment_t fs;
    file_ptr = filp_open(path, O_RDONLY, 0);

    for(i=0;i<buffer_size;i++){
        buffer[i] = '\0';
    }

    if(IS_ERR(file_ptr)){
        printk(KERN_INFO "%s cannot be opened!", path);
        return NULL;
    }
    else{
        fs = get_fs();
        set_fs(get_ds());
        file_ptr->f_op->read(file_ptr, buffer, buffer_size, &file_ptr->f_pos);
        set_fs(fs);
    }
    filp_close(file_ptr, NULL);

    return buffer;
}

char** split(char* string, char delimiter){

    int i;
    int delimiter_cnt = 0;
    int str_cnt = 0;
    char **str_arr;

    for(i=0;i<strlen(string);i++)
        if(string[i] == delimiter)
            delimiter_cnt++;

    if(string[strlen(string) - 1] == '\n') string[strlen(string) - 1] = '\0';

    str_arr = (char**)kmalloc(sizeof(char*) * (delimiter_cnt + 1), GFP_KERNEL);

    for(i=0;i<strlen(string);){

        if(i==0 || string[i] == delimiter){
            char *tmp = (char*)kmalloc(sizeof(char) * 10, GFP_KERNEL);
            int k=0;
            if(i != 0) i++;
            while(string[i] != delimiter && i < strlen(string)){
                tmp[k++] = string[i++];
            }
            tmp[k] = '\0';
            str_arr[str_cnt++] = tmp;

        }
    }

    return str_arr;
}


SYSCALL_DEFINE4(systeminfo, char __user *, usr_write_back_buffer, char __user *, usr_param_0, char __user *, usr_param_1, char __user *, usr_param_2){

    char param_0[PARAM_LEN] = {'\0'};
    char param_1[PARAM_LEN] = {'\0'};
    char param_2[PARAM_LEN] = {'\0'};
    char* cpuinfo_buffer = read_file("/proc/cpuinfo", CPUINFO_BUFFER_LEN);
    char* uptime_buffer = NULL;
    char** uptime_arr = NULL;
    char* loadavg_buffer = NULL;
    char** loadavg_arr = NULL;
    char* status_buffer = NULL;
    char proc_path[20] = "/proc/";
    char return_buffer[RETURN_BUFFER_LEN] = {'\0'};
    int i = 0;
    int proc_info_check = 0;
    int return_val = 0;

    if(copy_from_user(param_0, usr_param_0, PARAM_LEN) == 0){
        printk(KERN_INFO "\"%s\" has been copied from user!", param_0);
    }
    if(copy_from_user(param_1, usr_param_1, PARAM_LEN) == 0){
        printk(KERN_INFO "\"%s\" has been copied from user!", param_1);
    }
    if(copy_from_user(param_2, usr_param_2, PARAM_LEN) == 0){
        printk(KERN_INFO "\"%s\" has been copied from user!", param_2);
    }

    cpuinfo_buffer = get_part(cpuinfo_buffer, 1, 4, CPUINFO_BUFFER_LEN);

    strcat(return_buffer, "CPU Information:\n");
    strcat(return_buffer, cpuinfo_buffer);

    if(param_1 != NULL && strcmp(param_1, "-p") == 0 && param_2 != NULL){
        strcat(proc_path, param_2);
        strcat(proc_path, "/status");
        status_buffer = read_file(proc_path, STATUS_BUFFER_LEN);
        strcat(return_buffer, "Process Information:\n\t\t");
        if(status_buffer != NULL){
            strcat(return_buffer, get_part(status_buffer, 0, 0, STATUS_BUFFER_LEN));
            strcat(return_buffer, get_part(status_buffer, 2, 2, STATUS_BUFFER_LEN));
            strcat(return_buffer, get_part(status_buffer, 8, 8, STATUS_BUFFER_LEN));
            strcat(return_buffer, get_part(status_buffer, 5, 5, STATUS_BUFFER_LEN));
            strcat(return_buffer, get_part(status_buffer, 6, 6, STATUS_BUFFER_LEN));
        }
        else{
            strcat(return_buffer, "ERR: Process not found!\n");
        }
        proc_info_check = 1;
    }
    if(param_0 != NULL && strcmp(param_0, "-all") == 0){

        uptime_buffer = get_part(read_file("/proc/uptime", UPTIME_BUFFER_LEN), 0, 0, UPTIME_BUFFER_LEN);
        uptime_arr = split(uptime_buffer, ' ');
        loadavg_buffer = get_part(read_file("/proc/loadavg", LOADAVG_BUFFER_LEN), 0, 0, LOADAVG_BUFFER_LEN);
        loadavg_arr = split(loadavg_buffer, ' ');

        strcat(return_buffer, "System Statistics:\n");
        strcat(return_buffer, "\t\tsystem was booted since\t\t: ");
        strcat(return_buffer, uptime_arr[0]);
        strcat(return_buffer, "\n\t\tsystem has been idle since\t: ");
        strcat(return_buffer, uptime_arr[1]);
        strcat(return_buffer, "\n\t\tthe number of active tasks\t: ");
        for(i=0;i<4;i++){
            strcat(return_buffer, loadavg_arr[i]);
            if(i != 3) strcat(return_buffer, " ");
            else strcat(return_buffer, "\n");
        }
        strcat(return_buffer, "\t\tthe total number of processes\t: ");
        strcat(return_buffer, split(loadavg_arr[3], '/')[1]);
        strcat(return_buffer, "\n");
    }
    else if(param_0 != NULL && strcmp(param_0, "-p") == 0 && proc_info_check == 0 && param_1 != NULL){

        strcat(proc_path, param_1);
        strcat(proc_path, "/status");
        status_buffer = read_file(proc_path, STATUS_BUFFER_LEN);
        strcat(return_buffer, "Process Information:\n\t\t");
        if(status_buffer != NULL){
            strcat(return_buffer, get_part(status_buffer, 0, 0, STATUS_BUFFER_LEN));
            strcat(return_buffer, get_part(status_buffer, 2, 2, STATUS_BUFFER_LEN));
            strcat(return_buffer, get_part(status_buffer, 8, 8, STATUS_BUFFER_LEN));
            strcat(return_buffer, get_part(status_buffer, 5, 5, STATUS_BUFFER_LEN));
            strcat(return_buffer, get_part(status_buffer, 6, 6, STATUS_BUFFER_LEN));
        }
        else{
            strcat(return_buffer, "ERR: Process not found!\n");
        }
    }

    return_val = copy_to_user(usr_write_back_buffer, return_buffer, RETURN_BUFFER_LEN);
    if(return_val == 0){
        printk(KERN_INFO "%s\nhas been copied to user!", return_buffer);
    }

    return return_val;
}
