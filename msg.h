/*
 * Error Infomation
 */
#define THREAD_ERROR "Sorry, an error occurred. Connect us after a minute."




/*
 * Reply Infomation
 */
#define CONFIRM "confirm"





/*
 * Command structure
 */

/* request */
#define CMD_GET 		1
#define CMD_LS 			2
#define CMD_CD 			3
#define CMD_MKDIR		4
#define CMD_RMDIR		5
#define CMD_PUT			6
#define	CMD_PWD			7
#define CMD_CLOSE		8

/* options */
#define bool unsigned char

struct lsentry {
	char name[32];
	int mode;
	long size;
};

typedef struct command_message {
	unsigned char type;		/* what kind of request */
	char buf[520];
} ftp_rqt;

typedef struct file_message {
	char path[128];
	char name[32];
} fs_rqt;

typedef struct file_retval {
	bool res;
	unsigned char error_no;
} fs_rpl;

typedef struct ls_retval {
	char error_no;
	bool end;
	short cnt;
	struct lsentry item[32];
} ls_rpl;

struct get_request {
	char name[128];
	long offset;
};

struct put_request {
	char path[128];
	char name[32];
	int mode;
};
struct put_reply{
	char flags;
};
struct put_content {
	unsigned char flags;
	char content[512];
};

struct get_reply {
	unsigned char flags;
	char content[512];
};

struct auth {
	char user[32];
	char pass[32];
	unsigned char first;
};

#define GET_ERR 	1
#define GET_END		2


#define RQT_LEN sizeof(struct command_message)
#define FSRQT_LEN sizeof(struct file_message)

#define FSRPL_LEN sizeof(struct file_retval)
#define LSRPL_LEN sizeof(struct ls_retval)

#define FTPHDR_LEN sizeof(char)

#define GETRQT_LEN sizeof(struct get_request)
#define GETRPL_LEN sizeof(struct get_reply)

#define PUTRQT_LEN sizeof(struct put_request)
#define PUTCON_LEN sizeof(struct put_content)
#define PUTRPL_LEN sizeof(struct put_reply)
