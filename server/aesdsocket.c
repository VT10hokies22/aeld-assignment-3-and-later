#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <signal.h> 

/*Variable for signal handling*/
volatile sig_atomic_t exit1 = 0;

/*Global listening socket int for handling when stuck in accept listening*/
volatile sig_atomic_t socketfd;

// Signal handler function
void signalHandler(int sig) {
    printf("Interrupt handled: %d\n", sig);
    fflush(stdout);
    syslog(LOG_INFO, "Caught signal, exiting %d", sig);
    exit1 = 1;
    close(socketfd); /* Close listening socket */
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    /* Check for daemon argument */
    int daemon_check = 0;
    /*Number of int arguments */
    if (argc > 1){
        if (strcmp(argv[1], "-d") == 0){
            daemon_check = 1;
            printf("Daemon\n");
        }
    }

    char *port = "9000";

    int domain = PF_INET6;

    int type = SOCK_STREAM;

    int protocol = 0;

    int bind_status;

    // Handle signal
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    socketfd = socket(domain, type, protocol);

    if (socketfd == -1){
        printf("Socket creation failed, returning -1\n");
        return -1;
    }

    /* Change socket setting so rebind is possible when the socket closes */
    // set SO_REUSEADDR on a socket to true (1):
    int optval = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    printf("Socket created fd: %d\n", socketfd);

    struct addrinfo hints;
    const char *node = NULL;
    struct addrinfo *res;

    int getaddr_return;

    memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    getaddr_return = getaddrinfo(node, port, &hints, &res);

    printf("getaddrinfo returned: %d\n", getaddr_return);

    if (getaddr_return != 0){
        printf("getaddrinfo failed, returning -1\n");
        return -1;
    }

    struct sockaddr *addr = (*res).ai_addr;
    socklen_t addr_len = (*res).ai_addrlen;

    bind_status = bind(socketfd, addr, addr_len);

    freeaddrinfo(res);

    if (bind_status == -1){
        printf("Bind failed, returning -1\n");
        return -1;
    }

    if (daemon_check == 1){
        /* Creates a parent and child process */
        pid_t pid;
        pid = fork(); /* Forks the current process */

        if (pid < 0) {
            /* Error to fork*/
            return 0;
        }
        else if (pid == 0){
            /* Child process, do nothing*/
        }
        else {
            /* Parent process */
            return 0; /* Exit parent process */
        }
    }

    printf("Bind status: %d\n", bind_status);

    int backlog = 10;
    int listen_result;
    listen_result = listen(socketfd, backlog);

    if (listen_result == -1){
        printf("Listen failed, returning -1\n");
        return -1;
    }

    printf("Listen result: %d\n", listen_result);

    while(exit1 == 0){
        int accept_fd;
        fflush(stdout);

        /* Storage locations for accept connection */
        struct sockaddr_storage accept_addr;
        socklen_t accept_addr_len = sizeof(accept_addr); /* Length of addr*/

        /* File descriptor return */
        accept_fd = accept(socketfd, (struct sockaddr *)&accept_addr, &accept_addr_len);



        if (accept_fd == -1){
            printf("Accept stopped, deleting the file to client\n");
            break;
        }

        char s[INET6_ADDRSTRLEN];

        inet_ntop(accept_addr.ss_family, get_in_addr((struct sockaddr *)&accept_addr), s, sizeof s);
            printf("server: got connection from %s\n", s);


        printf("Accept result: %s\n", s);


        syslog(LOG_INFO, "Accepted connection from %s", s);
        fflush(stdout);
        char buf[1024];
        int n = 0;
        int input_len;
        char *whole_buf = malloc(1);
        *whole_buf = '\0'; /* Initialize whole_buf to an empty string */
        int total_len = 0; /* Initialize total_len to 0 */

        /* Cycles through looking for newline */
        while (n==0){
            /* Recv needs a \0 at the end to indicate the end of the string, recv doesnt automatically do this. needed for strcat for c
            to know where to add strings together*/
            input_len = recv(accept_fd, buf, sizeof(buf) - 1, 0);
            total_len += input_len;

            /* If recv failed */
            if (input_len == -1){
                printf("Recv failed, returning -1\n");
                return -1;
            }
            /* If nothing received */
            else if (input_len == 0){
                printf("Client closed connection\n");
                break;
            }

            /* Received data */
            else {
                n = 0;
                buf[input_len] = '\0';
                printf("HHHHHHHHHHHHHHHHHHHHHHHHH\n");
                /*printf("%s", buf);*/
                whole_buf = realloc(whole_buf, total_len +1); /* +1 for \0*/

                /* For length of buf*/
                for (int i = 0; i < input_len; i++){
                    /* Looking for newline*/
                    if (buf[i] == '\n'){
                        /* Adds buffer to whole_buf even on last line*/
                        strcat(whole_buf, buf);
                        n = 1;
                        break;
                    }
                }

                if (n == 0){
                    /* Adds buffer to whole_buf but only when no newline is found */
                    /* Doesnt get run if newline is found */
                    strcat(whole_buf, buf);
                }
            }
        }

        /*printf("Whole buffer: %s\n", whole_buf);*/    

        FILE *wri;
        wri = fopen("/var/tmp/aesdsocketdata", "a");
        fprintf(wri, "%s", whole_buf);
        fclose(wri);
        /* Return the allocated space*/
        free(whole_buf);

        /* Reads file and sendd*/
        FILE *read1 = fopen("/var/tmp/aesdsocketdata", "r");
        fseek(read1, 0, SEEK_END);
        printf("%ld", ftell(read1));
        int length1 = ftell(read1);
        fseek(read1, 0, SEEK_SET);
        char send_result[length1+1];
        send_result[length1] = '\0';
        fread(send_result,1,length1,read1);
        fclose(read1);

        printf("send result: %s\n", send_result);
        /*Send file back to client*/
        send(accept_fd, send_result, length1,0);
        syslog(LOG_INFO, "Closed connection from %s", s);
        close(accept_fd); /* Close the accepted connection */
    }

    remove("/var/tmp/aesdsocketdata");
    return 0;

}
