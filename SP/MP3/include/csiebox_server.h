#ifndef _CSIEBOX_SERVER_
#define _CSIEBOX_SERVER_

#ifdef __cplusplus
extern "C" {
#endif

#include "csiebox_common.h"

#include <limits.h>

typedef struct {
  char user[USER_LEN_MAX];
  char passwd_hash[MD5_DIGEST_LENGTH];
} csiebox_account_info;

typedef struct {
  csiebox_account_info account;
  int conn_fd;
} csiebox_client_info;

typedef struct {
  struct {
    char path[PATH_MAX];
    char account_path[PATH_MAX];
  } arg;
  int listen_fd;
  csiebox_client_info** client;
} csiebox_server;

void csiebox_server_init(
  csiebox_server** server, int argc, char** argv);
int csiebox_server_run(csiebox_server* server);
void csiebox_server_destroy(csiebox_server** server);

#ifdef __cplusplus
}
#endif

#endif
