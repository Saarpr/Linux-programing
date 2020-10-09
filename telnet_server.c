#include <libcli.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define PORT_T 8090
#define LEN 100

#ifdef __GNUC__
#define UNUSED(d) d __attribute__((unused))
#else
#define UNUSED(d) d
#endif

//globals//

pthread_t telnet;
char **backtrace_arr;
int num_of_traces;
char flag;
sem_t semaphore; 
void *backtrace_buffer[LEN];
unsigned int regular_count = 0;
unsigned int debug_regular = 0;

//--------------------------From libCli library------------------------//

int init_backtrace(struct cli_def *cli, UNUSED(const char *command), UNUSED(char *argv[]), UNUSED(int argc)) {

    flag = 1;
    cli_print(cli, "Backtrace command returned %d addresses\n", num_of_traces);

    for (int j = 0; j < num_of_traces; j++) {
        cli_print(cli, "%s\n", backtrace_arr[j]);
    }
    sem_post(&semaphore);

    return CLI_OK;
}

int check_auth(const char *username, const char *password) {
    if (strcasecmp(username, "fred") != 0) return CLI_ERROR;
    if (strcasecmp(password, "nerk") != 0) return CLI_ERROR;
    return CLI_OK;
}

int regular_callback(struct cli_def *cli) {
    regular_count++;
    if (debug_regular) {
        cli_print(cli, "Regular callback - %u times so far", regular_count);
        cli_reprompt(cli);
    }
    return CLI_OK;
}

int check_enable(const char *password) {
    return !strcasecmp(password, "topsecret");
}

int idle_timeout(struct cli_def *cli) {
    cli_print(cli, "Custom idle timeout");
    return CLI_QUIT;
}

void pc(UNUSED(struct cli_def *cli), const char *string) {
    printf("%s\n", string);
}

void run_child(int x) {
    struct cli_def *cli;

    cli = cli_init();
    cli_set_banner(cli, "Type command: backtrace ");
    cli_set_hostname(cli, "Saar And David");
    cli_telnet_protocol(cli, 1);
    cli_regular(cli, regular_callback);

    cli_regular_interval(cli, 5);

    cli_set_idle_timeout_callback(cli, 240, idle_timeout);


    cli_register_command(cli, NULL, "backtrace", init_backtrace, PRIVILEGE_UNPRIVILEGED, MODE_EXEC,
                                          "BT exec"); 
    cli_set_auth_callback(cli, check_auth);
    cli_set_enable_callback(cli, check_enable);
    {
        FILE *fh;

        if ((fh = fopen("clitest.txt", "r"))) {
            cli_print_callback(cli, pc);
            cli_file(cli, fh, PRIVILEGE_UNPRIVILEGED, MODE_EXEC);
            cli_print_callback(cli, NULL);
            fclose(fh);
        }
    }
    cli_loop(cli, x);
    cli_done(cli);
}

void* telnet_task (void *args){
    int s,x;
    struct sockaddr_in addr;
    int on = 1;

    telnet = pthread_self();

    signal(SIGCHLD, SIG_IGN);

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(0);
    }

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
        perror("setsockopt");
        exit(0);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT_T);
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(0);
    }

    if (listen(s, 50) < 0) {
        perror("listen");
        exit(0);
    }

    printf("Listening on port %d\n", PORT_T);
    while ((x = accept(s, NULL, 0))) {
        run_child(x);
        exit(0);
    }

    return 0;
}

void  __attribute__ ((no_instrument_function))  __cyg_profile_func_enter (void *this_fn,
                                                                          void *call_site)
{
        if(flag == 1) {
        sem_wait(&semaphore);
        free(backtrace_arr);
        backtrace_arr = (char**)malloc(0*sizeof(char*));
        num_of_traces = 0;
        flag = 0;
        }
        if (!pthread_equal(telnet, pthread_self())) {
            int count = backtrace(backtrace_buffer, LEN);
            char** string = backtrace_symbols(backtrace_buffer, count);
        backtrace_arr = (char**)realloc(backtrace_arr, (count + num_of_traces) * sizeof(char*));
        for (int h = 0; h < count; h++) {
            backtrace_arr[num_of_traces + h] = (char*)malloc(LEN*sizeof(char));
            strcpy(backtrace_arr[num_of_traces + h], string[h]);
        }
        num_of_traces += count;
        }
}