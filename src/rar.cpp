/** \file
    RAR-related functions

    Copyright (C) 2007 - 2008 Jeffrey P.

	This file is licensed under the UnRAR license. You may not use it to
	recreate the compression algorithm used in the Roshal Archive (RAR)
	format. For more information, see the file rar/license.txt.
**/
#define PSPComic_rar_cpp
#include "rar.hpp"

void* rar_open(char *filename, unsigned int om) {
	RAROpenArchiveData arc_open;
	arc_open.ArcName = make_string(filename);
	arc_open.OpenMode = om;
	arc_open.CmtBuf = NULL;
	arc_open.CmtBufSize = 0;
	HANDLE rar_file = RAROpenArchive(&arc_open);
	if(arc_open.OpenResult != 0) return NULL;
	free(arc_open.ArcName);
	return rar_file;
}

int rar_close(void *rar_file) {
	return RARCloseArchive(rar_file);
}

int rar_get_header(void *rar_file, struct RARHeaderData *HeaderData) {
	return RARReadHeader(rar_file,HeaderData);
}

int rar_next_file(void *rar_file) {
	return RARProcessFile(rar_file,RAR_SKIP,NULL,NULL);
}

int rar_get_file_by_name(char *arc_file, char *filename, char **data, size_t *size) {
	RARHeaderData hd;
	memset(&hd,0,sizeof(RARHeaderData));
	HANDLE rar_file = rar_open(arc_file,RAR_OM_EXTRACT);
	if(!rar_file) return -1;
	int status;
	while((status = rar_get_header(rar_file,&hd)) == 0) {
		char* temp = (char*)&hd.FileName;
		if(cb_compare(&filename,&temp) == 0) break;
		rar_next_file(rar_file);
	}
	if(status) {
		rar_close(rar_file);
		return status;
	}
	rar_callback_info cb_info;
	cb_info.buffer = data;
	*(cb_info.buffer) = (char*)malloc(sizeof(char)*hd.UnpSize);
	char *orig_data = *data;
	cb_info.size = hd.UnpSize;
	*size = hd.UnpSize;
	RARSetCallback(rar_file,rar_file_callback,(long)&cb_info);
	
	status = RARProcessFile(rar_file,RAR_TEST,NULL,NULL);
	rar_close(rar_file);
	if(status) {
		free(orig_data);
		*data = NULL;
	}
	else *data = orig_data;
	return status;
}

int CALLBACK rar_file_callback(UINT msg,LONG UserData,LONG P1,LONG P2) {
	switch(msg) {
		case UCM_CHANGEVOLUME:
			if(P2 == RAR_VOL_NOTIFY) return 0;
			return -1;
		break;
		case UCM_PROCESSDATA: {
			rar_callback_info *cb_info = (rar_callback_info*)UserData;
			char **buffer = cb_info->buffer;
			if((P2 >= 0) && ((unsigned long) P2 > cb_info->size)) return -1;
			memcpy(*buffer,(char*)P1,P2);
			*buffer += P2;
			cb_info->size -= P2;
			return 0;
		}
		break;
		default: return -1;
	}
}
