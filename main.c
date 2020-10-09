#include <errno.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/inotify.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "myMonitor.c"


int main(int argc, char *argv[])
{
  params par;

  pthread_t inotify;
  // pthread_t telnet;



  args_parser(argc, argv, &par);

  printf("dir:  %s\n ip : %s\n",par._dir,par._ip );

  // inotify_task(&par);


 if (pthread_create(&inotify, NULL, inotify_task, (void*)&par))
    return 1;
 // if (pthread_create(&telnet, NULL, telnet_task, (void*)&bt))
 //    return 1;

 pthread_join(inotify, NULL);
 // pthread_join(telnet, NULL);



  exit(EXIT_SUCCESS);
}