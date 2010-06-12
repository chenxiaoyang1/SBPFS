// SBPFS -  DFS
//
// Copyright (C) 2009-2010, SBPFS Developers Team
//
// SBPFS is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// SBPFS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SBPFS; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA

#include "sbpfs.h"
#include "lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
s32_t sbp_opendir(char* filename){
	s32_t fd = get_slot();
	struct sbpfs_head head;
	char* usr;
	char* pass;
	char* data;
	char* rec_data;
	char tran_usr[TRAN_USERNAME_LEN];
	u64_t rec_len = 0;
	int data_len = 0;
	head.data = NULL;
	head.title = PROTOCOL;
	head.entry_num = 0;
	if (sbp_getUandP(&usr, &pass) == -1) {
		seterr(DATA_ERR, U_AND_P);
		return -1;
	}
	sprintf(tran_usr, "Client_%s", usr);
	mkent(head,USER,tran_usr);
	mkent(head,PASS,pass);
	mkent(head,METHOD,"OPENDIR");
	mkent(head,ARGC,"1");
	mkent(head,"Arg0",filename);
	mkent(head,CONTENT_LEN,"0");

	make_head(&data, &data_len, &head);
	free(usr);
	free(pass);
	if (sendrec_hostname(sbp_host, CNODE_SERVICE_PORT, data, data_len, &rec_data,
			&rec_len) != 0) {
		goto err_exit;
	}

	if (strlen(rec_data) == 0) {
		seterr(SOCKET_ERR,RECV);
		return -1;
	}
	if (decode_head(rec_data, rec_len, &head) == -1) {
		seterr(DATA_ERR, DECODE_HEAD);
		goto err_exit2;
	}
	if (strncmp(head.title, REQUEST_OK, strlen(REQUEST_OK)) == 0) {
		if((fds[fd] = (struct sbp_filedesc *)malloc(sizeof(struct sbp_filedesc)))==NULL)
		{
			seterr(MEM_ERR,MALLOC);
			goto err_exit3;
		}
		bzero(fds[fd],sizeof(struct sbp_filedesc));
		if((fds[fd]->filename = (char *)malloc(sizeof(strlen(filename)+1)))==NULL)
		{
			seterr(MEM_ERR,MALLOC);
			goto err_exit4;
		}
		memcpy(fds[fd]->filename,filename,strlen(filename));
		fds[fd]->filename[strlen(filename)] = 0;
		fds[fd]->offset = 0;
		fds[fd]->server_fd = atoll(get_head_entry_value(&head, FILE_DESC));
		char* auth_code = get_head_entry_value(&head,AUTH_CODE);
		if(auth_code==NULL)
		{
			seterr(DATA_ERR,AUTH_LEN);
			goto err_exit4;
		}
		int auth_len = strlen(auth_code);
		if(auth_len > AUTH_CODE_LEN)
		{
			seterr(DATA_ERR,AUTH_LEN);
			goto err_exit4;
		}
		memcpy(fds[fd]->auth_code,auth_code,auth_len);

		goto ok_exit;
	} else if (strncmp(head.title, REQUEST_ERR, strlen(REQUEST_ERR)) == 0) {
		sbp_update_err(&head);
		goto err_exit3;
	} else {
		seterr(HEAD_ERR, UNKNOWN_HEAD);
	}

	err_exit4: sbp_close(fd);

	err_exit3: free_head(&head);
	err_exit2: free(data);
	free(rec_data);
	err_exit: return -1;
	ok_exit: free(data);
	free(rec_data);
	free_head(&head);
	return fd;

}
s32_t sbp_readdir(u32_t dirfd){
	if (fds[dirfd] == NULL){
		return -1;
	}
	return 0;
}


s32_t sbp_mkdir(char* filename){
	struct sbpfs_head head;
	char* usr;
	char* pass;
	char* data;
	char* rec_data;
	char tran_usr[TRAN_USERNAME_LEN];
	u64_t rec_len = 0;
	int data_len = 0;
	head.data = NULL;
	head.title = PROTOCOL;
	head.entry_num = 0;
	if (sbp_getUandP(&usr, &pass) == -1) {
		printf("Can not get username and password\n");
		return -1;
	}
	sprintf(tran_usr, "Client_%s", usr);
	mkent(head,USER,tran_usr);
	mkent(head,PASS,pass);
	mkent(head,METHOD,"MKDIR");
	mkent(head,ARGC,"1");
	mkent(head,"Arg0",filename);
	mkent(head,CONTENT_LEN,"0");

	make_head(&data, &data_len, &head);
	free(usr);
	free(pass);
	if (sendrec_hostname(sbp_host, CNODE_SERVICE_PORT, data, data_len, &rec_data,
			&rec_len) != 0) {
		goto err_exit;
	}

	if (strlen(rec_data) == 0) {
		printf("Return len == 0\n");
		return 0;
	}
	if (decode_head(rec_data, rec_len, &head) == -1) {
		seterr("Data Error", "Can not decode SBPFS_HEAD");
		goto err_exit2;
	}
	if (strncmp(head.title, REQUEST_OK, strlen(REQUEST_OK)) == 0) {
		goto ok_exit;
	} else if (strncmp(head.title, REQUEST_ERR, strlen(REQUEST_ERR)) == 0) {
		sbp_update_err(&head);
		goto err_exit3;
	} else {

	}
	err_exit3: free_head(&head);
	err_exit2: free(data);
	free(rec_data);
	err_exit: return -1;
	ok_exit: free(data);
	free(rec_data);
	free_head(&head);
	return 0;
}

