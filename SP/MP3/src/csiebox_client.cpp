#include "csiebox_client.h"

#include "csiebox_common.h"
#include "connect.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//header included by myself
#include <utime.h>
#include <assert.h>
#include <errno.h>
#include <sys/inotify.h> //header for inotify
#include <dirent.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <unistd.h>
//c++ headers
#include <map>
#include <string>

static int parse_arg(csiebox_client* client, int argc, char** argv);
static int login(csiebox_client* client);

//read config file, and connect to server
void csiebox_client_init(
    csiebox_client** client, int argc, char** argv) {
    csiebox_client* tmp = (csiebox_client*)malloc(sizeof(csiebox_client));
    if (!tmp) {
      fprintf(stderr, "client malloc fail\n");
      return;
    }
    memset(tmp, 0, sizeof(csiebox_client));
    if (!parse_arg(tmp, argc, argv)) {
      fprintf(stderr, "Usage: %s [config file]\n", argv[0]);
      free(tmp);
      return;
    }
    int fd = client_start(tmp->arg.name, tmp->arg.server);
    if (fd < 0) {
      fprintf(stderr, "connect fail\n");
      free(tmp);
      return;
    }
    tmp->conn_fd = fd;
    *client = tmp;
}

// my code starts here

// sync_meta return -1 if error occurs, otherwise return respond status
static int sync_meta(csiebox_client *client, char *fullpath, const int rel) {
      csiebox_protocol_meta req;
      memset(&req, 0, sizeof(req));
      req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
      req.message.header.req.op = CSIEBOX_PROTOCOL_OP_SYNC_META;
      req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);
      req.message.body.pathlen = strlen(fullpath+rel);

      struct stat st;
      if( lstat(fullpath, &st ) < 0 )  return puts("lstat error"), -1;
      req.message.body.stat = st;

      if( !S_ISDIR( st.st_mode) ) md5_file(fullpath, req.message.body.hash);

      //send request
      if (!send_message(client->conn_fd, &req, sizeof(req))) {
        fprintf(stderr, "send fail\n");
        return -1;
      }
      //send pathname
      if( !send_message(client->conn_fd, fullpath+rel, strlen(fullpath+rel)) ) {
        fprintf(stderr, "send fail\n");
        return -1;
      }
      if( S_ISLNK( st.st_mode ) ) return 0x03;
      csiebox_protocol_header header;
      recv_message(client->conn_fd, &header, sizeof(header));
      return header.res.status;
}
//sync_file do a sync_meta request first
static int sync_file(csiebox_client * client, char *fullpath, const int rel) {
    int res = sync_meta(client, fullpath, rel);
    if( res == 0x03 ) {
        //handle symbolic link independently
        char link[4096];
        ssize_t res = readlink(fullpath, link, 4096);
        link[res] = 0;
        send_message(client->conn_fd, &res, sizeof(ssize_t));
        send_message(client->conn_fd, link, res);
        return 1;
    }
    //for directory or regular file
    struct stat st; lstat(fullpath, &st);
    //directory must be existed after sync_meta, no further action needed
    if( S_ISDIR(st.st_mode) ) return CSIEBOX_PROTOCOL_STATUS_OK;
    csiebox_protocol_file req;
    switch(res) {
        case CSIEBOX_PROTOCOL_STATUS_OK:
            return 1;
        case CSIEBOX_PROTOCOL_STATUS_FAIL:
            return fprintf(stderr, "sync_meta error"), -1;
        case CSIEBOX_PROTOCOL_STATUS_MORE:
            req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
            req.message.header.req.op = CSIEBOX_PROTOCOL_OP_SYNC_META;
            req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);
            req.message.body.datalen = st.st_size;
            fprintf(stderr, "upload file size = %d\n", (int)st.st_size);
            send_message(client->conn_fd, &req, sizeof(req));

#if 1
            fprintf(stderr, "sync_file %s\n", fullpath+rel);
#endif
			int in_fd = open(fullpath, O_RDONLY );
            char buf[8192];
            while( 1 ) {
                ssize_t res = read(in_fd, buf, sizeof(buf) );
                if( !res ) break;
                send_message(client->conn_fd, buf, res);
            }
            close(in_fd);
            break;
    }
}
static int Rm(csiebox_client *client, char *fullpath, const int rel) {
      csiebox_protocol_rm req;
      memset(&req, 0, sizeof(req));
      req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
      req.message.header.req.op = CSIEBOX_PROTOCOL_OP_RM;
      req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);
      req.message.body.pathlen = strlen(fullpath+rel);
      //send request
      if (!send_message(client->conn_fd, &req, sizeof(req))) {
        fprintf(stderr, "send fail\n");
        return -1;
      }
      //send pathname
      if(!send_message(client->conn_fd, fullpath+rel, strlen(fullpath+rel)) ) {
        fprintf(stderr, "send fail\n");
        return -1;
      }
      csiebox_protocol_header header;
      recv_message(client->conn_fd, &header, sizeof(header));
      return header.res.status;
}
static int hardlink(csiebox_client *client, char *fullpath, const int rel, struct stat st, std::map<ino_t, std::string> &ma) {
    csiebox_protocol_hardlink req;
    memset(&req, 0, sizeof(req));
    req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
    req.message.header.req.op = CSIEBOX_PROTOCOL_OP_SYNC_HARDLINK;
    req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);
    req.message.body.srclen = strlen(fullpath+rel);
    req.message.body.targetlen = (int)ma[ st.st_ino ].size();
    fprintf(stderr, "hardlink, src = %s target = %s\n", fullpath+rel, ma[st.st_ino].c_str());

    if (!send_message(client->conn_fd, &req, sizeof(req))) {
      fprintf(stderr, "send fail\n");
      return -1;
    }

    //send path
    fprintf(stderr, "srclen = %d\n", req.message.body.srclen);
    if(!send_message(client->conn_fd, fullpath+rel, req.message.body.srclen) ) {
      fprintf(stderr, "send fail\n");
      return -1;
    }
    fprintf(stderr, "targetlen = %d\n", req.message.body.targetlen);
    if(!send_message(client->conn_fd, (void *)ma[ st.st_ino ].c_str(), req.message.body.targetlen) ) {
      fprintf(stderr, "send fail\n");
      return -1;
    }
}
static void download(csiebox_client *client) {

    csiebox_protocol_header header;
    memset(&header, 0, sizeof(header));
    header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
    header.req.op = 0x06;
    send_message(client->conn_fd, &header, sizeof(header));

    struct stat st;
    int len;
    char fullpath[4096];
    strcpy(fullpath, client->arg.path);
    int rel = strlen(client->arg.path);
    if( fullpath[rel-1] != '/' ) fullpath[rel] = '/', fullpath[++rel] = 0;

    std::map<ino_t, std::string> ma;
    while( recv_message(client->conn_fd, &len, sizeof(int)) ) {
        if( len == 0 ) break;
        recv_message(client->conn_fd, fullpath+rel, len);
        fullpath[rel+len] = 0;
        fprintf(stderr, "load %s, rel = %s\n", fullpath, fullpath+rel);
        recv_message(client->conn_fd, &st, sizeof(st));


        if( S_ISDIR(st.st_mode) ) {
            mkdir( fullpath, st.st_mode );
        }else if( S_ISREG( st.st_mode) ) {

            if( st.st_nlink > 1 ) {
                if( ma.count( st.st_ino ) ) {
                    int len;
                    char buf[PATH_MAX]; strncpy(buf, fullpath, rel);

                    recv_message(client->conn_fd, &len, sizeof(len));
                    recv_message(client->conn_fd, buf+rel, len);
                    buf[ rel + len ] = 0;
                    fprintf(stderr, "link %s %s\n", buf, fullpath);
                    link(buf, fullpath);
                    continue;
                }else
                    ma[ st.st_ino ] = std::string(fullpath+rel);
            }

            int out_fd = open(fullpath, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);
            off_t sz; recv_message(client->conn_fd, &sz, sizeof(off_t));
            char buff[8192];
            while(sz > 0) {
                ssize_t res = (8192 < sz)?8192:sz;
                recv_message(client->conn_fd, buff, res);
                write(out_fd, buff, res);
                sz -= res;
            }
        }else if( S_ISLNK( st.st_mode) ) {
            char link[4096];
            ssize_t res;
            recv_message(client->conn_fd, &res, sizeof(ssize_t));
            recv_message(client->conn_fd, link, res);
            link[res] = 0;
            symlink(link, fullpath);
        }else;

        struct utimbuf new_times;
        new_times.actime = st.st_atime; //keep atime unchanged
        new_times.modtime = st.st_mtime;
        utime(fullpath, &new_times);
    }
}

//usage : fullpath terminates with '/' if it's a directory
int myftw(csiebox_client *client, char *fullpath, const int rel, std::map< ino_t, std::string> &ma) {
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
        if( lstat(fullpath, &st ) < 0 ) return puts("lstat error"), -1;

        if( st.st_nlink > 1 )  {
            //handle hardlink
            fprintf(stderr, "hardlink %s\n", fullpath+rel);
            if( ma.count( st.st_ino ) ) {
                fprintf(stderr, "call hardlink\n");
                hardlink(client, fullpath, rel, st, ma);
            }
            else {
                ma[ st.st_ino ] = std::string(fullpath+rel);
                sync_file(client, fullpath, rel);
            }
        }else
            sync_file(client, fullpath, rel);

        if( S_ISDIR( st.st_mode )  ) {
            printf("directory %s\n", fullpath);
            int N = strlen(fullpath);
            if( fullpath[N-1] != '/') fullpath[N] = '/', fullpath[++N] = 0;

            if( myftw(client, fullpath, rel, ma) == -1 ) return -1;
        }
        else if( S_ISREG(st.st_mode) ) {
            fprintf(stderr, "regular %s\n", fullpath);
        }
        else if( S_ISLNK(st.st_mode) ) {
            printf("symbolic %s\n", fullpath);
        }else {
            printf("other file type not handled\n");
        }
    }
    closedir(dp);
    return ret;
}
//ftw for inotify
void ftw_inotify(csiebox_client *client, char *fullpath, const int rel, int fd, std::map<int, std::string> &ma) {

    struct dirent *dirp;
    DIR *dp;

    if( (dp = opendir(fullpath)) == NULL ) {
        puts("can't read directory");
        return;
    }

    int wd = inotify_add_watch(fd, fullpath, IN_CREATE | IN_DELETE | IN_ATTRIB | IN_MODIFY);
    ma[ wd ] = std::string(fullpath+rel);

    int n = strlen(fullpath);
    while( (dirp = readdir(dp)) != NULL ) {
        if( dirp->d_name[0] == '.' ) continue;

        strcpy(fullpath+n, dirp->d_name);
        fprintf(stderr, "fullpath = %s, rel = %s\n", fullpath, fullpath+rel);
        struct stat st;
        if( lstat(fullpath, &st ) < 0 ) {
            puts("lstat error");
            return;
        }
        if( S_ISDIR( st.st_mode )  ) {
            printf("Add directory to inotify %s\n", fullpath);
            int N = strlen(fullpath);
            if( fullpath[N-1] != '/') fullpath[N] = '/', fullpath[++N] = 0;

            ftw_inotify(client, fullpath, rel, fd, ma);
        }
        /*
        else if( S_ISREG(st.st_mode) ) {
            printf("regular %s\n", fullpath);
        }
        else if( S_ISLNK(st.st_mode) ) {
            printf("symbolic %s\n", fullpath);
        }else {
            printf("other file type not handled\n");
        }
        */
    }
    closedir(dp);
}

//this is where client sends request, you sould write your code here
int csiebox_client_run(csiebox_client* client) {
  if (!login(client)) {
    fprintf(stderr, "login fail\n");
    return 0;
  }
  fprintf(stderr, "login success\n");


  //====================
  //        TODO: add your client-side code here
  //====================

  // traverse file system, do the syncronization of the first scenerio
  char fullpath[4096];
  int rel = strlen(client->arg.path);
  strcpy( fullpath, client->arg.path );
  if( fullpath[rel-1] != '/' ) fullpath[ rel++ ] = '/', fullpath[ rel ] = 0;
  std::map<ino_t, std::string> maa;
  int ftw_status = myftw(client, fullpath, rel, maa);
  if( ftw_status == -1 ) fprintf(stderr, "myftw error\n");
  if( ftw_status == 0 ) {
      //cdir is empty
      //download content from the sdir
      download(client);
  }

  // start inotify ==> the third syncronization
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

  int length, i = 0;
  int fd;
  char buffer[EVENT_BUF_LEN];
  memset(buffer, 0, EVENT_BUF_LEN);

  //create a instance and returns a file descriptor
  fd = inotify_init();
  if (fd < 0) {
    perror("inotify_init");
  }

  std::map<int, std::string> ma;
  rel = strlen(client->arg.path);
  strcpy( fullpath, client->arg.path );
  if( fullpath[rel] != '/' ) fullpath[ rel++ ] = '/', fullpath[ rel ] = 0;

  ftw_inotify(client, fullpath, rel, fd, ma);
  //add directory ".." to watch list with specified events
  //int wd = inotify_add_watch(fd, client->arg.path, IN_CREATE | IN_DELETE | IN_ATTRIB | IN_MODIFY);
  while ((length = read(fd, buffer, EVENT_BUF_LEN)) > 0) {
    i = 0;
    while (i < length) {
      struct inotify_event* event = (struct inotify_event*)&buffer[i];
      //relpath should be relative pathname of a directory
      std::string relpath = ma[ event->wd ] + std::string(event->name);

      fprintf(stderr, "inotify relpath = %s\n", relpath.c_str());
      fprintf(stderr, "event: (%d, %d, %s)\ntype: ", (int)event->wd, (int)strlen(event->name), event->name);

      i += EVENT_SIZE + event->len;

      if (event->mask & IN_CREATE) { 
        fprintf(stderr, "create ");
        strcpy(fullpath+rel, relpath.c_str());
        sync_file( client, fullpath ,rel);
      }
      else if (event->mask & IN_DELETE) {
        fprintf(stderr, "delete ");
        strcpy(fullpath+rel, relpath.c_str());
        Rm( client, fullpath, rel);
      }
      else if (event->mask & IN_ATTRIB) {
        fprintf(stderr, "attrib ");
        strcpy(fullpath+rel, relpath.c_str());
        sync_file( client, fullpath ,rel);
      }
      else if (event->mask & IN_MODIFY) {
        fprintf(stderr, "modify ");
        strcpy(fullpath+rel, relpath.c_str());
        sync_file( client, fullpath ,rel);
      }else
        continue;

      if (event->mask & IN_ISDIR) {
        fprintf(stderr, "dir\n");
      } else {
        fprintf(stderr, "file\n");
      }

      if( (event->mask & IN_CREATE) && (event->mask & IN_ISDIR) ) {
          //add the created directory into inotify
          int wd = inotify_add_watch(fd, fullpath, IN_CREATE | IN_DELETE | IN_ATTRIB | IN_MODIFY);
          ma[ wd ] = relpath + '/';
          fprintf(stderr, "New directory created are added to inotify\nfullpath = %s, relpath = %s\n",fullpath,  ma[wd].c_str());
      }
      if( (event->mask & IN_DELETE) && (event->mask & IN_ISDIR) ) {
          fprintf(stderr, "inotify_rm_watch %s, wd = %d\n", fullpath, event->wd);
          inotify_rm_watch(fd, event->wd);
      }

    }
    memset(buffer, 0, EVENT_BUF_LEN);
  }
  close(fd);
  return 1;
}

void csiebox_client_destroy(csiebox_client** client) {
  csiebox_client* tmp = *client;
  *client = 0;
  if (!tmp) {
    return;
  }
  close(tmp->conn_fd);
  free(tmp);
}

//read config file
static int parse_arg(csiebox_client* client, int argc, char** argv) {
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
  int accept_config_total = 5;
  int accept_config[5] = {0, 0, 0, 0, 0};
  while ((keylen = getdelim(&key, &keysize, '=', file) - 1) > 0) {
    key[keylen] = '\0';
    vallen = getline(&val, &valsize, file) - 1;
    val[vallen] = '\0';
    fprintf(stderr, "config (%d, %s)=(%d, %s)\n", (int)keylen, key, (int)vallen, val);
    if (strcmp("name", key) == 0) {
      if (vallen <= sizeof(client->arg.name)) {
        strncpy(client->arg.name, val, vallen);
        accept_config[0] = 1;
      }
    } else if (strcmp("server", key) == 0) {
      if (vallen <= sizeof(client->arg.server)) {
        strncpy(client->arg.server, val, vallen);
        accept_config[1] = 1;
      }
    } else if (strcmp("user", key) == 0) {
      if (vallen <= sizeof(client->arg.user)) {
        strncpy(client->arg.user, val, vallen);
        accept_config[2] = 1;
      }
    } else if (strcmp("passwd", key) == 0) {
      if (vallen <= sizeof(client->arg.passwd)) {
        strncpy(client->arg.passwd, val, vallen);
        accept_config[3] = 1;
      }
    } else if (strcmp("path", key) == 0) {
      if (vallen <= sizeof(client->arg.path)) {
        strncpy(client->arg.path, val, vallen);
        accept_config[4] = 1;
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

static int login(csiebox_client* client) {
  csiebox_protocol_login req;
  memset(&req, 0, sizeof(req));
  req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
  req.message.header.req.op = CSIEBOX_PROTOCOL_OP_LOGIN;
  req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);

  memcpy(req.message.body.user, client->arg.user, strlen(client->arg.user));
  md5(client->arg.passwd,
      strlen(client->arg.passwd),
      req.message.body.passwd_hash);
  if (!send_message(client->conn_fd, &req, sizeof(req))) {
    fprintf(stderr, "send fail\n");
    return 0;
  }
  csiebox_protocol_header header;
  memset(&header, 0, sizeof(header));
  if (recv_message(client->conn_fd, &header, sizeof(header))) {
    if (header.res.magic == CSIEBOX_PROTOCOL_MAGIC_RES &&
        header.res.op == CSIEBOX_PROTOCOL_OP_LOGIN &&
        header.res.status == CSIEBOX_PROTOCOL_STATUS_OK) {
      client->client_id = header.res.client_id;
      return 1;
    } else {
      return 0;
    }
  }
  return 0;
}
