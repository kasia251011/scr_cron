#include "task_manager.h"

struct task_node_t *head = NULL;
int id_generator = 0;
sigset_t set;

void task_manager_init() {
  sigfillset(&set);
}

task_t * convert_strings_to_task(char str[MAX_ARGV][MAX_LEN_ARGV] , int count) {
  //error handling - arguments count, time format

  task_t * task = malloc(sizeof(task_t));
  task->program_argc = count;
  task->seconds = atoi(str[count - 1]);
  for (int i = 0; i < count; i++) {
    strcpy(task->program_args[i], str[i]);
  }

  return task;
}

int add_task(task_t * task) {

  task->id = id_generator++;
  detect_task(task);

  struct task_node_t * task_node = malloc(sizeof(struct task_node_t));

  task_node->task = task;
  task_node->next = head;
  head = task_node;

  return 0;
}

int delete_task(int task_id) {

  struct task_node_t * current = head;
  struct task_node_t * previous = NULL;

  if(head == NULL) {
    printf("List is empty \n");
    return -1;
  }

  while(current->task->id != task_id) {

    if(current->next == NULL) {
      return -1;
    } else {
      previous = current;
      current = current->next;
    }
  }

  printf("Delete task with id: %d\n", current->task->id);

  timer_delete(*(current->task->timer));
  free(current->task->timer);
  free(current->task);

  if(current == head) {
    head = head->next;
  } else {
    previous->next = current->next;
  }

  free(current);

  return 0;

}

void for_each_task(for_each_func func, mqd_t mqd, size_t bytes) {
  struct task_node_t * task_node = head;
  char  response [100];
  while(task_node != NULL) {

    sprintf(response, "%-5d %-5d %s",
            task_node->task->id,
            task_node->task->seconds,
            concat_strings(task_node->task->program_args, task_node->task->program_argc));
    func(mqd, response, bytes, 0);
    task_node = task_node->next;
  }
}

char * concat_strings(char str [][100], int count) {
  int len = 0;

  for (int i = 0; i < count; i++) {
    len += (int)strlen(str[i]);
  }
  len += count - 1;
  char * string = malloc(sizeof(char ) * (len + 1)); //+1 for \0

  strcpy(string, str[0]);

  for(int i = 1; i < count; i++) {
    strcat(string, " ");
    strcat(string, str[i]);
  }

  return string;
}

int detect_task(task_t * task) {

  struct sigaction sa;
  sa.sa_mask = set;
  sa.sa_flags = SA_RESTART;

  timer_t * timer = malloc(sizeof(timer_t));

  struct sigevent timer_event;
  timer_event.sigev_notify = SIGEV_THREAD;
  timer_event.sigev_notify_function = execute_program;
  timer_event.sigev_value.sival_ptr = (void*) task;
  timer_event.sigev_notify_attributes = NULL;
  timer_create(CLOCK_REALTIME, &timer_event, timer);

  struct itimerspec its;
  its.it_value.tv_sec = task->seconds;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;

  timer_settime(*timer, 0, &its, NULL);

  task->timer = timer;

  return 0;
}

char** create_array_with_null(int argc, char argv[MAX_ARGV][MAX_LEN_ARGV]) {
  char** arr = (char**)malloc(sizeof(char*) * (argc + 1));
  for (int i = 0; i < argc; i++) {
    arr[i] = strdup(argv[i]);
  }

  arr[argc] = NULL;
  return arr;
}

void execute_program(sigval_t sigval)  {

  task_t * task = (task_t*) sigval.sival_ptr;
  printf("\tStart executing program `%s` id: %d after %d sec\n", task->program_args[0], task->id, task->seconds);
  char ** args = create_array_with_null(task->program_argc, task->program_args);
  pid_t child_pid;
  posix_spawn(&child_pid, task->program_args[0], NULL, NULL, args, NULL);
  int status;
  do
    waitpid(child_pid, &status, 0);
  while(!WIFEXITED(status));

  printf("\tExecuted program\n");

  delete_task(task->id);
}

