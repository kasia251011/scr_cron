#include <stdio.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include "task_manager.h"
#include <signal.h>

#define MAX_BYTES sizeof(main_args_t)
#define MAX_ARGV 10
#define MAX_LEN_ARGV 100
#define EXIT_RESPONSE "exit"

mqd_t server_reader;
mqd_t server_writer;
const char * CLIENT_WRITER = "/client_writer";
const char * SERVER_WRITER = "/server_writer";

typedef struct{
  char argv[MAX_ARGV][MAX_LEN_ARGV];
  int argc;
} main_args_t;

void handler(int n) {
  printf("Server clean up\n");
  mq_unlink(CLIENT_WRITER);
  mq_unlink(SERVER_WRITER);
  mq_close(server_reader);
  mq_close(server_writer);
  delete_linked_list();
  exit(0);
}

int main(int argc, char * argv[]) {

  mqd_t client_reader = mq_open(SERVER_WRITER, O_RDONLY, 0666, NULL);
  if (client_reader == (mqd_t) -1) {

    signal(SIGTERM, handler);

    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_flags = 0;
    attr.mq_msgsize = MAX_BYTES; //size in bytes for message
    server_reader = mq_open(CLIENT_WRITER, O_CREAT | O_RDONLY, 0666, &attr);
    server_writer = mq_open(SERVER_WRITER, O_CREAT | O_WRONLY, 0666, &attr);
    printf("------------- SERVER -------------\n");

    task_manager_init();

    while(1) {
      char msg[MAX_BYTES];
      if(mq_receive(server_reader, msg, MAX_BYTES, NULL) == -1) {
        perror( "Error" );
      }
      main_args_t * main_client_args = (main_args_t*)msg;


      if (strcmp(main_client_args->argv[1], "add") == 0) {
        task_t * task = convert_strings_to_task(main_client_args->argv + 2, main_client_args->argc - 2);
        add_task(task);
        char * response = "Added new task!";
        printf("%s\n", response);
        mq_send(server_writer, response, MAX_BYTES, 0);
        mq_send(server_writer, EXIT_RESPONSE, MAX_BYTES, 0);
      }

      if (strcmp(main_client_args->argv[1], "list") == 0) {
        mq_send(server_writer, "ID    TIME  TASK\n---------------------------------------\n", MAX_BYTES, 0);
        for_each_task(mq_send, server_writer, MAX_BYTES);
        mq_send(server_writer, EXIT_RESPONSE, MAX_BYTES, 0);
      }

      if (strcmp(main_client_args->argv[1], "del") == 0) {
        delete_task(atoi(main_client_args->argv[2]));
        char * response = "Deleted task!";
        printf("%s\n", response);
        mq_send(server_writer, response, MAX_BYTES, 0);
        mq_send(server_writer, EXIT_RESPONSE, MAX_BYTES, 0);
      }
    }

  } else {
    printf("------------- CLIENT -------------\n");
    //convert args to structure
    main_args_t main_args;
    main_args.argc = argc;
    for(int i = 0; i < argc; i++) {
      strcpy(main_args.argv[i], argv[i]);
    }

    mqd_t client_writer = mq_open(CLIENT_WRITER, O_WRONLY, 0666, NULL);

    if(mq_send(client_writer, (char*)&main_args, MAX_BYTES, 0) == -1) {
      perror("Error");
    }

    char msg[MAX_BYTES] = "";

    while(strcmp(msg, EXIT_RESPONSE) != 0) {
      printf("%s\n", msg);
      mq_receive(client_reader, msg, MAX_BYTES, NULL);
    }

    mq_close(client_reader);
    mq_close(client_writer);
  }

  return 0;
}

