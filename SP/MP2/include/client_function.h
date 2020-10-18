#ifndef _CLIENT_FUNCTION_H_
#define _CLIENT_FUNCTION_H_

#include "csiebox_common.h"
#include "csiebox_client.h"
#include <fts.h>

#define DIR_S_FLAG (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) //permission you can use to create new directory
#define REG_S_FLAG (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) //permission you can use to create new file

void client_sync(const char client_dir_path[], const int sock_fd);

int client_sync_dir( const char *path, const csiebox_client *client);

int client_sync_file(const char *path, const csiebox_client *client);

int client_sync_symblink( const char *path, const csiebox_client *client );

int client_sync_hardlink( const char *src,const char *target, const csiebox_client *client );

int client_send_meta( const char *path, const csiebox_client *client);

int client_send_file( const char *path, const csiebox_client *client);

int client_send_symblink( const char *path, const csiebox_client *client );

int client_send_hardlink( const char *src,const char *target, const csiebox_client *client );

int client_rm( const char *path, const csiebox_client *client);

int client_download_meta( const csiebox_protocol_meta req, int conn_fd, const csiebox_client *client );

int client_download_file( int conn_fd, const char *path );

int client_download_symblink( int conn_fd, const char *path );

int client_download_hardlink( const csiebox_protocol_hardlink req, int conn_fd, const csiebox_client *client );

#endif
