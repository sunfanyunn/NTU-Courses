#ifndef _SERVER_FUNCTION_H_
#define _SERVER_FUNCTION_H_

#include "csiebox_common.h"
#include "csiebox_server.h"

#define DIR_S_FLAG (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) //permission you can use to create new directory
#define REG_S_FLAG (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) //permission you can use to create new file

int server_sync_meta( const csiebox_protocol_meta req, int conn_fd, const csiebox_server *server );

int server_sync_file( int conn_fd, const char *path );

int server_sync_symblink(  int conn_fd, const char *path  );

int server_sync_hardlink( const csiebox_protocol_hardlink req, int conn_fd, const csiebox_server *server );

int server_rm( const csiebox_protocol_rm req, int conn_fd, const csiebox_server *server);

int server_send_meta( const char *path, int conn_fd, const csiebox_server *server );

int server_send_file( const char *path, int conn_fd, const csiebox_server *server);

int server_send_symblink( const char *path, int conn_fd, const csiebox_server *server );

int server_send_hardlink( const char *src, const char *target, int conn_fd, const csiebox_server *server );

#endif
