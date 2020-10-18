#include "csiebox_server.h"

#include "csiebox_common.h"
#include "connect.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>

// header included
#include <assert.h>
#include <dirent.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <map>
#include <algorithm>

static int parse_arg(csiebox_server* server, int argc, char** argv);
static int handle_request(csiebox_server* server, int conn_fd);
static int get_account_info(
  csiebox_server* server,  const char* user, csiebox_account_info* info);
static void logout(csiebox_server* server, int conn_fd);

static char* get_user_homedir(
  csiebox_server* server, csiebox_client_info* info);

// five session declaration
static char* get_user_dir(
  csiebox_server* server, int conn_fd);
static void login(
  csiebox_server* server, int conn_fd, csiebox_protocol_login* login);
//sync file is included in sync_meta
static void sync_meta(
  csiebox_server* server, int conn_fd, csiebox_protocol_meta* meta);
static void sync_hardlink(
  csiebox_server* server, int conn_fd, csiebox_protocol_hardlink* hardlink);
static void Rm(
  csiebox_server* server, int conn_fd, csiebox_protocol_rm* rm);
int exist(char *name);


#define DIR_S_FLAG (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)//permission you can use to create new file
#define REG_S_FLAG (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)//permission you can use to create new directory

//read config file, and start to listen
void csiebox_server_init(
    csiebox_server** server, int argc, char** argv) {
    csiebox_server* tmp = (csiebox_server*)malloc(sizeof(csiebox_server));
    if (!tmp) {
      fprintf(stderr, "server malloc fail\n");
      return;
    }
    memset(tmp, 0, sizeof(csiebox_server));
    if (!parse_arg(tmp, argc, argv)) {
      fprintf(stderr, "Usage: %s [config file]\n", argv[0]);
      free(tmp);
      return;
    }
    int fd = server_start();
    if (fd < 0) {
      fprintf(stderr, "server fail\n");
      free(tmp);
      return;
    }
    tmp->client = (csiebox_client_info**)
        malloc(sizeof(csiebox_client_info*) * getdtablesize());

    if (!tmp->client) {
      fprintf(stderr, "client list malloc fail\n");
      close(fd);
      free(tmp);
      return;
    }
    memset(tmp->client, 0, sizeof(csiebox_client_info*) * getdtablesize());
    tmp->listen_fd = fd;
    *server = tmp;
}

//wait client to connect and handle requests from connected socket fd
//===============================
//		TODO: you need to modify code in here and handle_request() to support I/O multiplexing
//===============================
int csiebox_server_run(csiebox_server* server) {

    fd_set master; FD_ZERO(&master);
    fd_set read_fds; FD_ZERO(&read_fds);
    FD_SET(server->listen_fd, &master);
    int fd_max = server->listen_fd;

    while (1) {
        read_fds = master;
        //read_fds will be modified after select
        if( select(fd_max+1, &read_fds, NULL, NULL, NULL) == -1 ) {
            perror("select");
        }
        fprintf(stderr, "fd_max = %d\n", fd_max);
        for(int i = 0; i <= fd_max; i++) if( FD_ISSET(i, &read_fds) ) {
            if( i == server->listen_fd) {
                struct sockaddr_in addr;
                memset(&addr, 0, sizeof(addr));
                int conn_len = 0;
                // waiting client connect
                int conn_fd = accept(
                  server->listen_fd, (struct sockaddr*)&addr, (socklen_t*)&conn_len);
                FD_SET( conn_fd, &master);
                fd_max = std::max( fd_max, conn_fd );
                if (conn_fd < 0) {
                  if (errno == ENFILE) {
                      fprintf(stderr, "out of file descriptor table\n");
                      continue;
                    } else if (errno == EAGAIN || errno == EINTR) {
                      continue;
                    } else {
                      fprintf(stderr, "accept err\n");
                      fprintf(stderr, "code: %s\n", strerror(errno));
                      break;
                    }
                }
                // handle request from connected socket fd
                continue;
            }
            // i is the connection file descriptor of the current processed client
            if( handle_request(server, i) == -1 ) {
                logout(server, i);
                FD_CLR(i, &master);
            }
            sleep(1);
        }
    }
    return 1;
}
void csiebox_server_destroy(csiebox_server** server) {
    csiebox_server* tmp = *server;
    *server = 0;
    if (!tmp) {
      return;
    }
    close(tmp->listen_fd);
    int i = getdtablesize() - 1;
    for (; i >= 0; --i) {
      if (tmp->client[i]) {
        free(tmp->client[i]);
      }
    }
    free(tmp->client);
    free(tmp);
}
//read config file
static int parse_arg(csiebox_server* server, int argc, char** argv) {
    if (argc != 2) {
      return 0;
    }
    FILE* file = fopen(argv[1], "r");
    if (!file) {
      return 0;
    }
    fprintf(stderr, "reading config...\n");
    size_t keysize = 20, valsize = 20;
    char* key = (char*)malloc(sizeof(char) * keysize);
    char* val = (char*)malloc(sizeof(char) * valsize);
    ssize_t keylen, vallen;
    int accept_config_total = 2;
    int accept_config[2] = {0, 0};
    while ((keylen = getdelim(&key, &keysize, '=', file) - 1) > 0) {
      key[keylen] = '\0';
      vallen = getline(&val, &valsize, file) - 1;
      val[vallen] = '\0';
      fprintf(stderr, "config (%d, %s)=(%d, %s)\n", keylen, key, vallen, val);
      if (strcmp("path", key) == 0) {
        if (vallen = sizeof(server->arg.path)) {
          strncpy(server->arg.path, val, vallen);
          accept_config[0] = 1;
        }
      } else if (strcmp("account_path", key) == 0) {
        if (vallen <= sizeof(server->arg.account_path)) {
          strncpy(server->arg.account_path, val, vallen);
          accept_config[1] = 1;
        }
      }
    }
    free(key);
    free(val);
    fclose(file);
    int i, test = 1;
    for (i = 0; i < accept_config_total; ++i) {
      test = test & accept_config[i];
    }
    if (!test) {
      fprintf(stderr, "config error\n");
      return 0;
    }
    return 1;
}

//usage : fullpath terminates with '/' if it's a directory
int myftw(csiebox_server *server, int conn_fd, char *fullpath, const int rel, std::map<ino_t, std::string>& ma) {
    fprintf(stderr, "enter server's myftw\n");
    struct stat st;
    struct dirent *dirp;
    DIR *dp;

    if( (dp = opendir(fullpath)) == NULL )
        return puts("can't read directory"), -1;
    int ret = 0;
    int n = strlen(fullpath);
    while( (dirp = readdir(dp)) != NULL ) {
        if( dirp->d_name[0] == '.' ) continue;
        ret++;

        strcpy(fullpath+n, dirp->d_name);
        fprintf(stderr, "fullpath = %s, rel = %s\n", fullpath, fullpath+rel);
        if( lstat(fullpath, &st ) < 0 ) {
            return fprintf(stderr, "%s lstat error", fullpath), -1;
        }
        int len = strlen(fullpath+rel);
        send_message(conn_fd, &len, sizeof(int));
        send_message(conn_fd, fullpath+rel, len);
        send_message(conn_fd, &st, sizeof(st));

        if( S_ISDIR( st.st_mode )  ) {
            printf("directory %s\n", fullpath);
            int N = strlen(fullpath);
            fullpath[N] = '/', fullpath[++N] = 0;

            if( myftw(server, conn_fd, fullpath, rel, ma) == -1 ) return -1;
        }
        else if( S_ISREG(st.st_mode) ) {
            if( st.st_nlink > 1) {
                if( ma.count( st.st_ino ) ) {
                    fprintf(stderr, "hardlink ");
                    int len = (int)ma[ st.st_ino ].size();
                    send_message(conn_fd, &len, sizeof(len));
                    send_message(conn_fd, (void *)ma[ st.st_ino ].c_str(), len);
                    continue;
                }
                else 
                    ma[ st.st_ino ] = std::string(fullpath+rel);
            }

            fprintf(stderr, "regular %s\n", fullpath);

            send_message(conn_fd, &st.st_size, sizeof(off_t));
            int in_fd = open(fullpath, O_RDONLY);
            char buff[8192];
            while(1) {
                ssize_t res = read(in_fd, buff, sizeof(buff));
                if( !res ) break;
                send_message(conn_fd, buff, res);
            }
        }
        else if( S_ISLNK(st.st_mode) ) {
            printf("symbolic %s\n", fullpath);
            char link[4096];
            ssize_t res = readlink(fullpath, link, 4096);
            link[res] = 0;
            send_message(conn_fd, &res, sizeof(ssize_t));
            send_message(conn_fd, link, res);
        }else {
            printf("other file type not handled\n");
        }
    }
    closedir(dp);
    return ret;
}

//this is where the server handle requests, you should write your code here
static int handle_request(csiebox_server* server, int conn_fd) {
  csiebox_protocol_header header;
  memset(&header, 0, sizeof(header));

  if (recv_message(conn_fd, &header, sizeof(header))) {
    if (header.req.magic != CSIEBOX_PROTOCOL_MAGIC_REQ) {
        return 1;
    }
    switch (header.req.op) {
      case CSIEBOX_PROTOCOL_OP_LOGIN:
        fprintf(stderr, "login\n");
        csiebox_protocol_login req;
        if (complete_message_with_header(conn_fd, &header, &req)) {
          login(server, conn_fd, &req);
        }
        break;
      case CSIEBOX_PROTOCOL_OP_SYNC_META:
        fprintf(stderr, "sync meta\n");
        csiebox_protocol_meta meta;
        if (complete_message_with_header(conn_fd, &header, &meta)) {
            sync_meta(server, conn_fd, &meta);
          //====================
          //        TODO: here is where you handle sync_meta and even sync_file request from client
          //====================
        }
        break;
      case CSIEBOX_PROTOCOL_OP_SYNC_HARDLINK:
        fprintf(stderr, "sync hardlink\n");
        csiebox_protocol_hardlink hardlink;
        if (complete_message_with_header(conn_fd, &header, &hardlink)) {
            sync_hardlink(server, conn_fd, &hardlink);
          //====================
          //        TODO: here is where you handle sync_hardlink request from client
          //====================
        }
        break;
      case CSIEBOX_PROTOCOL_OP_SYNC_END:
        fprintf(stderr, "sync end\n");
        csiebox_protocol_header end;
        fprintf(stderr, "Not implemented\n");
          //====================
          //        TODO: here is where you handle end of synchronization request from client
          //====================
        break;
      case CSIEBOX_PROTOCOL_OP_RM:
        fprintf(stderr, "rm\n");
        csiebox_protocol_rm rm;
        if (complete_message_with_header(conn_fd, &header, &rm)) {
            Rm(server, conn_fd, &rm);
          //====================
          //        TODO: here is where you handle rm file or directory request from client
          //====================
        }
        break;
      case 0x06:
      {
        //load everything to client directory
        fprintf(stderr, "Download request from client\n");
        char *fullpath = get_user_dir(server, conn_fd);
        int rel = strlen(fullpath);
        std::map<ino_t, std::string> ma;
        myftw(server, conn_fd, fullpath, rel, ma);
        free(fullpath);
        rel = 0;
        send_message(conn_fd, &rel, sizeof(int));
        break;
      }
      default:
        fprintf(stderr, "unknown op %x\n", header.req.op);
    }
  }else
    return fprintf(stderr, "Connection Closed\n"), -1;

  fprintf(stderr, "end of a request\n");
  return 1;
//  logout(server, conn_fd);
}

//open account file to get account information
static int get_account_info(
    csiebox_server* server,  const char* user, csiebox_account_info* info) {
    FILE* file = fopen(server->arg.account_path, "r");
    if (!file) {
      return 0;
    }
    size_t buflen = 100;
    char* buf = (char*)malloc(sizeof(char) * buflen);
    memset(buf, 0, buflen);
    ssize_t len;
    int ret = 0;
    int line = 0;
    while ((len = getline(&buf, &buflen, file) - 1) > 0) {
      ++line;
      buf[len] = '\0';
      char* u = strtok(buf, ",");
      if (!u) {
        fprintf(stderr, "illegal form in account file, line %d\n", line);
        continue;
      }
      if (strcmp(user, u) == 0) {
        memcpy(info->user, user, strlen(user));
        char* passwd = strtok(NULL, ",");
        if (!passwd) {
          fprintf(stderr, "illegal form in account file, line %d\n", line);
          continue;
        }
        md5(passwd, strlen(passwd), (uint8_t *)info->passwd_hash);
        ret = 1;
        break;
      }
    }
    free(buf);
    fclose(file);
    return ret;
}
static void logout(csiebox_server* server, int conn_fd) {
    free(server->client[conn_fd]);
    server->client[conn_fd] = 0;
    close(conn_fd);
}
static char* get_user_homedir(
    csiebox_server* server, csiebox_client_info* info) {
    char* ret = (char*)malloc(sizeof(char) * PATH_MAX);
    memset(ret, 0, PATH_MAX);
    sprintf(ret, "%s/%s", server->arg.path, info->account.user);
    return ret;
}
//my code
static char* get_user_dir(
    csiebox_server* server, int conn_fd) {

    char* ret = (char*)malloc(sizeof(char) * PATH_MAX);
    memset(ret, 0, PATH_MAX);

    sprintf(ret, "%s/%s/", server->arg.path, server->client[conn_fd]->account.user);
    return ret;
}
//handle the login request from client
static void login(
    csiebox_server* server, int conn_fd, csiebox_protocol_login* login) {
    int succ = 1;
    csiebox_client_info* info =
      (csiebox_client_info*)malloc(sizeof(csiebox_client_info));
    memset(info, 0, sizeof(csiebox_client_info));
    if (!get_account_info(server, (char *)login->message.body.user, &(info->account))) {
      fprintf(stderr, "cannot find account\n");
      succ = 0;
    }
    if (succ &&
        memcmp(login->message.body.passwd_hash,
               info->account.passwd_hash,
               MD5_DIGEST_LENGTH) != 0) {
      fprintf(stderr, "passwd miss match\n");
      succ = 0;
    }
    csiebox_protocol_header header;
    memset(&header, 0, sizeof(header));
    header.res.magic = CSIEBOX_PROTOCOL_MAGIC_RES;
    header.res.op = CSIEBOX_PROTOCOL_OP_LOGIN;
    header.res.datalen = 0;
    if (succ) {
      if (server->client[conn_fd]) {
        free(server->client[conn_fd]);
      }
      info->conn_fd = conn_fd;
      server->client[conn_fd] = info;
      header.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
      header.res.client_id = info->conn_fd;
      char* homedir = get_user_homedir(server, info);
      mkdir(homedir, DIR_S_FLAG);
      free(homedir);
    } else {
      header.res.status = CSIEBOX_PROTOCOL_STATUS_FAIL;
      free(info);
    }
    send_message(conn_fd, &header, sizeof(header));
}
//check file
int exist(char *name) {
    struct stat buffer;
    return (lstat (name, &buffer) == 0);
}
//handle sync_meta and sync_file
static void sync_meta(
  csiebox_server* server, int conn_fd, csiebox_protocol_meta* meta) {

    char *fullpath = get_user_dir(server, conn_fd);
    int rel = strlen(fullpath);

    char *buf = (char*) malloc(meta->message.body.pathlen+1);
    if( !recv_message(conn_fd, buf, meta->message.body.pathlen) ) {
         fprintf(stderr, "No pathname received\n");
         return ;
    }
    strncpy(fullpath+rel, buf, meta->message.body.pathlen);
    fullpath[ rel + meta->message.body.pathlen ] = 0;
    free(buf);

    //handle symbolic link independently
    if( S_ISLNK(meta->message.body.stat.st_mode) ) {
        //if the symbolic link exists, remove it
        if( exist(fullpath) ) unlink(fullpath);
        fprintf(stderr, "soft link %s\n", fullpath);

        char link[4096];
        ssize_t len;
        recv_message(conn_fd, &len, sizeof(ssize_t));
        recv_message(conn_fd, link, len);
        link[len] = 0;
        symlink(link, fullpath);
        return;
    }

    if( !exist(fullpath) ) {
        //file not exist
        if( S_ISDIR(meta->message.body.stat.st_mode) )
            mkdir( fullpath, DIR_S_FLAG);
        else if( S_ISREG(meta->message.body.stat.st_mode ) )
            open(fullpath, O_WRONLY|O_CREAT|O_TRUNC, REG_S_FLAG);
    }
#if 1
    fprintf(stderr, "sync_meta to %s\n", fullpath);
#endif

    struct stat old, New;
    lstat(fullpath, &old); New = meta->message.body.stat;
    struct utimbuf new_times;
    new_times.actime = old.st_atime; //keep atime unchanged
    new_times.modtime = New.st_mtime;
    utime(fullpath, &new_times);
    chmod(fullpath, New.st_mode);
    //no further actions needed for directory
    if( S_ISDIR( old.st_mode ) ) {
        csiebox_protocol_header header;
        memset(&header, 0, sizeof(header));
        header.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
        send_message(conn_fd, &header, sizeof(header));
        return ;
    }
    // respond, compare file hash
    buf = (char *) malloc(MD5_DIGEST_LENGTH);
    md5_file(fullpath, (uint8_t *) buf);

    if( strcmp( buf, (char *)meta->message.body.hash) == 0 ) {
        // file content is the same
        fprintf(stderr, "same file hash\n");
        csiebox_protocol_header header;
        memset(&header, 0, sizeof(header));
        /*
        header.res.magic = CSIEBOX_PROTOCOL_MAGIC_RES;
        header.res.op = CSIEBOX_PROTOCOL_OP_SYNC_META;
        header.res.client_id = conn_fd;
        header.res.datalen = 0;
        */
        header.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
        send_message(conn_fd, &header, sizeof(header));
    }else {
        //really have to receive file data from client and sync them
        csiebox_protocol_header header;
        memset(&header, 0, sizeof(header));
        /*
        header.res.magic = CSIEBOX_PROTOCOL_MAGIC_RES;
        header.res.op = CSIEBOX_PROTOCOL_OP_SYNC_META;
        header.res.client_id = conn_fd;
        header.res.datalen = 0;
        //returning MORE is the key
        */
        header.res.status = CSIEBOX_PROTOCOL_STATUS_MORE;
        send_message(conn_fd, &header, sizeof(header));

        csiebox_protocol_file file;
        recv_message(conn_fd, &file, sizeof(file));
        // start sync_file here
        char buff[8192];
        assert( sizeof(buff) == 8192 );
        off_t sz = file.message.body.datalen;
        int out_fd = open(fullpath, O_WRONLY | O_CREAT | O_TRUNC, New.st_mode );
        while( sz > 0 ) {
            ssize_t res = (8192 < sz )? 8192 : sz;
            recv_message(conn_fd, buff, res);
            write( out_fd, buff, res );
            sz -= res;
        }
        close(out_fd);
    }
    // modification time
    utime(fullpath, &new_times);
    free(buf);
    free(fullpath);
}
//handle sync_hardlink
static void sync_hardlink(
  csiebox_server* server, int conn_fd, csiebox_protocol_hardlink* hardlink) {

    char *fullpath = get_user_dir(server, conn_fd);
    int rel = strlen(fullpath);
    int srclen = hardlink->message.body.srclen ;
    int targetlen = hardlink->message.body.targetlen;
    char src[PATH_MAX], target[PATH_MAX];
    strcpy(src, fullpath); strcpy(target, fullpath);

    fprintf(stderr, "srclen = %d, targetlen = %d\n", srclen, targetlen);
    recv_message(conn_fd, src+rel, srclen);
    src[ rel+srclen ] = 0;
    recv_message(conn_fd, target+rel, targetlen);
    target[ targetlen+rel ] = 0;
    fprintf(stderr, "link %s %s\n", target, src);
    link(target, src);
    fprintf(stderr, "end sync_hardlink\n");
}
static void Rm(
  csiebox_server* server, int conn_fd, csiebox_protocol_rm* rm) {
    char *fullpath = get_user_dir(server, conn_fd);
    int rel = strlen(fullpath);

    char *buf = (char*) malloc(rm->message.body.pathlen+1);
    if( !recv_message(conn_fd, buf, rm->message.body.pathlen) ) {
         fprintf(stderr, "No pathname received\n");
         return ;
    }
    buf[ rm->message.body.pathlen ] = 0;
    strncpy(fullpath+rel, buf, rm->message.body.pathlen);
    fullpath[ rel + rm->message.body.pathlen ] = 0;
    fprintf(stderr, "remove file/dir %s\n" ,fullpath);
    if( remove( fullpath ) != 0 ) {
        fprintf(stderr, "remove error\n");
        perror("error");
    }

    csiebox_protocol_header header;
    memset(&header, 0, sizeof(header));
    header.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
    send_message(conn_fd, &header, sizeof(header));
    free(fullpath);
}
