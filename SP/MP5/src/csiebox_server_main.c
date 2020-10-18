#include "csiebox_server.h"
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
csiebox_server* box = 0;
void daemonize(int argc, char **argv) {
    fprintf(stderr, "start daemonize\n");
    pid_t pid = fork();
    if( pid < 0 ) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    //kill parent process
    if( pid > 0) exit(0);
    umask(0);
    pid_t sid = setsid();
    if( sid < 0 ) {
        fprintf(stderr, "setsid failure\n");
        exit(1);
    }
    // Change the current working directory to root, required root permission, or tmp
    if ( chdir( "/tmp") < 0) fprintf(stderr, "Cannot switch to tmp directory"); 
    /* * Attach file descriptors 0, 1, and 2 to /dev/null.  */
    FILE *fp = fopen ( "var/log/B04902045.log" , "w+"); 
    if( !fp ) fprintf(stderr, "Fail to open log file\n");

    int _fd = open( "/dev/null" , O_RDWR); 
    for(int i=0;i<=2;i++) dup2( _fd, i );
    // Open a log file in write mode.

    while( 1 ) {
        sleep(1);
        csiebox_server_init(&box, argc, argv);
        if( box ) 
            csiebox_server_run(box);
        else 
            fprintf(stderr, "Server init fail\n");
    }
    fclose(fp);
}
//fifo file path
char fifo_path[ PATH_MAX ];
void handler(int sig) {
    if( sig == SIGUSR1) {
        uint32_t ret = 0;
        for(int i=0;i<getdtablesize();i++) 
            if( box->client[i]) {
                assert( box->client[i]->conn_fd == i );
                ret++;
            }
        //fprintf(stderr, "ret = %u\n", ret);
        ret = htonl(ret);
        int fd = open(fifo_path, O_WRONLY);
        write(fd, &ret, sizeof(uint32_t));
        close(fd);
        signal(SIGUSR1, handler);
    }else if( sig == SIGINT || sig == SIGTERM ) {
        //delete pid file
        char path[ PATH_MAX ];
        sprintf(path, "%s\ncsiebox_server.pid", box->run_path);
        unlink(path);
        exit(0);
    }
}
int main(int argc, char** argv) {
  fprintf(stderr, "pid = %d\n", getpid());
  if( argc > 2 && strcmp(argv[2], "-d") == 0) {
      //deamonize the server
      signal(SIGINT, handler);
      signal(SIGTERM, handler);
      daemonize(argc, argv);
  }else {
    csiebox_server_init(&box, argc, argv);
    if (box) {
        sprintf(fifo_path, "%s/csiebox_server.%d", box->run_path, getpid());
        mkfifo(fifo_path, 0644);
        signal(SIGUSR1, handler);
        csiebox_server_run(box);
    }else
        fprintf(stderr, "Server init fail\n");
  }

  csiebox_server_destroy(&box);
  return 0;
}
