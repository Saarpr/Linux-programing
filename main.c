#include <errno.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <execinfo.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/inotify.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include "myMonitor.c"
#include "telnet_server.c"


int main(int argc, char *argv[])
{
  params par;


  pthread_t inotify;

  if (sem_init(&semaphore, 0, 0) == -1){
        printf("sem failed\n");
        return 1;
  }

  args_parser(argc, argv, &par);

  printf("dir:  %s\n ip : %s\n",par._dir,par._ip );

  // inotify_task(&par);

  if (pthread_create(&inotify, NULL, (void*)inotify_task, (void*)&par))
    return 1;
  if (pthread_create(&telnet, NULL, telnet_task, NULL))
    return 1;

  pthread_join(inotify, NULL);
  pthread_join(telnet, NULL);


  sem_close (&semaphore);
  exit(EXIT_SUCCESS);
}