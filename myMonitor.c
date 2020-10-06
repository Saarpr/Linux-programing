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


/////////////////////////////DEFINE//////////////////////////////

#define PORT     8080 
#define MAXLINE 1024 

/////////////////////////////PARAMS//////////////////////////////
typedef struct params{
char* _dir;
char* _ip;
}params;

/////////////////////////////FUNC//////////////////////////////

void args_parser(int argc, char* argv[], params *par){
  int opt; 
  // put ':' in the starting of the 
  // string so that program can  
  //distinguish between '?' and ':'  
  while((opt = getopt(argc, argv, "d:i:")) != -1)  
  {  
    switch(opt)  
      {  
      case 'i':
        par->_ip = (char*)malloc(128*sizeof(char));
        strcpy(par->_ip, optarg);
        break;  
      case 'd':  
        par->_dir = (char*)malloc(128*sizeof(char));
        strcpy(par->_dir, optarg);
        break;  
      default:  
        printf("unknown option: %c\n", optopt); 
        exit(EXIT_FAILURE);  
      }  
    }  
  }

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


const void handle_events(int fd, int *wd, params *par)
{

  int sockfd; 
  char buffer[MAXLINE]; 
  char *hello = "Hello from client"; 
  struct sockaddr_in servaddr; 
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
    perror("socket creation failed"); 
    exit(EXIT_FAILURE); 
  } 

  memset(&servaddr, 0, sizeof(servaddr)); 
  // Filling server information 
  servaddr.sin_family = AF_INET; 
  servaddr.sin_port = htons(PORT); 
  servaddr.sin_addr.s_addr = INADDR_ANY; 
    
  int n, nsent; 

  sendto(sockfd, (const char *)hello, strlen(hello), 
    MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
        sizeof(servaddr)); 

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

    /* Loop over all events in the buffer */

    for (ptr = buf; ptr < buf + len;
         ptr += sizeof(struct inotify_event) + event->len) {


         if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
      {
        printf("\n Error : Connect Failed \n");
        exit(0);
      }


      char time[50];
      format_time(time);
      event = (const struct inotify_event *) ptr;

      /* Print event type */
      if (event->mask & IN_OPEN){
        apache_print(par->_dir,event->name,"IN_OPEN: ",time);
        printf("IN_OPEN: ");
        sprintf( message, "FILE ACCESSED: %s\nACCESS: %s\nTIME OF ACCESS: %s\n",event->name, "IN_OPEN", time );
        printf("%s\n",message);
        sendto(sockfd, (const char *)message, strlen(message), 
    MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
        sizeof(servaddr)); 
      }
      if (event->mask & IN_CLOSE_NOWRITE){
        apache_print(par->_dir,event->name,"IN_CLOSE_NOWRITE: ",time);
        printf("IN_CLOSE_NOWRITE: ");
        sprintf( message, "FILE ACCESSED: %s\nACCESS: %s\nTIME OF ACCESS: %s\n",event->name, "IN_CLOSE_NOWRITE", time );
        printf("%s\n",message);
            sendto(sockfd, (const char *)message, strlen(message), 
    MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
        sizeof(servaddr)); 

      }
      if (event->mask & IN_CLOSE_WRITE){
        apache_print(par->_dir,event->name,"IN_CLOSE_WRITE: ",time);
        printf("IN_CLOSE_WRITE: ");
        sprintf( message, "FILE ACCESSED: %s\nACCESS: %s\nTIME OF ACCESS: %s\n",event->name, "IN_CLOSE_WRITE", time );
        printf("%s\n",message);
               sendto(sockfd, (const char *)message, strlen(message), 
    MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
        sizeof(servaddr)); 

      }
    }
  }
    close(sockfd); 

}



/////////////////////////////MAIN//////////////////////////////


int main(int argc, char *argv[])
{
  params par;

  args_parser(argc, argv, &par);

  printf("dir:  %s\n ip : %s\n",par._dir,par._ip );


  char buf;
  int fd, i, poll_num;
  int wd;
  nfds_t nfds;
  struct pollfd fds[2];

  if (argc < 2) {
    printf("Usage: %s PATH [PATH ...]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  /* Create the file descriptor for accessing the inotify API */

  fd = inotify_init1(IN_NONBLOCK);
  if (fd == -1) {
    perror("inotify_init1");
    exit(EXIT_FAILURE);
  }
  /* Mark directories for events
     - file was opened
     - file was closed */

    wd= inotify_add_watch(fd, par._dir, IN_OPEN | IN_CLOSE);
    if (wd == -1) {
      fprintf(stderr, "Cannot watch '%s'\n", par._dir);
      perror("inotify_add_watch");
      exit(EXIT_FAILURE);
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

        handle_events(fd, wd, &par);
      }
    }
  }

  printf("Listening for events stopped.\n");

  /* Close inotify file descriptor */

  close(fd);

  free(wd);
  exit(EXIT_SUCCESS);
}