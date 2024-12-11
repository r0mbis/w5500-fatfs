/*
* Wiznet.
* (c) Copyright 2002, Wiznet.
*
* Filename	: ftpd.c
* Version	: 1.0
* Programmer(s)	: 
* Created	: 2003/01/28
* Description   : FTP daemon. (AVR-GCC Compiler)
*/

#include "main.h"
#include <stdio.h> 
#include <ctype.h> 
#include <string.h>
#include <limits.h>
#include <stdarg.h>
#include "stdio_private.h"
#include "socket.h"
#include "ftpd.h"
#include "sys_log.h"
#include <stdlib.h>
#include "server_device.h"

/* Command table */
static char *commands[] = {
	"user",
	"acct",
	"pass",
	"type",
	"list",
	"cwd",
	"dele",
	"name",
	"quit",
	"retr",
	"stor",
	"port",
	"nlst",
	"pwd",
	"xpwd",
	"mkd",
	"xmkd",
	"xrmd",
	"rmd ",
	"stru",
	"mode",
	"syst",
	"xmd5",
	"xcwd",
	"feat",
	"pasv",
	"size",
	"mlsd",
	"appe",
	NULL
};

extern char *login_list[];
extern char *password_list[];

#if 0
/* Response messages */
char banner[] = "220 %s FTP version %s ready.\r\n";
char badcmd[] = "500 Unknown command '%s'\r\n";
char binwarn[] = "100 Warning: type is ASCII and %s appears to be binary\r\n";
char unsupp[] = "500 Unsupported command or option\r\n";
char givepass[] = "331 Enter PASS command\r\n";
char logged[] = "230 Logged in\r\n";
char typeok[] = "200 Type %s OK\r\n";
char only8[] = "501 Only logical bytesize 8 supported\r\n";
char deleok[] = "250 File deleted\r\n";
char mkdok[] = "200 MKD ok\r\n";
char delefail[] = "550 Delete failed: %s\r\n";
char pwdmsg[] = "257 \"%s\" is current directory\r\n";
char badtype[] = "501 Unknown type \"%s\"\r\n";
char badport[] = "501 Bad port syntax\r\n";
char unimp[] = "502 Command does not implemented yet.\r\n";
char bye[] = "221 Goodbye!\r\n";
char nodir[] = "553 Can't read directory \"%s\": %s\r\n";
char cantopen[] = "550 Can't read file \"%s\": %s\r\n";
char sending[] = "150 Opening data connection for %s (%d.%d.%d.%d,%d)\r\n";
char cantmake[] = "553 Can't create \"%s\": %s\r\n";
char writerr[] = "552 Write error: %s\r\n";
char portok[] = "200 PORT command successful.\r\n";
char rxok[] = "226 Transfer complete.\r\n";
char txok[] = "226 Transfer complete.\r\n";
char noperm[] = "550 Permission denied\r\n";
char noconn[] = "425 Data connection reset\r\n";
char lowmem[] = "421 System overloaded, try again later\r\n";
char notlog[] = "530 Please log in with USER and PASS\r\n";
char userfirst[] = "503 Login with USER first.\r\n";
char okay[] = "200 Ok\r\n";
char syst[] = "215 %s Type: L%d Version: %s\r\n";
char sizefail[] = "550 File not found\r\n";
#endif

extern ftp_data_t ftp_data;

un_l2cval remote_ip;
uint16_t  remote_port;
un_l2cval local_ip;
uint16_t  local_port;
uint8_t connect_state_control = 0;
uint8_t connect_state_data = 0;

struct ftpd ftp;

int current_year = 2014;
int current_month = 12;
int current_day = 31;
int current_hour = 10;
int current_min = 10;
int current_sec = 30;

int fsprintf(uint8_t s, const char *format, ...)
{
	int i = 0;
/*
	char buf[LINELEN];
	FILE f;
	va_list ap;

	f.flags = __SWR | __SSTR;
	f.buf = buf;
	f.size = INT_MAX;
	va_start(ap, format);
	i = vfprintf(&f, format, ap);
	va_end(ap);
	buf[f.len] = 0;

	send(s, (uint8_t *)buf, strlen(buf));
*/
	return i;
}

void ftpd_init(uint8_t * src_ip)
{
	ftp.state = FTPS_NOT_LOGIN;
	ftp.current_cmd = NO_CMD;
	ftp.dsock_mode = ACTIVE_MODE;

	local_ip.cVal[0] = src_ip[0];
	local_ip.cVal[1] = src_ip[1];
	local_ip.cVal[2] = src_ip[2];
	local_ip.cVal[3] = src_ip[3];
	local_port = 35000;
	
	strcpy(ftp.workingdir, "3:");		// "/"

	socket(CTRL_SOCK, Sn_MR_TCP, IPPORT_FTP, 0x0);
}

int get_filesize(char* path, char *filename)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	int i, len, buf_ptr = 0;
	(void)i;
	(void)buf_ptr;
	(void)len;
	char *fn; 	/* This function is assuming no_Unicode cfg.*/
	char fullpath[100];
//#ifdef _USE_LFN
#if 0
	static char lfn[_MAX_LFN + 1];
	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
#endif

	if(*path == 0x00)
		res = f_opendir(&dir, "3:");	// "/"
	else if (*path == 0xFF)
	{
		res = f_opendir(&dir, ftp.workingdir);		// +
	}
	else
	{
		sprintf(fullpath, "%s%s", ftp.workingdir, path);	// +
		res = f_opendir(&dir, fullpath);	// path -> fullpath
	}
#if defined(_FF_F_DEBUG_)
    printf("f_opendir res: %d\r\n", res);
#endif
	if(res == FR_OK){
		for(;;){
			res = f_readdir(&dir, &fno);
			if(res != FR_OK || fno.fname[0] == 0) break;
			if(fno.fname[0] == '.') continue;
            #if 0
#ifdef _USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			fn = fno.fname;
#endif
#else
            fn = fno.fname;
#endif
			if(!strcmp(fn, filename))
			{
				if(fno.fattrib & AM_DIR){
#if defined(_FF_F_DEBUG_)
                    printf("\r\n%s/%s is a directory\r\n", path, filename);
#endif
					return 0;
				}
				return fno.fsize;
			}
		}
#if defined(_FF_F_DEBUG_)
        printf("\r\n%s/%s was not found\r\n", path, filename);
#endif
        f_closedir(&dir);

	}
	return -1;
}

/*
 * Inserted from earlier realize FAT FS ->
 * FAT file system module  R0.10c                (C)ChaN, 2014
 */
/*
 * Preferable structure of out <LIST> see here:
 * https://files.stairways.com/other/ftp-list-specs-info.txt
 *
 * something like:
	-rw-------  1 peter         848 Dec 14 11:22 00README.txt
or
	- whatever you feel like 848 Dec 14 11:22 00README.txt

 * also
 * http://cr.yp.to/ftp/list/binls.html
 *
/bin/ls format is a series of lines terminated by \015\012. Servers using /bin/ls format need to be prepared to generate lines in the following form:

     -rw-r--r-- 1 owner group           213 Aug 26 16:31 README

The line contains

    - for a regular file or d for a directory;
    the literal string rw-r--r-- 1 owner group for a regular file, or rwxr-xr-x 1 owner group for a directory;
    the file size in decimal right-justified in a 13-byte field;
    a three-letter month name, first letter capitalized;
    a day number right-justified in a 3-byte field;
    a space and a 2-digit hour number;
    a colon and a 2-digit minute number;
    a space and the abbreviated pathname of the file.
 */
#if 1
FRESULT scan_files(char* path, char *buf, int * buf_len)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	int i, len = 0, buf_ptr = 0;
	(void)i;
	char *fn; 	/* This function is assuming no_Unicode cfg.*/
	char date_str[15];
	int date_str_ptr = 0;
    DWORD now_sect = 0;
//#ifdef FF_USE_LFN
#if 0
	static char lfn[_MAX_LFN + 1];
	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
#endif

	res = f_opendir(&dir, path);
#if defined(_FF_F_DEBUG_)
    printf("%s->%s [%d]\r\n",  __FILE__, __FUNCTION__, __LINE__);
	printf("f_opendir res: %d path:%s\r\n", res, path);
#endif
	if(res == FR_OK){
		i = strlen(path);
#if defined(_FF_F_DEBUG_)
        printf("strlen of path: %s %d \r\n", path, i);
#endif
		for(;;){
			res = f_readdir(&dir, &fno);
            log_i(FAT_TAG, "\tf_readdir ret : %d, fname %c", res, fno.fname[0]);
			if(res != FR_OK || fno.fname[0] == 0) break;
			if(fno.fname[0] == '.') continue;
            #if 0
#ifdef FF_USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			fn = fno.fname;
#endif
#else
            fn = fno.fname;
#endif
#if defined(_FF_F_DEBUG_)
            printf("1 sect : %ld\r\n", dir.sect);
#endif
            now_sect = dir.sect;
			switch((fno.fdate >> 5) & 0x0f)
			{
			case 1:
				len = sprintf(date_str, "JAN ");
				break;
			case 2:
				len = sprintf(date_str, "FEB ");
				break;
			case 3:
				len = sprintf(date_str, "MAR ");
				break;
			case 4:
				len = sprintf(date_str, "APR ");
				break;
			case 5:
				len = sprintf(date_str, "MAY ");
				break;
			case 6:
				len = sprintf(date_str, "JUN ");
				break;
			case 7:
				len = sprintf(date_str, "JUL ");
				break;
			case 8:
				len = sprintf(date_str, "AUG ");
				break;
			case 9:
				len = sprintf(date_str, "SEP ");
				break;
			case 10:
				len = sprintf(date_str, "OCT ");
				break;
			case 11:
				len = sprintf(date_str, "NOV ");
				break;
			case 12:
				len = sprintf(date_str, "DEC ");
				break;
			}
			date_str_ptr += len;
			len = sprintf(date_str + date_str_ptr, "%d ", (fno.fdate & 0x1f));
			date_str_ptr += len;
			len = sprintf(date_str + date_str_ptr, "%d", (((fno.fdate >> 9) & 0x7f) + 1980));
			date_str_ptr = 0;
#if defined(_FF_F_DEBUG_)
            printf("date str : %s \r\n date_str_ptr:%d\r\n", date_str, date_str_ptr);
#endif
#if defined(_FF_F_DEBUG_)
                        printf("2 sect : %ld\r\n", dir.sect);
#endif

			if(fno.fattrib & AM_DIR)
			{
				sprintf(buf + buf_ptr, "d");
			}else
			{
				sprintf(buf + buf_ptr, "-");
			}
			buf_ptr++;
			// drwxr-xr-x 1 ftp ftp              0 Apr 07  2014 $RECYCLE.BIN\r\n
			//len = sprintf(buf + buf_ptr, "rwxr-xr-x 1 ftp ftp              %d %s %s\r\n", fno.fsize, date_str, fn);
			len = sprintf(buf + buf_ptr, "rwxr-xr-x 1 ftp ftp %ld %s %s\r\n", fno.fsize, date_str, fn);
			buf_ptr += len;
#if defined(_FF_F_DEBUG_)
            printf("fn: %s, buff_ptr:%d \r\n", fn, buf_ptr);
#endif
            if(dir.sect != now_sect)
            {
#if defined(_FF_F_DEBUG_)
                printf("now sect:%d, dir.sect: %d\r\n", now_sect, dir.sect);
#endif
                break;
            }
		}
#if defined(_FF_F_DEBUG_)
        *buf_len = strlen(buf);
        printf("[%s]\r\n", buf);
        printf("buf_len : %d, sizeof(buf): %d\r\n", buf_len, sizeof(buf));
#endif
		f_closedir(&dir);
	}
	return res;
}
#endif

uint8_t ftpd_run(uint8_t * dbuf)
{
	uint16_t size = 0, i;
	long ret = 0;
	uint32_t blocklen, send_byte, recv_byte;
	(void)send_byte;
	(void)i;
	uint32_t remain_filesize;
	uint32_t remain_datasize;
#if defined(F_FILESYSTEM)
	//FILINFO fno;
#endif

	//memset(dbuf, 0, sizeof(_MAX_SS));
	
    switch(getSn_SR(CTRL_SOCK))
    {
    	case SOCK_ESTABLISHED :
    		if(!connect_state_control)
    		{
#if defined(_FTP_DEBUG_)
    			log_i(ETH_TAG, "\t%d:FTP Connected", CTRL_SOCK);
#endif
    			//fsprintf(CTRL_SOCK, banner, HOSTNAME, VERSION);
    			strcpy(ftp.workingdir, "3:");		// "/"
    			sprintf((char *)dbuf, "220 %s FTP version %s ready.\r\n", HOSTNAME, VERSION);
    			ret = send(CTRL_SOCK, (uint8_t *)dbuf, strlen((const char *)dbuf));
    			if(ret < 0)
    			{
#if defined(_FTP_DEBUG_)
    				log_i(ETH_TAG, "\t%d:send() error:%ld",CTRL_SOCK,ret);
#endif
    				close(CTRL_SOCK);
    				return ret;
    			}
    			connect_state_control = 1;
    		}
	
#if defined(_FTP_DEBUG_)
    		//printf("ftp socket %d\r\n", CTRL_SOCK);
#endif
			 
    		if((size = getSn_RX_RSR(CTRL_SOCK)) > 0) // Don't need to check SOCKERR_BUSY because it doesn't not occur.
    		{
#if defined(_FTP_DEBUG_)
    			log_i(ETH_TAG, "\tsize: %d", size);
#endif

    			memset(dbuf, 0, FF_MAX_SS);

    			if(size > FF_MAX_SS) size = FF_MAX_SS - 1;

    			ret = recv(CTRL_SOCK,dbuf,size);
    			dbuf[ret] = '\0';
    			if(ret != size)
    			{
    				if(ret==SOCK_BUSY) return 0;
    				if(ret < 0)
    				{
#if defined(_FTP_DEBUG_)
    					log_i(ETH_TAG, "\t%d:recv() error:%ld",CTRL_SOCK,ret);
#endif
    					close(CTRL_SOCK);
    					return ret;
    				}
    			}
#if defined(_FTP_DEBUG_)
    			log_i(ETH_TAG, "\tRcvd Command: %s", dbuf);
#endif
    			proc_ftpd((char *)dbuf);
    		}
    		break;

    	case SOCK_CLOSE_WAIT :
#if defined(_FTP_DEBUG_)
    		log_i(ETH_TAG, "\t%d:CloseWait",CTRL_SOCK);
#endif
    		if((ret=disconnect(CTRL_SOCK)) != SOCK_OK) return ret;
#if defined(_FTP_DEBUG_)
    		log_i(ETH_TAG, "\t%d:Closed",CTRL_SOCK);
#endif
    		break;

    	case SOCK_CLOSED :
#if defined(_FTP_DEBUG_)
    		log_i(ETH_TAG, "\t%d:FTPStart",CTRL_SOCK);
#endif
    		if((ret=socket(CTRL_SOCK, Sn_MR_TCP, IPPORT_FTP, 0x0)) != CTRL_SOCK)
    		{
#if defined(_FTP_DEBUG_)
    			log_i(ETH_TAG, "%d:socket() error:%ld\r\n", CTRL_SOCK, ret);
#endif
    			close(CTRL_SOCK);
    			return ret;
    		}
    		break;

    	case SOCK_INIT :
#if defined(_FTP_DEBUG_)
    		log_i(ETH_TAG, "\t%d:Opened",CTRL_SOCK);
#endif
    		//strcpy(ftp.workingdir, "/");
    		if( (ret = listen(CTRL_SOCK)) != SOCK_OK)
    		{
#if defined(_FTP_DEBUG_)
    			log_i(ETH_TAG, "\t%d:Listen error",CTRL_SOCK);
#endif
    			return ret;
    		}
			connect_state_control = 0;

#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "\t%d:Listen ok",CTRL_SOCK);
#endif
			break;

    	default :
    		break;
    }

#if 1
    switch(getSn_SR(DATA_SOCK))
    {
    	case SOCK_ESTABLISHED :
    		if(!connect_state_data)
    		{
#if defined(_FTP_DEBUG_)
    			log_i(ETH_TAG, "\t%d:FTP Data socket Connected", DATA_SOCK);
#endif
    			connect_state_data = 1;
    		}
	
    		switch(ftp.current_cmd)
    		{
    			case LIST_CMD:
    			case MLSD_CMD:
#if defined(_FTP_DEBUG_)
    				log_i(ETH_TAG, "\tprevious size: %d", size);
#endif
#if defined(F_FILESYSTEM)
    				scan_files(ftp.workingdir, (char*)dbuf, (int *)&size);
#endif
#if defined(_FTP_DEBUG_)
    				log_i(ETH_TAG, "\treturned size: %d", size);
    				log_i(ETH_TAG, "\t%s", dbuf);
#endif
#if !defined(F_FILESYSTEM)
    				if (strncmp(ftp.workingdir, "/$Recycle.Bin", sizeof("/$Recycle.Bin")) != 0)
    					size = sprintf((char*)dbuf, "drwxr-xr-x 1 ftp ftp 0 Dec 31 2014 $Recycle.Bin\r\n-rwxr-xr-x 1 ftp ftp 512 Dec 31 2014 test.txt\r\n");
#endif
    				size = strlen((char*)dbuf);
    				send(DATA_SOCK, dbuf, size);
    				ftp.current_cmd = NO_CMD;
    				disconnect(DATA_SOCK);
    				size = sprintf((char*)dbuf, "226 Successfully transferred \"%s\"\r\n", ftp.workingdir);
    				send(CTRL_SOCK, dbuf, size);
    				break;

    			case RETR_CMD:
#if defined(_FTP_DEBUG_)
    				log_i(ETH_TAG, "\tfilename to retrieve : %s %d", ftp.filename, strlen(ftp.filename));
#endif
#if defined(F_FILESYSTEM)
    				ftp.fr = f_open(&(ftp.fil), (const char *)ftp.filename, FA_READ);
    				//print_filedsc(&(ftp.fil));
    				if(ftp.fr == FR_OK){
    					remain_filesize = ftp.fil.obj.objsize;
#if defined(_FTP_DEBUG_)
    					log_i(ETH_TAG, "\tf_open return FR_OK");
#endif
    					do{
#if defined(_FTP_DEBUG_)
    						//printf("remained file size: %d\r\n", ftp.fil.fsize);
#endif
    						memset(dbuf, 0, FF_MAX_SS);

    						if(remain_filesize > FF_MAX_SS)
    							send_byte = FF_MAX_SS;
    						else
    							send_byte = remain_filesize;

    						ftp.fr = f_read(&(ftp.fil), dbuf, send_byte , (UINT*)&blocklen);
    						if(ftp.fr != FR_OK)
    							break;
#if defined(_FTP_DEBUG_)
    						log_i(ETH_TAG, "\t#");
    						//printf("----->fsize:%d recv:%d len:%d \r\n", remain_filesize, send_byte, blocklen);
    						//printf("----->fn:%s data:%s \r\n", ftp.filename, dbuf);
#endif
    						send(DATA_SOCK, dbuf, blocklen);
    						remain_filesize -= blocklen;
    					}while(remain_filesize != 0);
#if defined(_FTP_DEBUG_)
    					log_i(ETH_TAG, "\tFile read finished");
#endif
    					ftp.fr = f_close(&(ftp.fil));
    				}else{
#if defined(_FTP_DEBUG_)
    					log_i(ETH_TAG, "\tFile Open Error: %d", ftp.fr);
#endif
    				}
#else
					remain_filesize = strlen(ftp.filename);

					do{
						memset(dbuf, 0, _MAX_SS);

						blocklen = sprintf((char*)dbuf, "%s", ftp.filename);

						log_i(ETH_TAG, "########## dbuf:%s\r\n", dbuf);

						send(DATA_SOCK, dbuf, blocklen);
						remain_filesize -= blocklen;
					}while(remain_filesize != 0);

#endif
    				ftp.current_cmd = NO_CMD;
    				disconnect(DATA_SOCK);
    				size = sprintf((char*)dbuf, "226 Successfully transferred \"%s\"\r\n", ftp.filename);
    				send(CTRL_SOCK, dbuf, size);
    				break;

    			case STOR_CMD:
#if defined(_FTP_DEBUG_)
    				log_i(ETH_TAG, "\tfilename to store : %s %d", ftp.filename, strlen(ftp.filename));
#endif
#if defined(F_FILESYSTEM)
    				ftp.fr = f_open(&(ftp.fil), (const char *)ftp.filename, FA_CREATE_ALWAYS | FA_WRITE);
    				//print_filedsc(&(ftp.fil));
    				if(ftp.fr == FR_OK){
#if defined(_FTP_DEBUG_)
    					log_i(ETH_TAG, "\tf_open return FR_OK");
#endif
    					while(1){
    						if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){
    							while(1){
    								memset(dbuf, 0, FF_MAX_SS);

    								if(remain_datasize > FF_MAX_SS)
    									recv_byte = FF_MAX_SS;
    								else
    									recv_byte = remain_datasize;

    								ret = recv(DATA_SOCK, dbuf, recv_byte);
#if defined(_FTP_DEBUG_)
    								//printf("----->fn:%s data:%s \r\n", ftp.filename, dbuf);
#endif

    								ftp.fr = f_write(&(ftp.fil), dbuf, (UINT)ret, (UINT*)&blocklen);
#if defined(_FTP_DEBUG_)
    								//printf("----->dsize:%d recv:%d len:%d \r\n", remain_datasize, ret, blocklen);
#endif
    								remain_datasize -= blocklen;

    								if(ftp.fr != FR_OK){
#if defined(_FTP_DEBUG_)
    									log_i(ETH_TAG, "\tf_write failed");
#endif
    									break;
    								}

    								if(remain_datasize <= 0)
    									break;
    							}

    							if(ftp.fr != FR_OK){
#if defined(_FTP_DEBUG_)
    								log_i(ETH_TAG, "\tf_write failed");
#endif
    								break;
    							}

#if defined(_FTP_DEBUG_)
    							log_i(ETH_TAG, "\t#");
#endif
    						}else{
    							if(getSn_SR(DATA_SOCK) != SOCK_ESTABLISHED)
    								break;
    						}
    					}
#if defined(_FTP_DEBUG_)
    					log_i(ETH_TAG, "\tFile write finished");
#endif
    					ftp.fr = f_close(&(ftp.fil));
    				}else{
#if defined(_FTP_DEBUG_)
    					log_i(ETH_TAG, "File Open Error: %d", ftp.fr);
#endif
    				}

    				//fno.fdate = (WORD)(((current_year - 1980) << 9) | (current_month << 5) | current_day);
    				//fno.ftime = (WORD)((current_hour << 11) | (current_min << 5) | (current_sec >> 1));
    				//f_utime((const char *)ftp.filename, &fno);
#else
					while(1){
						if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){
							while(1){
								memset(dbuf, 0, _MAX_SS);

								if(remain_datasize > _MAX_SS)
									recv_byte = _MAX_SS;
								else
									recv_byte = remain_datasize;

								ret = recv(DATA_SOCK, dbuf, recv_byte);

								log_i(ETH_TAG, "########## dbuf:%s\r\n", dbuf);

								remain_datasize -= ret;

								if(remain_datasize <= 0)
									break;
							}
						}else{
							if(getSn_SR(DATA_SOCK) != SOCK_ESTABLISHED)
								break;
						}
					}
#endif
    				ftp.current_cmd = NO_CMD;
    				disconnect(DATA_SOCK);
    				size = sprintf((char*)dbuf, "226 Successfully transferred \"%s\"\r\n", ftp.filename);
    				send(CTRL_SOCK, dbuf, size);
    				break;

    			case NO_CMD:
    			default:
    				break;
    		}
    		break;

   		case SOCK_CLOSE_WAIT :
#if defined(_FTP_DEBUG_)
   			log_i(ETH_TAG, "\t%d:CloseWait",DATA_SOCK);
#endif
   			if((ret=disconnect(DATA_SOCK)) != SOCK_OK) return ret;
#if defined(_FTP_DEBUG_)
   			log_i(ETH_TAG, "\t%d:Closed",DATA_SOCK);
#endif
   			break;

   		case SOCK_CLOSED :
   			if(ftp.dsock_state == DATASOCK_READY)
   			{
   				if(ftp.dsock_mode == PASSIVE_MODE){
#if defined(_FTP_DEBUG_)
   					log_i(ETH_TAG, "\t%d:FTPDataStart, port : %d",DATA_SOCK, local_port);
#endif
   					if((ret=socket(DATA_SOCK, Sn_MR_TCP, local_port, 0x0)) != DATA_SOCK)
   					{
#if defined(_FTP_DEBUG_)
   						log_i(ETH_TAG, "\t%d:socket() error:%ld", DATA_SOCK, ret);
#endif
   						close(DATA_SOCK);
   						return ret;
   					}

   					local_port++;
   					if(local_port > 50000)
   						local_port = 35000;
   				}else{
#if defined(_FTP_DEBUG_)
   					log_i(ETH_TAG, "\t%d:FTPDataStart, port : %d",DATA_SOCK, IPPORT_FTPD);
#endif
   					if((ret=socket(DATA_SOCK, Sn_MR_TCP, IPPORT_FTPD, 0x0)) != DATA_SOCK)
   					{
#if defined(_FTP_DEBUG_)
   						log_i(ETH_TAG, "\t%d:socket() error:%ld", DATA_SOCK, ret);
#endif
   						close(DATA_SOCK);
   						return ret;
   					}
   				}

   				ftp.dsock_state = DATASOCK_START;
   			}
   			break;

   		case SOCK_INIT :
#if defined(_FTP_DEBUG_)
   			log_i(ETH_TAG, "\t%d:Opened",DATA_SOCK);
#endif
   			if(ftp.dsock_mode == PASSIVE_MODE){
   				if( (ret = listen(DATA_SOCK)) != SOCK_OK)
   				{
#if defined(_FTP_DEBUG_)
   					log_i(ETH_TAG, "\t%d:Listen error",DATA_SOCK);
#endif
   					return ret;
   				}

#if defined(_FTP_DEBUG_)
   				log_i(ETH_TAG, "\t%d:Listen ok",DATA_SOCK);
#endif
   			}else{
   				if((ret = connect(DATA_SOCK, remote_ip.cVal, remote_port)) != SOCK_OK){
#if defined(_FTP_DEBUG_)
   					log_i(ETH_TAG, "\t%d:Connect error", DATA_SOCK);
#endif
   					return ret;
   				}
   			}
   			connect_state_data = 0;
   			break;

   		default :
   			break;
    }
#endif

    return 0;
}

char proc_ftpd(char * buf)
{
	char **cmdp, *cp, *arg, *tmpstr;
	char sendbuf[200];
	int slen;
	long ret;
	

	/* Translate first word to lower case */
	for (cp = buf; *cp != ' ' && *cp != '\0'; cp++)
		*cp = tolower(*cp);

	/* Find command in table; if not present, return syntax error */
	for (cmdp = commands; *cmdp != NULL; cmdp++)
		if (strncmp(*cmdp, buf, strlen(*cmdp)) == 0)
			break;

	if (*cmdp == NULL)
	{
		//fsprintf(CTRL_SOCK, badcmd, buf);
		slen = sprintf(sendbuf, "500 Unknown command '%s'\r\n", buf);
		send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
		return 0;
	}
	/* Allow only USER, PASS and QUIT before logging in */
	if (ftp.state == FTPS_NOT_LOGIN)
	{
		switch(cmdp - commands)
		{
			case USER_CMD:
			case PASS_CMD:
			case QUIT_CMD:
				break;
			default:
				//fsprintf(CTRL_SOCK, notlog);
				slen = sprintf(sendbuf, "530 Please log in with USER and PASS\r\n");
				send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
				return 0;
		}
	}
	
	arg = &buf[strlen(*cmdp)];
	while(*arg == ' ') arg++;

	/* Execute specific command */
	switch (cmdp - commands)
	{
		case USER_CMD :
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "\tUSER_CMD : %s", arg);
#endif
			slen = strlen(arg);
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			strcpy(ftp.username, arg);
			//fsprintf(CTRL_SOCK, givepass);
			slen = sprintf(sendbuf, "331 Enter PASS command\r\n");
			ret = send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			if(ret < 0)
			{
#if defined(_FTP_DEBUG_)
				log_i(ETH_TAG, "\t%d:send() error:%ld",CTRL_SOCK,ret);
#endif
				close(CTRL_SOCK);
				return ret;
			}
			break;

		case PASS_CMD :
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "\tPASS_CMD : %s", arg);
#endif
			slen = strlen(arg);
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			ftplogin(arg);
			break;

		case TYPE_CMD :
			slen = strlen(arg);
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			switch(arg[0])
			{
				case 'A':
				case 'a':	/* Ascii */
					ftp.type = ASCII_TYPE;
					//fsprintf(CTRL_SOCK, typeok, arg);
					slen = sprintf(sendbuf, "200 Type set to %s\r\n", arg);
					send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
					break;

				case 'B':
				case 'b':	/* Binary */
				case 'I':
				case 'i':	/* Image */
					ftp.type = IMAGE_TYPE;
					//fsprintf(CTRL_SOCK, typeok, arg);
					slen = sprintf(sendbuf, "200 Type set to %s\r\n", arg);
					send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
					break;

				default:	/* Invalid */
					//fsprintf(CTRL_SOCK, badtype, arg);
					slen = sprintf(sendbuf, "501 Unknown type \"%s\"\r\n", arg);
					send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
					break;
			}
			break;

		case FEAT_CMD :
			slen = sprintf(sendbuf, "211-Features:\r\n MDTM\r\n REST STREAM\r\n SIZE\r\n MLST size*;type*;create*;modify*;\r\n MLSD\r\n UTF8\r\n CLNT\r\n MFMT\r\n211 END\r\n");
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			break;

		case QUIT_CMD :
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "QUIT_CMD\r\n");
#endif
			//fsprintf(CTRL_SOCK, bye);
			slen = sprintf(sendbuf, "221 Goodbye!\r\n");
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			disconnect(CTRL_SOCK);
			break;

		case RETR_CMD :
			slen = strlen(arg);
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "RETR_CMD\r\n");
#endif
			if (slen <= 50)
			{
				if (arg[slen - 3] == '/')
					arg[slen - 3] == 0x0;
				if (*arg == '/')
					sprintf(ftp.filename, "3:%s", (arg + 1));
				//else if(strlen(ftp.workingdir) == 1)
					//sprintf(ftp.filename, "/%s", arg);
				else
					sprintf(ftp.filename, "%s%s", ftp.workingdir, arg);
				slen = sprintf(sendbuf, "150 Opening data channel for file downloand from server of \"%s\"\r\n", ftp.filename);
				send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
				ftp.current_cmd = RETR_CMD;
			}
			else
			{
				slen = sprintf(sendbuf, "550 File not Found\r\n");
				send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
				ftp.current_cmd = NO_CMD;
			}

			break;

		case APPE_CMD :
		case STOR_CMD:
			slen = strlen(arg);
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "\tSTOR_CMD");
#endif
			if(strlen(ftp.workingdir) == 1)
				sprintf(ftp.filename, "/%s", arg);
			else
				sprintf(ftp.filename, "%s/%s", ftp.workingdir, arg);
			slen = sprintf(sendbuf, "150 Opening data channel for file upload to server of \"%s\"\r\n", ftp.filename);
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			ftp.current_cmd = STOR_CMD;
			if((ret = connect(DATA_SOCK, remote_ip.cVal, remote_port)) != SOCK_OK){
#if defined(_FTP_DEBUG_)
				log_i(ETH_TAG, "\t%d:Connect error", DATA_SOCK);
#endif
				return ret;
			}
   			connect_state_data = 0;
			break;

		case PORT_CMD:
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "\tPORT_CMD");
#endif
			if (pport(arg) == -1){
				//fsprintf(CTRL_SOCK, badport);
				slen = sprintf(sendbuf, "501 Bad port syntax\r\n");
				send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			} else{
				//fsprintf(CTRL_SOCK, portok);
				ftp.dsock_mode = ACTIVE_MODE;
				ftp.dsock_state = DATASOCK_READY;
				slen = sprintf(sendbuf, "200 PORT command successful.\r\n");
				send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			}
			break;

		case MLSD_CMD:
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "MLSD_CMD\r\n");
#endif
			slen = sprintf(sendbuf, "150 Opening data channel for directory listing of \"%s\"\r\n", ftp.workingdir);
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			ftp.current_cmd = MLSD_CMD;
			break;

		case LIST_CMD:
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "LIST_CMD\r\n");
#endif
			slen = sprintf(sendbuf, "150 Opening data channel for directory listing of \"%s\"\r\n", ftp.workingdir);
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			ftp.current_cmd = LIST_CMD;
			break;

		case NLST_CMD:
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "NLST_CMD\r\n");
#endif
			break;

		case SYST_CMD:
			slen = sprintf(sendbuf, "215 UNIX emulated by WIZnet\r\n");
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			break;

		case PWD_CMD:
		case XPWD_CMD:
			slen = sprintf(sendbuf, "257 \"%s\" is current directory.\r\n", ftp.workingdir);
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			break;

		case PASV_CMD:
			slen = sprintf(sendbuf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n", local_ip.cVal[0], local_ip.cVal[1], local_ip.cVal[2], local_ip.cVal[3], local_port >> 8, local_port & 0x00ff);
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			disconnect(DATA_SOCK);
			ftp.dsock_mode = PASSIVE_MODE;
			ftp.dsock_state = DATASOCK_READY;
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "\tPASV port: %d", local_port);
#endif
		break;

		case SIZE_CMD:
			slen = strlen(arg);
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			if(slen > 3)
			{
				tmpstr = strrchr(arg, '/');
				if (tmpstr != NULL)
				{
					*tmpstr = 0;
#if defined(F_FILESYSTEM)
					slen = get_filesize(arg, tmpstr + 1);
				}
				else
				{
					char tmpsymb = 0xFF;
					slen = get_filesize(&tmpsymb, arg);
				}
#else
				slen = _MAX_SS;
#endif
				if(slen > 0)
					slen = sprintf(sendbuf, "213 %d\r\n", slen);
				else
					slen = sprintf(sendbuf, "550 File not Found\r\n");
			}
			else
			{
				slen = sprintf(sendbuf, "550 File not Found\r\n");
			}
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			break;

		case CWD_CMD:
			slen = strlen(arg);
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
			if(slen > 3)
			{
				if (arg[slen-3] == '/')
					arg[slen - 3] = 0x00;
				tmpstr = strrchr(arg, '/');
				if (tmpstr != NULL)
				{
					*tmpstr = 0;
#if defined(F_FILESYSTEM)
					slen = get_filesize(arg, tmpstr + 1);
				}
				else
				{
					char tmpsymb = 0xFF;
					slen = get_filesize(&tmpsymb, arg);
				}
#else
				slen = 0;
#endif
				*tmpstr = '/';
				if(slen == 0){
					slen = sprintf(sendbuf, "213 %d\r\n", slen);
					if (*arg == '/')
						sprintf(ftp.workingdir, "%s%s", "3:", (arg + 1));
					else
						strcat(ftp.workingdir, arg);
					//strcpy(ftp.workingdir, arg);	//	-
					slen = sprintf(sendbuf, "250 CWD successful. \"%s\" is current directory.\r\n", ftp.workingdir);
				}
				else
				{
					slen = sprintf(sendbuf, "550 CWD failed. \"%s\"\r\n", arg);
				}
			}
			else
			{
				strcpy(ftp.workingdir, arg);
				slen = sprintf(sendbuf, "250 CWD successful. \"%s\" is current directory.\r\n", ftp.workingdir);
			}
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			break;

		case MKD_CMD:
		case XMKD_CMD:
			slen = strlen(arg);
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
#if defined(F_FILESYSTEM)
			if (slen <= 50)
			{
				if (slen > 3)
				{
					if (arg[slen - 3] == '/')
						arg[slen -3] = 0x00;
				}
				tmpstr = strrchr(arg, '/');
				if (tmpstr != NULL)
				{
					*tmpstr = 0;
					if (*arg == 0)
						sprintf(sendbuf, "%s%s", "3:", (arg + 1));
					else
					{
						*tmpstr = '/';
						sprintf(sendbuf, "%s%s", ftp.workingdir, arg);
					}
				}
				else
					sprintf(sendbuf, "%s%s", ftp.workingdir, arg);
				if (f_mkdir(arg) != 0)
				{
					slen = sprintf(sendbuf, "550 Can't create directory. \"%s\"\r\n", arg);
				}
				else
				{
					slen = sprintf(sendbuf, "257 MKD command successful. \"%s\"\r\n", arg);
					//strcpy(ftp.workingdir, arg);
				}
			}
			else
				slen = sprintf(sendbuf, "550 Can't create directory. Too long length\r\n");
#else
			slen = sprintf(sendbuf, "550 Can't create directory. Permission denied\r\n");
#endif
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			break;

		case DELE_CMD:
			slen = strlen(arg);
			arg[slen - 1] = 0x00;
			arg[slen - 2] = 0x00;
#if defined(F_FILESYSTEM)
			if (slen <= 50)
			{
				if (slen > 3)
				{
					if (arg[slen - 3] == '/')
						arg[slen -3] = 0x00;
				}
				tmpstr = strrchr(arg, '/');
				if (tmpstr != NULL)
				{
					*tmpstr = 0;
					if (*arg == 0)
						sprintf(sendbuf, "%s%s", "3:", (arg + 1));
					else
					{
						*tmpstr = '/';
						sprintf(sendbuf, "%s%s", ftp.workingdir, arg);
					}
				}
				else
					sprintf(sendbuf, "%s%s", ftp.workingdir, arg);
				if (f_unlink(sendbuf) != 0)
				{
					slen = sprintf(sendbuf, "550 Could not delete. \"%s\"\r\n", arg);
				}
				else
				{
					slen = sprintf(sendbuf, "250 Deleted. \"%s\"\r\n", arg);
				}
			}
			else 
				slen = sprintf(sendbuf, "550 Can't delete directory or file. Too long length\r\n");
#else
			slen = sprintf(sendbuf, "550 Could not delete. Permission denied\r\n");
#endif
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			break;

		case XCWD_CMD:
		case ACCT_CMD:
		case XRMD_CMD:
		case RMD_CMD:
		case STRU_CMD:
		case MODE_CMD:
		case XMD5_CMD:
			//fsprintf(CTRL_SOCK, unimp);
			slen = sprintf(sendbuf, "502 Command does not implemented yet.\r\n");
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			break;

		default:	/* Invalid */
			//fsprintf(CTRL_SOCK, badcmd, arg);
			slen = sprintf(sendbuf, "500 Unknown command \'%s\'\r\n", arg);
			send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
			break;
	}
	
	return 1;
}


char ftplogin(char * pass)
{
	char sendbuf[100];
	int slen = 0;
	char **loginp;
	char **paswordp = password_list;
	
	//memset(sendbuf, 0, DATA_BUF_SIZE);
	for (loginp = login_list; *loginp != NULL; loginp++)
	{
		if (strncmp(*loginp, ftp.username, strlen(ftp.username)) == 0)
		{
			if (strncmp(*paswordp, pass, strlen(pass)) == 0)
			{
				log_i(ETH_TAG, "%s logged in\r\n", ftp.username);
				slen = sprintf(sendbuf, "230 Logged on\r\n");
				send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
				ftp.state = FTPS_LOGIN;
				break;
			}
			else
			{
				slen = sprintf(sendbuf, "530 Login or Password incorrect.\r\n");
				send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
				return 0;
			}
		}
		paswordp++;
	}
	if (*loginp == NULL)
	{
		slen = sprintf(sendbuf, "530 Login or Password incorrect.\r\n");
		send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
		return 0;
	}
	return 1;

	// if(!(memcmp((char*)&ftp.username, (char*)&ftp_data.login, strlen((char*)&ftp_data.login))) && !(memcmp(pass, (char*)&ftp_data.password, strlen((char*)&ftp_data.password))))
	// {
	// 	#if defined(_FTP_DEBUG_)
	// 	printf("%s logged in\r\n", ftp.username);
	// 	#endif						
	// 	slen = sprintf(sendbuf, "230 Logged in\r\n");
	// 	send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
	// 	ftp.state = FTPS_LOGIN;
	// }
	// else
	// {
	// 	slen = sprintf(sendbuf, "530 Login or Password incorrect.\r\n");
	// 	send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
	// 	return 0;	
	// }
	// return 1;
	
// #if defined(_FTP_DEBUG_)
// 	log_i(ETH_TAG, "%s logged in\r\n", ftp.username);
// #endif
// 	//fsprintf(CTRL_SOCK, logged);
// 	slen = sprintf(sendbuf, "230 Logged on\r\n");
// 	send(CTRL_SOCK, (uint8_t *)sendbuf, slen);
// 	ftp.state = FTPS_LOGIN;
	
// 	return 1;
}

int pport(char * arg)
{
	int i;
	char* tok=0;

	for (i = 0; i < 4; i++)
	{
		if(i==0) tok = strtok(arg,",\r\n");
		else	 tok = strtok(NULL,",");
		remote_ip.cVal[i] = (uint8_t)atoi(tok);
		if (!tok)
		{
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "bad pport : %s\r\n", arg);
#endif
			return -1;
		}
	}
	remote_port = 0;
	for (i = 0; i < 2; i++)
	{
		tok = strtok(NULL,",\r\n");
		remote_port <<= 8;
		remote_port += atoi(tok);
		if (!tok)
		{
#if defined(_FTP_DEBUG_)
			log_i(ETH_TAG, "bad pport : %s\r\n", arg);
#endif
			return -1;
		}
	}
#if defined(_FTP_DEBUG_)
	log_i(ETH_TAG, "ip : %d.%d.%d.%d, port : %d\r\n", remote_ip.cVal[0], remote_ip.cVal[1], remote_ip.cVal[2], remote_ip.cVal[3], remote_port);
#endif

	return 0;
}

#if defined(F_FILESYSTEM)
void print_filedsc(FIL *fil)
{
#if defined(_FTP_DEBUG_)
	// printf("File System pointer : %08X\r\n", fil->fs);
	// printf("File System mount ID : %d\r\n", fil->id);
	// printf("File status flag : %08X\r\n", fil->flag);
	// printf("File System pads : %08X\r\n", fil->err);
	// printf("File read write pointer : %08X\r\n", fil->fptr);
	// printf("File size : %08X\r\n", fil->fsize);
	// printf("File start cluster : %08X\r\n", fil->sclust);
	// printf("current cluster : %08X\r\n", fil->clust);
	// printf("current data sector : %08X\r\n", fil->dsect);
	// printf("dir entry sector : %08X\r\n", fil->dir_sect);
	// printf("dir entry pointer : %08X\r\n", fil->dir_ptr);
#endif
}
#endif
