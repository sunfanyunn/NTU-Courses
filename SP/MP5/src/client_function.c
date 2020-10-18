#include "client_function.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int client_sync_dir( const char *path, const csiebox_client *client)
{
	fprintf(stderr,"start sync dir %s\n",path);
	csiebox_protocol_status status = client_send_meta( path,client);
	if( status == CSIEBOX_PROTOCOL_STATUS_OK )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

//==================================================================
//							TODO
// You should add shared lock on file that is currently synchronizing
//==================================================================
int client_sync_file(const char *path, const csiebox_client *client)
{
	fprintf(stderr, "start sync file %s\n",path);

	csiebox_protocol_status status = client_send_meta( path,client);
	if( status == CSIEBOX_PROTOCOL_STATUS_OK )
	{
		return 0;
	}	
	else if( status == CSIEBOX_PROTOCOL_STATUS_MORE )
	{
		client_send_file(path, client);
		return 0;
	}
	else
	{
		return -1;
	}
	
}

int client_sync_symblink( const char *path, const csiebox_client *client )
{
	fprintf(stderr, "start sync symbolic %s\n",path);
	csiebox_protocol_status status = client_send_meta( path,client);
	if( status == CSIEBOX_PROTOCOL_STATUS_OK )
	{
		return 0;
	}	
	else if( status == CSIEBOX_PROTOCOL_STATUS_MORE )
	{

		client_send_symblink(path, client);
		return 0;
	}
	else
	{
		return -1;
	}
}

int client_sync_hardlink( const char *src,const char *target, const csiebox_client *client )
{
	fprintf(stderr, "start sync hardlink %s\n",src);

	client_send_hardlink(src, target, client);

}

int client_send_meta( const char *path, const csiebox_client *client )
{
	struct stat s;
	csiebox_protocol_meta req; 
	char *relaPath = path + strlen(client->arg.path);
	memset(&req,0,sizeof(csiebox_protocol_meta));	
	req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
	req.message.header.req.op = CSIEBOX_PROTOCOL_OP_SYNC_META;
	req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);
	req.message.body.pathlen = strlen(relaPath);
	lstat(path,&(req.message.body.stat));
	if( S_ISREG(req.message.body.stat.st_mode ) )
		md5_file(path,req.message.body.hash);
	if( S_ISLNK(req.message.body.stat.st_mode ) )
	{	
		char buf[PATH_MAX];
		int len;
		memset(buf,0,PATH_MAX);
		len = readlink(path,buf,PATH_MAX);
		md5(buf, len, req.message.body.hash );
	} 

	// Send meta req to server
	if (!send_message(client->conn_fd, &req, sizeof(req))) {
		fprintf(stderr, "send req fail\n");
		return -1;
	}

	// Send file path to server
	if (!send_message(client->conn_fd, relaPath, strlen(relaPath))) {
		fprintf(stderr, "send data fail\n");
		return -1;
	}
	
	csiebox_protocol_header header;
	memset(&header, 0, sizeof(header));
	if (recv_message(client->conn_fd, &header, sizeof(header))) {
		if (header.res.magic == CSIEBOX_PROTOCOL_MAGIC_RES &&
			header.res.op == CSIEBOX_PROTOCOL_OP_SYNC_META ) {
				fprintf(stderr, "receive from server: %04x\n",header.res.status );
				return (int) header.res.status;
			} else {
				fprintf(stderr,"receive from server fail\n");
				return -1;
		}
	}
	return -1;
}

int client_send_file( const char *path, const csiebox_client *client)
{
	csiebox_protocol_file req;
	csiebox_protocol_header header;
	int  file_fd;
	char buf[4096];
	int  nByte_read;	

	if( (file_fd = open(path, O_RDONLY)) < 0 )
	{
		fprintf(stderr, "open fail: %s\n",path);
		return -1;
	}
	
	do
	{
		nByte_read = read(file_fd,buf,4096);
		// fill file req
		memset(&req,0,sizeof(csiebox_protocol_file));
		req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
		req.message.header.req.op = CSIEBOX_PROTOCOL_OP_SYNC_FILE;
		req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);
		req.message.body.datalen = nByte_read;
		
		// send file req
		if (!send_message(client->conn_fd, &req, sizeof(req))) {
			fprintf(stderr, "send req fail\n");
			return -1;
		}

		// Send file data to server
		if (nByte_read != 0 && !send_message(client->conn_fd, buf, nByte_read)) {
			fprintf(stderr, "send data fail\n");
			return -1;
		}

		// wait for server response
		memset(&header, 0, sizeof(header));
		if (recv_message(client->conn_fd, &header, sizeof(header))) {
			if (header.res.magic == CSIEBOX_PROTOCOL_MAGIC_RES &&
				header.res.op == CSIEBOX_PROTOCOL_OP_SYNC_FILE 	   &&
				header.res.status == CSIEBOX_PROTOCOL_STATUS_OK ) {

			} else {
					fprintf(stderr, "receive res fail\n");
					return -1;
			}
		}
	}while( nByte_read == 4096);
	close(file_fd);
	fprintf(stderr,"end send file\n");
	return 0;
}

int client_send_symblink( const char *path, const csiebox_client *client )
{
	csiebox_protocol_file req;
	csiebox_protocol_header header;
	char *relaPath = path + strlen(client->arg.path);
	char buf[PATH_MAX];
	int  nByte_read;
	
	memset(buf,0,PATH_MAX);
	if( (nByte_read = readlink(path, buf, PATH_MAX)) < 0 )
	{
		fprintf(stderr, "read link fail: %s\n",path);
		return -1;
	}
	
	// fill symblink req
	memset(&req,0,sizeof(csiebox_protocol_file));
	req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
	req.message.header.req.op = CSIEBOX_PROTOCOL_OP_SYNC_FILE;
	req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);
	req.message.body.datalen = nByte_read;
		
	// send symblink req
	if (!send_message(client->conn_fd, &req, sizeof(req))) {
		fprintf(stderr, "send req fail\n");
		return -1;
	}
	// Send symblink target to server
	if (!send_message(client->conn_fd, buf, nByte_read)) {
		fprintf(stderr, "send data fail\n");
		return -1;
	}	

	// wait for server response
	memset(&header, 0, sizeof(header));
	if (recv_message(client->conn_fd, &header, sizeof(header))) {
		if (header.res.magic == CSIEBOX_PROTOCOL_MAGIC_RES &&
			header.res.op == CSIEBOX_PROTOCOL_OP_SYNC_FILE &&
			header.res.status == CSIEBOX_PROTOCOL_STATUS_OK ) {
		} else {
				fprintf(stderr, "receive res fail\n");
				return -1;
		}
	}
	return 0;
}

int client_send_hardlink( const char *src, const char *target, const csiebox_client *client )
{
	csiebox_protocol_hardlink req;
	csiebox_protocol_header header;
	char *relaPath_src = src + strlen(client->arg.path);
	char *relaPath_target = target + strlen(client->arg.path);
	
	// fill hardlink req
	memset(&req,0,sizeof(csiebox_protocol_hardlink));
	req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
	req.message.header.req.op = CSIEBOX_PROTOCOL_OP_SYNC_HARDLINK;
	req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);
	req.message.body.srclen = strlen(relaPath_src);
	req.message.body.targetlen = strlen(relaPath_target);
	
	// send hardlink req
	if (!send_message(client->conn_fd, &req, sizeof(req))) {
		fprintf(stderr, "send req fail\n");
		return -1;
	}

	// Send hardlink src to server
	if (!send_message(client->conn_fd, relaPath_src, strlen(relaPath_src))) {
		fprintf(stderr, "send data fail\n");
		return -1;
	}
	// Send hardlink target to server
	if (!send_message(client->conn_fd, relaPath_target, strlen(relaPath_target))) {
		fprintf(stderr, "send data fail\n");
		return -1;
	}	

	// wait for server response
	memset(&header, 0, sizeof(header));
	if (recv_message(client->conn_fd, &header, sizeof(header))) {
		if (header.res.magic == CSIEBOX_PROTOCOL_MAGIC_RES &&
			header.res.op == CSIEBOX_PROTOCOL_OP_SYNC_HARDLINK &&
			header.res.status == CSIEBOX_PROTOCOL_STATUS_OK ) {
				fprintf(stderr,"receive res OK, sync hardlink end\n");
				return 0;
		} else {
				fprintf(stderr, "receive res fail\n");
				return -1;
		}
	}
}

int client_rm( const char *path, const csiebox_client *client)
{
	csiebox_protocol_rm req;
	csiebox_protocol_header header;
	char *relaPath = path + strlen(client->arg.path);

	fprintf(stderr,"%s\n",relaPath);
	memset(&req,0,sizeof(req));
	req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
	req.message.header.req.op = CSIEBOX_PROTOCOL_OP_RM;
	req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);
	req.message.body.pathlen = strlen(relaPath);
	if (!send_message(client->conn_fd, &req, sizeof(req))) {
		fprintf(stderr, "send req fail\n");
		return -1;
	}
	if( !send_message(client->conn_fd, relaPath, strlen(relaPath)))
	{
		fprintf(stderr, "send path fail\n");
		return -1;
	}
	memset(&header, 0, sizeof(header));
	if (recv_message(client->conn_fd, &header, sizeof(header))) {
		if (header.res.magic == CSIEBOX_PROTOCOL_MAGIC_RES &&
			header.res.op == CSIEBOX_PROTOCOL_OP_RM &&
			header.res.status == CSIEBOX_PROTOCOL_STATUS_OK ) {
				fprintf(stderr,"receive res OK, rm end\n");
				return 0;
		} else {
				fprintf(stderr, "receive res fail\n");
				return -1;
		}
	}
}

int client_download_meta( const csiebox_protocol_meta req, int conn_fd, const csiebox_client *client )
{
	int pathlen;
	char path[PATH_MAX];
	char hash[MD5_DIGEST_LENGTH];
	struct stat stat;
	csiebox_protocol_header res;
	

	memset(&res,0,sizeof(res));
	res.res.magic = CSIEBOX_PROTOCOL_MAGIC_RES;
	res.res.op = CSIEBOX_PROTOCOL_OP_SYNC_META;
	res.res.datalen = 0;

	memset(path,0,PATH_MAX);
	sprintf(path, "%s",client->arg.path);
	pathlen = req.message.body.pathlen;

	if( !recv_message(conn_fd, path + strlen(client->arg.path), pathlen))
	{
		fprintf(stderr, "recv fail\n");
		return -1;
	}
	fprintf(stderr, "sync %s\n",path);
	if( lstat(path,&stat) < 0 )
	{
		// if the file or directory isn't exist
		if( S_ISDIR(req.message.body.stat.st_mode) )
		{
			fprintf(stderr, "directory not exist\n");
			// if it is a directory, create it and send OK to client
			mkdir( path, DIR_S_FLAG);
			res.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
			send_message(conn_fd, &res, sizeof(res));
		}
		else if( S_ISREG(req.message.body.stat.st_mode) )
		{
			// if it is a regular file, create it and send MORE to client
			fprintf(stderr, "file not exist\n");
			res.res.status = CSIEBOX_PROTOCOL_STATUS_MORE;
			send_message(conn_fd, &res, sizeof(res));
			if( client_download_file(conn_fd,path) < 0 )
			{
				fprintf(stderr,"sync file fail\n");
				return -1;
			}
		}
		else if( S_ISLNK(req.message.body.stat.st_mode))
		{
			fprintf(stderr, "sLink not exist\n");
			// if it is a symbolic link, send MORE to client
			res.res.status = CSIEBOX_PROTOCOL_STATUS_MORE;
			send_message(conn_fd, &res, sizeof(res));
			if( client_download_symblink(conn_fd,path) < 0 )
			{
				fprintf(stderr,"sync slink fail\n");
				return -1;
			}
		}
	}
	else
	{
		// if the file or directory exist
		if( S_ISDIR(req.message.body.stat.st_mode) )
		{
			fprintf(stderr, "directory exist\n");
			// if it is a directory, send OK to client
			res.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
			send_message(conn_fd, &res, sizeof(res));
		}
		else if( S_ISREG(req.message.body.stat.st_mode) )
		{
			fprintf(stderr, "file exist\n");
			// if it is a regular file, compare has to see if it is modified
			md5_file(path, hash);
			if( memcmp(req.message.body.hash,hash, MD5_DIGEST_LENGTH ) == 0 )
			{
				// if not modified, send OK to client
				fprintf(stderr, "file not modified\n");
				res.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
				send_message(conn_fd,&res,sizeof(res));
			}
			else
			{
				// the file has been modified, synchronization is needed
				fprintf(stderr,"file modified\n");
				res.res.status = CSIEBOX_PROTOCOL_STATUS_MORE;
				send_message(conn_fd, &res, sizeof(res));
				if( client_download_file(conn_fd,path) < 0 )
				{
					fprintf(stderr,"sync file fail\n");
					return -1;
				}
			}
		}
		else if( S_ISLNK(req.message.body.stat.st_mode) )
		{
			fprintf(stderr, "sLink exist\n");
			char buf[PATH_MAX];
			uint8_t hash[MD5_DIGEST_LENGTH];
			int length;
			
			memset(buf,0,PATH_MAX);
			memset(hash,0,MD5_DIGEST_LENGTH);
			length = readlink( path, buf, PATH_MAX);
			md5(buf,length,hash);
			if( memcmp(req.message.body.hash, hash, MD5_DIGEST_LENGTH ) == 0 )
			{
				fprintf(stderr, "link not modified\n");
				res.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
				send_message(conn_fd,&res,sizeof(res));
			}
			else
			{
				fprintf(stderr, "link modified\n");
				res.res.status = CSIEBOX_PROTOCOL_STATUS_MORE;
				send_message(conn_fd, &res, sizeof(res));
				if( client_download_symblink(conn_fd,path) < 0 )
				{
					fprintf(stderr, "sync slink fail\n");
					return -1;
				}
			}
			
		}
	}
	// sync stat
	fprintf(stderr,"sycn_meta\n");
	struct timespec timebuf[2];
	memset(&timebuf,0,sizeof(timebuf));
	timebuf[0].tv_sec = req.message.body.stat.st_atime;
	timebuf[1].tv_sec = req.message.body.stat.st_mtime;
	utimensat(AT_FDCWD,path,timebuf,AT_SYMLINK_NOFOLLOW);
	if( ! S_ISLNK(req.message.body.stat.st_mode) )
	{		
		chmod(path,req.message.body.stat.st_mode);
	}
	lchown( path, req.message.body.stat.st_uid, req.message.body.stat.st_gid);

	return 0;
	
}

int client_download_file( int conn_fd, const char *path )
{
	int datalen;
	char buf[4096];
	csiebox_protocol_file file;
	csiebox_protocol_header res; 
		
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, REG_S_FLAG );
	if( fd < 0 )
	{
		fprintf(stderr, "open file fail\n");
		return -1;
	}

	do
	{
		memset(&file,0,sizeof(file));
		if( !recv_message(conn_fd, &file, sizeof(file)))
		{
			fprintf(stderr,"recv fail\n");
			break;
		}
		if (file.message.header.req.magic != CSIEBOX_PROTOCOL_MAGIC_REQ) {
			break;
		}
		if( file.message.header.req.op != CSIEBOX_PROTOCOL_OP_SYNC_FILE)
		{
			break;
		}
		datalen = file.message.body.datalen;
		memset(buf,0,4096);
		fprintf( stderr,"nByte_sent=%d\n",datalen);
		if( datalen != 0 && !recv_message(conn_fd, buf,datalen) )
		{
			fprintf(stderr, "recv file data fail\n");
			memset(&res,0, sizeof(res));
			res.res.magic = CSIEBOX_PROTOCOL_MAGIC_RES;
			res.res.op = CSIEBOX_PROTOCOL_OP_SYNC_FILE;
			res.res.datalen = 0;
			res.res.status = CSIEBOX_PROTOCOL_STATUS_FAIL;
			send_message(conn_fd, &res, sizeof(res));	
			break;
		}
		write( fd, buf, datalen);
		memset(&res,0, sizeof(res));
		res.res.magic = CSIEBOX_PROTOCOL_MAGIC_RES;
		res.res.op = CSIEBOX_PROTOCOL_OP_SYNC_FILE;
		res.res.datalen = 0;
		res.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
		send_message(conn_fd, &res, sizeof(res));
	}while( datalen == 4096 );
	close(fd);
	return 0;		
}

int client_download_symblink( int conn_fd, const char *path )
{
	int datalen;
	char buf[4096];
	csiebox_protocol_file file;
	csiebox_protocol_header res; 
	memset(&file,0,sizeof(file));
	if( !recv_message(conn_fd, &file,sizeof(file)))
	{
		fprintf(stderr,"recv fail\n");
		return -1;
	} 
	if( file.message.header.req.magic != CSIEBOX_PROTOCOL_MAGIC_REQ)
	{
		return -1;
	}			
	if( file.message.header.req.op != CSIEBOX_PROTOCOL_OP_SYNC_FILE)
	{
		return -1;
	}
	datalen = file.message.body.datalen;
	memset(buf,0,4096);
	if( !recv_message(conn_fd,buf,datalen))
	{
		memset(&res,0, sizeof(res));
		res.res.magic = CSIEBOX_PROTOCOL_MAGIC_RES;
		res.res.op = CSIEBOX_PROTOCOL_OP_SYNC_FILE;
		res.res.datalen = 0;
		res.res.status = CSIEBOX_PROTOCOL_STATUS_FAIL;
		send_message(conn_fd, &res, sizeof(res));
		fprintf(stderr,"recv data fail\n");
		return -1;
	}

	if( symlink( buf, path) < 0)
	{
		fprintf(stderr,"symlink fail\n");
		return -1;
	}

	memset(&res,0, sizeof(res));
	res.res.magic = CSIEBOX_PROTOCOL_MAGIC_RES;
	res.res.op = CSIEBOX_PROTOCOL_OP_SYNC_FILE;
	res.res.datalen = 0;
	res.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
	send_message(conn_fd, &res, sizeof(res));
	return 0;
}

int client_download_hardlink( const csiebox_protocol_hardlink req, int conn_fd, const csiebox_client *client )
{
	int srclen, targetlen;
	char buf1[PATH_MAX],buf2[PATH_MAX];
	char src[PATH_MAX],target[PATH_MAX];
	csiebox_protocol_header res;

	memset(&res,0, sizeof(res));
	res.res.magic = CSIEBOX_PROTOCOL_MAGIC_RES;
	res.res.op = CSIEBOX_PROTOCOL_OP_SYNC_HARDLINK;
	res.res.datalen = 0;

	srclen = req.message.body.srclen;
	targetlen = req.message.body.targetlen;
	
	memset(buf1,0,PATH_MAX);
	if( !recv_message(conn_fd, buf1,srclen) )
	{
		fprintf(stderr, "recv src fail\n");
		res.res.status = CSIEBOX_PROTOCOL_STATUS_FAIL;
		send_message(conn_fd, &res, sizeof(res));	
		return -1;
	}

	memset(buf2,0,PATH_MAX);
	if( !recv_message(conn_fd, buf2,targetlen) )
	{
		fprintf(stderr, "recv target fail\n");
		res.res.status = CSIEBOX_PROTOCOL_STATUS_FAIL;
		send_message(conn_fd, &res, sizeof(res));	
		return -1;
	}

	memset(src,0,PATH_MAX);
	memset(target,0,PATH_MAX);
	sprintf(src,"%s/%s",client->arg.path,buf1);
	sprintf(target,"%s/%s",client->arg.path,buf2);
	fprintf(stderr,"linking %s, %s\n",src, target);
	if( access(src,F_OK) == 0 && access(target,F_OK) == 0 )
	{
		// if both file exist, remove old src and create new one linking to target	
		unlink(src);
		link(target,src);
		res.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
	}
	else if( access(target,F_OK) < 0)
	{
		// if target not exist, send fail to client
		res.res.status = CSIEBOX_PROTOCOL_STATUS_FAIL;
	}
	else if( access(src, F_OK) < 0 && access(target,F_OK) == 0 )
	{
		link( target,src);
		res.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
	}
	res.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
	send_message(conn_fd, &res, sizeof(res));
	return 0;
}


