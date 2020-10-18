#include "csiebox_client.h"
#include "client_function.h"
#include "csiebox_common.h"
#include "connect.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

static int parse_arg(csiebox_client* client, int argc, char** argv);
static int login(csiebox_client* client);
static int download_from_server(csiebox_client *client);

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

//==================================================================
//							TODO
// You should add shared lock on file that is currently synchronizing
//==================================================================
int csiebox_client_run(csiebox_client* client) {
  if (!login(client)) {
    fprintf(stderr, "login fail\n");
    return 0;
  }
  fprintf(stderr, "login success\n");
  
	int inotify_fd;
	char **corres;

	inotify_fd = inotify_init();
	if( inotify_fd < 0 )
	{
		fprintf(stderr,"inotify_init fail\n");
		return -1;
	}
	
	corres = malloc(sizeof(char *) * 128);

	//==============================
	//		Sync files with server
	//==============================
	FTS *fts;
	FTSENT *ftsent_head;
	FTSENT *p1,*p2;
	struct dirent *dir;
	DIR *Dir;
	char path[PATH_MAX];
	char *a[] = {client->arg.path,NULL};
	int wd;
	int numFD = 0;

	if( !(Dir = opendir(client->arg.path)))
	{
		fprintf(stderr,"opendir fail: %s\n",client->arg.path);
		return -1;
	}
	while( dir = readdir(Dir) )
	{
		fprintf(stderr,"%s\n",dir->d_name);
		numFD++;
	}
	closedir(Dir);
	fprintf(stderr,"Num of file and dir: %d\n",numFD);

	// if cdir is empty, download from server
	download_from_server(client);

	fts = fts_open( a, FTS_PHYSICAL | FTS_NOCHDIR, NULL);
	if( fts == NULL )
	{
		fprintf(stderr, "fts fail\n");
		return -1;
	}
	
	while( ( ftsent_head = fts_read(fts)) != NULL )
	{
		ftsent_head = fts_children( fts,0);
		if( ftsent_head != NULL )
		{
			wd = inotify_add_watch( inotify_fd, ftsent_head->fts_path, IN_CREATE | IN_DELETE | IN_ATTRIB | IN_MODIFY );
			corres[wd] = malloc(PATH_MAX);
			memset(corres[wd],0,PATH_MAX);
			strncpy(corres[wd],ftsent_head->fts_path,strlen(ftsent_head->fts_path));			
			fprintf(stderr,"add %s to inotify\n", ftsent_head->fts_path);	
		}
		for( p1 = ftsent_head; p1 != NULL; p1 = p1->fts_link )
		{		
			memset(path,0,PATH_MAX);
			sprintf(path,"%s/%s",p1->fts_path,p1->fts_name);
			// if the file is hidden, ignore it
			if( p1->fts_name[0] == '.' )
			{
				continue;
			}
			if( strcmp(path, client->arg.path) ==0 )
			{
				continue;
			}
			switch( p1->fts_info )
			{
				case FTS_D:
					client_sync_dir(path,client);
					break;
				case FTS_F:
					if( p1->fts_statp->st_nlink == 1)
					{	
						// there is no hard link
						client_sync_file(path,client);
					}
					else
					{
						// there might be hard link
						p2 = ftsent_head;
						char path2[PATH_MAX];
						while(p2 != p1 )
						{
							memset(path2,0,PATH_MAX);
							sprintf(path2,"%s/%s",p2->fts_path,p2->fts_name);
							if( p2->fts_statp->st_ino == p1->fts_statp->st_ino )
							{
								// if it is a hardlink
								client_sync_hardlink( path, path2, client);
								break;
							}
							p2 = p2->fts_link;
						}
						if( p2 == p1 )
							client_sync_file(path,client);
					}				
					break;
				case FTS_SL:
					client_sync_symblink(path,client);
					break;
				case FTS_SLNONE:
					client_sync_symblink(path,client);
					break;
				default:
					fprintf(stderr,"Unknown type of fts_info\n");
					break;
			}
		}
	}

	//===============================================
	//		Monitoring file change with inotify
	//===============================================
	int length, i;
	char buffer[EVENT_BUF_LEN];
	
	while ((length = read(inotify_fd, buffer, EVENT_BUF_LEN)) > 0) {
    	i = 0;
    	while (i < length) {
      		struct inotify_event* event = (struct inotify_event*)&buffer[i];
			memset(path,0,PATH_MAX);
			sprintf(path,"%s/%s",corres[event->wd], event->name);
      		fprintf(stderr,"%s->%d, %s->%d\n", corres[event->wd], strlen(corres[event->wd]), event->name, strlen(event->name));
			if( event->mask & IN_ISDIR )
			{
				if (event->mask & IN_CREATE ) {
        			fprintf(stderr,"create dir %s\n",path);
					wd = inotify_add_watch( inotify_fd, path, IN_CREATE | IN_DELETE | IN_ATTRIB | IN_MODIFY );
					corres[wd] = malloc(strlen(path));
					strncpy(corres[wd],path,strlen(path));			
					fprintf(stderr,"add %s to inotify\n", path);
					client_sync_dir(path,client);
      			}
      			if (event->mask & IN_DELETE) {
        			fprintf(stderr,"delete dir %s\n",path);
					client_rm(path,client);
	       		}
     	 		if (event->mask & IN_ATTRIB ) {
        			fprintf(stderr,"attrib dir %s\n",path);
					client_sync_dir(path,client);
      			}
      			if (event->mask & IN_MODIFY) {
        			fprintf(stderr,"modify dir %s\n",path);
					client_sync_dir(path,client);
      			}
			}
			else{
				if (event->mask & IN_CREATE) {
        			fprintf(stderr,"create file %s\n",path);
					client_sync_file(path,client);
      			}
      			if (event->mask & IN_DELETE) {
        			fprintf(stderr,"delete file %s\n",path);
					client_rm(path,client);
	       		}
     	 		if (event->mask & IN_ATTRIB) {
        			fprintf(stderr,"attrib file %s\n",path);
					client_sync_file(path,client);
      			}
      			if (event->mask & IN_MODIFY) {
        			fprintf(stderr,"modify file %s\n",path);
					client_sync_file(path,client);
      			}
			}
      		i += EVENT_SIZE + event->len;
    	}
		fsync(1);
		fsync(2);
    	memset(buffer, 0, EVENT_BUF_LEN);
  	}
	

  
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
    fprintf(stderr, "config (%d, %s)=(%d, %s)\n", keylen, key, vallen, val);
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
		client->arg.path[vallen] = '\0';
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

static int download_from_server(csiebox_client *client)
{
	csiebox_protocol_header header;
  	memset(&header, 0, sizeof(header));
	while(1){
  		recv_message(client->conn_fd, &header, sizeof(header));
  		if (header.req.magic != CSIEBOX_PROTOCOL_MAGIC_REQ) {
			return;
		}
		if (header.req.op == CSIEBOX_PROTOCOL_OP_SYNC_END )	{
			fprintf(stderr,"download from server end\n");			
			break;		
		}
		switch (header.req.op) {
    		case CSIEBOX_PROTOCOL_OP_SYNC_META:
      			fprintf(stderr, "sync meta\n");
      			csiebox_protocol_meta meta;
      			if (complete_message_with_header(client->conn_fd, &header, &meta)) {
        			//====================
        			//        TODO
        			//====================
					client_download_meta(meta, client->conn_fd, client);
				}
      			break;
   			case CSIEBOX_PROTOCOL_OP_SYNC_HARDLINK:
      			fprintf(stderr, "sync hardlink\n");
      			csiebox_protocol_hardlink hardlink;
      			if (complete_message_with_header(client->conn_fd, &header, &hardlink)) {
        			//====================
        			//        TODO
        			//====================
					client_download_hardlink( hardlink, client->conn_fd, client);
      			}
      			break;
    		default:
      			fprintf(stderr, "unknown op %x\n", header.req.op);
      			break;
		}
  }
}
