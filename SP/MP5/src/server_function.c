#include "server_function.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <errno.h>

//=======================================================================
//        						TODO
// You should add exclusive lock on file that is currenting synchronizing
//=======================================================================
int server_sync_meta( const csiebox_protocol_meta req, int conn_fd, const csiebox_server *server )
{
	int pathlen;
	char path[PATH_MAX];
	char hash[MD5_DIGEST_LENGTH];  // hash of file on server, to be compared with that from client
	char *userName;
	struct stat stat;
	csiebox_protocol_header res;
	

	memset(&res,0,sizeof(res));
	res.res.magic = CSIEBOX_PROTOCOL_MAGIC_RES;
	res.res.op = CSIEBOX_PROTOCOL_OP_SYNC_META;
	res.res.datalen = 0;

	userName = server->client[conn_fd]->account.user;
	pathlen = req.message.body.pathlen;

	memset(path,0,PATH_MAX);
	sprintf(path, "%s/%s",server->arg.path,userName);

	if( !recv_message(conn_fd, path + strlen(server->arg.path) + strlen(userName)+1, pathlen))
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
			if( server_sync_file(conn_fd,path) < 0 )
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
			if( server_sync_symblink(conn_fd,path) < 0 )
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
				if( server_sync_file(conn_fd,path) < 0 )
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
				if( server_sync_symblink(conn_fd,path) < 0 )
				{
					fprintf(stderr, "sync slink fail\n");
					return -1;
				}
			}
			
		}
	}
	// sync stat
	fprintf(stderr,"sync_meta\n");
	struct timespec timebuf[2];
	memset(&timebuf,0,sizeof(timebuf));
	timebuf[0].tv_sec = req.message.body.stat.st_atime;
	timebuf[1].tv_sec = req.message.body.stat.st_mtime;
	utimensat(AT_FDCWD,path,timebuf,AT_SYMLINK_NOFOLLOW);
	if( ! S_ISLNK(req.message.body.stat.st_mode) )
	{		
		chmod(path,req.message.body.stat.st_mode);
	}

	return 0;
	
}

int server_sync_file( int conn_fd, const char *path )
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
			fprintf(stderr,"recv magic wrong\n");
			break;
		}
		if( file.message.header.req.op != CSIEBOX_PROTOCOL_OP_SYNC_FILE)
		{
			fprintf(stderr,"recv op wrong\n");
			fprintf(stderr,"recv %x\n",file.message.header.req.op);
			break;
		}
		datalen = file.message.body.datalen;
		memset(buf,0,4096);
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

int server_sync_symblink( int conn_fd, const char *path )
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

int server_sync_hardlink( const csiebox_protocol_hardlink req, int conn_fd, const csiebox_server *server )
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
	sprintf(src,"%s/%s%s",server->arg.path,server->client[conn_fd]->account.user,buf1);
	sprintf(target,"%s/%s%s",server->arg.path,server->client[conn_fd]->account.user,buf2);
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

int server_rm( const csiebox_protocol_rm req, int conn_fd, const csiebox_server *server)
{
	char buf[PATH_MAX];
	char path[PATH_MAX];
	csiebox_protocol_header res;
	struct stat stat;	

	memset(&res,0, sizeof(res));
	res.res.magic = CSIEBOX_PROTOCOL_MAGIC_RES;
	res.res.op = CSIEBOX_PROTOCOL_OP_RM;
	res.res.datalen = 0;

	memset(buf,0,PATH_MAX);
	if( !recv_message( conn_fd, buf, req.message.body.pathlen) )
	{
		fprintf(stderr, "recv path fail\n");
		return -1;
	}

	memset(path,0,PATH_MAX);
	sprintf(path, "%s/%s%s", server->arg.path,server->client[conn_fd]->account.user, buf);
	
	fprintf(stderr, "%s\n",path);
	memset(&stat, 0, sizeof(stat));
	if( lstat(path, &stat) < 0 )
	{
		// file not exist, return fail
		res.res.status = CSIEBOX_PROTOCOL_STATUS_FAIL;
		send_message( conn_fd, &res, sizeof(res));
		return -1;
	}
	
	if( S_ISDIR( stat.st_mode))
	{
		rmdir(path);
	}
	else
	{
		unlink(path);
	}
	
	res.res.status = CSIEBOX_PROTOCOL_STATUS_OK;
	send_message(conn_fd, &res, sizeof(res));
	return 0;
}

int server_send_meta( const char *path, int conn_fd, const csiebox_server *server )
{
	struct stat s;
	csiebox_protocol_meta req; 
	char *relaPath = path + strlen(server->arg.path)+strlen(server->client[conn_fd]->account.user)+1;
	char cwd[PATH_MAX];
	getcwd(cwd,PATH_MAX);
	fprintf(stderr,"cwd: %s\n",cwd);
	memset(&req,0,sizeof(csiebox_protocol_meta));	
	req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
	req.message.header.req.op = CSIEBOX_PROTOCOL_OP_SYNC_META;
	req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);
	fprintf(stderr,"%s\n",relaPath);
	fprintf(stderr,"%s->%d\n",path,strlen(path));
	req.message.body.pathlen = strlen(relaPath);
	if( lstat(path,&(req.message.body.stat)) == -1 )
	{
		if( errno == EACCES )
		{
			fprintf(stderr,"lstat fail: EACCES\n");
		}
		if( errno == EBADF )
		{
			fprintf(stderr,"lstat fail: EBADF\n");
		}
		if( errno == EFAULT )
		{
			fprintf(stderr,"lstat fail: EFAULT\n");
		}
		if( errno == ELOOP )
		{
			fprintf(stderr,"lstat fail: ELOOP\n");
		}
		if( errno == ENAMETOOLONG )
		{
			fprintf(stderr,"lstat fail: ENAMETOOLONG\n");
		}
		if( errno == ENOENT )
		{
			fprintf(stderr,"lstat fail: ENOENT\n");
		}
		if( errno == ENOMEM )
		{
			fprintf(stderr,"lstat fail: ENOMEM\n");
		}
		if( errno ==ENOTDIR )
		{
			fprintf(stderr,"lstat fail: ENOTDIR\n");
		}
		if( errno == EOVERFLOW )
		{
			fprintf(stderr,"lstat fail: EOVERFLOW\n");
		}
	
	}
	if( S_ISREG(req.message.body.stat.st_mode ) )
	{
		fprintf(stderr,"regular file\n");
		md5_file(path,req.message.body.hash);
	}
	else if( S_ISDIR(req.message.body.stat.st_mode)) 
	{
		fprintf(stderr,"directory\n");
	}
	else if( S_ISLNK(req.message.body.stat.st_mode))
	{
		fprintf(stderr,"soft link\n");		
	}
	else if( S_ISCHR(req.message.body.stat.st_mode))
	{
		fprintf(stderr,"char device\n");	
	}
	else if( S_ISBLK(req.message.body.stat.st_mode))
	{
		fprintf(stderr,"block device");	
	}
	else if( S_ISFIFO(req.message.body.stat.st_mode))
	{
		fprintf(stderr,"FIFO\n");
	}
	else if( S_ISSOCK(req.message.body.stat.st_mode))
	{
		fprintf(stderr,"socket %x\n",req.message.body.stat.st_mode & S_IFMT);	
	}
	else
	{
		fprintf(stderr,"unknown %x\n",req.message.body.stat.st_mode & S_IFMT);	
	}
	if( S_ISLNK(req.message.body.stat.st_mode ) )
	{	
		char buf[PATH_MAX];
		int len;
		memset(buf,0,PATH_MAX);
		len = readlink(path,buf,PATH_MAX);
		md5(buf, len, req.message.body.hash );
	} 

	// Send meta req to server
	if (!send_message(conn_fd, &req, sizeof(req))) {
		fprintf(stderr, "send req fail\n");
		return -1;
	}

	// Send file path to server
	if (!send_message(conn_fd, relaPath, strlen(relaPath))) {
		fprintf(stderr, "send data fail\n");
		return -1;
	}
	
	csiebox_protocol_header header;
	memset(&header, 0, sizeof(header));
	if (recv_message(conn_fd, &header, sizeof(header))) {
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

int server_send_file( const char *path, int conn_fd, const csiebox_server *server)
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
		fprintf(stderr, "sending req\n");
		if (!send_message(conn_fd, &req, sizeof(req))) {
			fprintf(stderr, "send req fail\n");
			return -1;
		}

		// Send file data to server
		if (nByte_read != 0 && !send_message(conn_fd, buf, nByte_read)) {
			fprintf(stderr, "send data fail\n");
			return -1;
		}

		// wait for server response
		memset(&header, 0, sizeof(header));
		if (recv_message(conn_fd, &header, sizeof(header))) {
			if (header.res.magic == CSIEBOX_PROTOCOL_MAGIC_RES &&
				header.res.op == CSIEBOX_PROTOCOL_OP_SYNC_FILE 	   &&
				header.res.status == CSIEBOX_PROTOCOL_STATUS_OK ) {
					fprintf(stderr, "receive res OK\n");
			} else {
					fprintf(stderr, "receive res fail\n");
					return -1;
			}
		}
	}while( nByte_read == 4096);
	fprintf(stderr,"end send file\n");
	return 0;
}

int server_send_symblink( const char *path, int conn_fd, const csiebox_server *server )
{
	csiebox_protocol_file req;
	csiebox_protocol_header header;
	char *relaPath = path + strlen(server->arg.path) + strlen(server->client[conn_fd]->account.user) + 1;
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
	if (!send_message(conn_fd, &req, sizeof(req))) {
		fprintf(stderr, "send req fail\n");
		return -1;
	}
	// Send symblink target to server
	if (!send_message(conn_fd, buf, nByte_read)) {
		fprintf(stderr, "send data fail\n");
		return -1;
	}	

	// wait for server response
	memset(&header, 0, sizeof(header));
	if (recv_message(conn_fd, &header, sizeof(header))) {
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

int server_send_hardlink( const char *src, const char *target, int conn_fd, const csiebox_server *server )
{
	csiebox_protocol_hardlink req;
	csiebox_protocol_header header;
	char *relaPath_src = src + strlen(server->arg.path) + strlen(server->client[conn_fd]->account.user) + 1;
	char *relaPath_target = target + strlen(server->arg.path) + strlen(server->client[conn_fd]->account.user) + 1;
	
	// fill hardlink req
	memset(&req,0,sizeof(csiebox_protocol_hardlink));
	req.message.header.req.magic = CSIEBOX_PROTOCOL_MAGIC_REQ;
	req.message.header.req.op = CSIEBOX_PROTOCOL_OP_SYNC_HARDLINK;
	req.message.header.req.datalen = sizeof(req) - sizeof(req.message.header);
	req.message.body.srclen = strlen(relaPath_src);
	req.message.body.targetlen = strlen(relaPath_target);
	
	// send hardlink req
	if (!send_message(conn_fd, &req, sizeof(req))) {
		fprintf(stderr, "send req fail\n");
		return -1;
	}

	// Send hardlink src to server
	if (!send_message(conn_fd, relaPath_src, strlen(relaPath_src))) {
		fprintf(stderr, "send data fail\n");
		return -1;
	}
	// Send hardlink target to server
	if (!send_message(conn_fd, relaPath_target, strlen(relaPath_target))) {
		fprintf(stderr, "send data fail\n");
		return -1;
	}	

	// wait for server response
	memset(&header, 0, sizeof(header));
	if (recv_message(conn_fd, &header, sizeof(header))) {
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



