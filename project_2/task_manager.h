#ifndef PROJECT_2_TASK_MANAGER_H
#define PROJECT_2_TASK_MANAGER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <mqueue.h>
#include <fcntl.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
//COMMAND STRUCTURE
//Add task to list - add <command> <seconds>
//Delete task - del <task-id>
//List all tasks - list

#define MAX_ARGV 10
#define MAX_LEN_ARGV 100

typedef enum {
  ADD, DELETE, LIST
} option_t;

typedef struct {
  int id;
  char program_args [MAX_ARGV][MAX_LEN_ARGV];
  int program_argc;
  timer_t * timer;
  int seconds;
} task_t;

struct task_node_t {
  task_t * task;
  struct task_node_t * next;
};

typedef int (*for_each_func)(mqd_t, const char *, size_t, unsigned int);

char** create_array_with_null(int argc, char argv[MAX_ARGV][MAX_LEN_ARGV]);
void for_each_task(for_each_func func, mqd_t mqd, size_t bytes);
void task_manager_init();
task_t * convert_strings_to_task(char str[MAX_ARGV][MAX_LEN_ARGV] , int count);
int add_task(task_t * task);
int delete_task(int task_id);
char * concat_strings(char str [][100], int count);

int detect_task(task_t * task);
void execute_program(sigval_t sigval);
void delete_linked_list();

#endif
