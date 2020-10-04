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

#define PORT 10000



void apache_print(char *dir, char *file, char *event, char *time) {
    FILE *apache = fopen("/var/www/html/index.html", "a+");
    fprintf(apache, "<h1> directory: %s file name: %s event %s time: %s </h1>", dir, file, event, time);
    fclose(apache);
}


void format_time(char *output){
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    sprintf(output, "[%d %d %d %d:%d:%d]",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}


static void handle_events(int fd, int *wd, int argc, char *argv[])
{

  int sock ,nsent;
  struct sockaddr_in s = {0};
  s.sin_family = AF_INET;
  s.sin_port = htons(PORT);
  s.sin_addr.s_addr = htons("127.0.0.2");

  sock = socket(AF_INET,SOCK_DGRAM,0);

  if(connect(sock,(struct sockaddr*)&s, sizeof(s))<0){
    perror("connect");
    return 1;
  }

  printf("Successfully connected. \n");
  char buf[4096];
  char message[2000];

      __attribute__ ((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  int i;
  ssize_t len;
  char *ptr;

  /* Loop while events can be read from inotify file descriptor. */

  for (;;) {

    /* Read some events. */

    len = read(fd, buf, sizeof buf);
    if (len == -1 && errno != EAGAIN) {
      perror("read");
      exit(EXIT_FAILURE);
    }

    /* If the nonblocking read() found no events to read, then
       it returns -1 with errno set to EAGAIN. In that case,
       we exit the loop. */

    if (len <= 0)
      break;

    /* Loop over all events in the buffer */

    for (ptr = buf; ptr < buf + len;
         ptr += sizeof(struct inotify_event) + event->len) {

      char time[50];
      format_time(time);
      event = (const struct inotify_event *) ptr;

      /* Print event type */

      if (event->mask & IN_OPEN){
        apache_print(argv[i],event->name,"IN_OPEN: ",time);
        printf("IN_OPEN: ");
        sprintf( message, "FILE ACCESSED: %s\nACCESS: %s\nTIME OF ACCESS: %s\n",event->name, "IN_OPEN", time );
        if ((nsent = send(sock,message,sizeof(message),0))<0){
          perror("recv");
          return 1;
        }
      }
      if (event->mask & IN_CLOSE_NOWRITE){
        apache_print(argv[i],event->name,"IN_CLOSE_NOWRITE: ",time);
        printf("IN_CLOSE_NOWRITE: ");
        sprintf( message, "FILE ACCESSED: %s\nACCESS: %s\nTIME OF ACCESS: %s\n",event->name, "IN_CLOSE_NOWRITE", time );
        if ((nsent = send(sock,message,sizeof(message),0))<0){
          perror("recv");
          return 1;
        }

      }
      if (event->mask & IN_CLOSE_WRITE){
        apache_print(argv[i],event->name,"IN_CLOSE_WRITE: ",time);
        printf("IN_CLOSE_WRITE: ");
        sprintf( message, "FILE ACCESSED: %s\nACCESS: %s\nTIME OF ACCESS: %s\n",event->name, "IN_CLOSE_WRITE", time );
        if ((nsent = send(sock,message,sizeof(message),0))<0){
          perror("recv");
          return 1;
        }

      }

      /* Print the name of the watched directory */

      for (i = 1; i < argc; ++i) {
        if (wd[i] == event->wd) {
          printf("%s/", argv[i]);
          break;
        }
      }

      /* Print the name of the file */

      if (event->len)
        printf("%s", event->name);

      /* Print type of filesystem object */

      if (event->mask & IN_ISDIR)
        printf(" [directory]\n");
      else
        printf(" [file]\n");
    }
  }
}

int main(int argc, char *argv[])
{

  char buf;
  int fd, i, poll_num;


  int *wd;
  nfds_t nfds;
  struct pollfd fds[2];

  if (argc < 2) {
    printf("Usage: %s PATH [PATH ...]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  printf("Press ENTER key to terminate.\n");

  /* Create the file descriptor for accessing the inotify API */

  fd = inotify_init1(IN_NONBLOCK);
  if (fd == -1) {
    perror("inotify_init1");
    exit(EXIT_FAILURE);
  }

  /* Allocate memory for watch descriptors */

  wd = calloc(argc, sizeof(int));
  if (wd == NULL) {
    perror("calloc");
    exit(EXIT_FAILURE);
  }

  /* Mark directories for events
     - file was opened
     - file was closed */

  for (i = 1; i < argc; i++) {
    wd[i] = inotify_add_watch(fd, argv[i], IN_OPEN | IN_CLOSE);
    if (wd[i] == -1) {
      fprintf(stderr, "Cannot watch '%s'\n", argv[i]);
      perror("inotify_add_watch");
      exit(EXIT_FAILURE);
    }
  }

  /* Prepare for polling */

  nfds = 2;

  /* Console input */

  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  /* Inotify input */

  fds[1].fd = fd;
  fds[1].events = POLLIN;

  /* Wait for events and/or terminal input */

  printf("Listening for events.\n");
  while (1) {
    poll_num = poll(fds, nfds, -1);
    if (poll_num == -1) {
      if (errno == EINTR)
        continue;
      perror("poll");
      exit(EXIT_FAILURE);
    }

    if (poll_num > 0) {

      if (fds[0].revents & POLLIN) {

        /* Console input is available. Empty stdin and quit */

        while (read(STDIN_FILENO, &buf, 1) > 0
               && buf != '\n')
          continue;
        break;
      }

      if (fds[1].revents & POLLIN) {

        /* Inotify events are available */

        handle_events(fd, wd, argc, argv);
      }
    }
  }

  printf("Listening for events stopped.\n");

  /* Close inotify file descriptor */

  close(fd);

  free(wd);
  exit(EXIT_SUCCESS);
}