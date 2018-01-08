
#define RPMB_IOCTL_SOTER_WRITE_DATA	5
#define RPMB_IOCTL_SOTER_READ_DATA	6
#define RPMB_IOCTL_SOTER_GET_CNT	7

#define RPMB_BUFF_SIZE			512
#define PAGE_SIZE_4K			(0x1000)

enum {
	TZ_VFS_OPEN = 0x11,
	TZ_VFS_READ = 0x12,
	TZ_VFS_WRITE = 0x13,
	TZ_VFS_IOCTL = 0x14,
	TZ_VFS_CLOSE = 0x15,
	TZ_VFS_TRUNC = 0x16,
	TZ_VFS_UNLINK = 0x17,
	TZ_VFS_LSEEK = 0x18,
	TZ_VFS_RENAME = 0x19,
	TZ_VFS_MKDIR = 0x21,
	TZ_VFS_RMDIR = 0x22,
};


struct TEEI_vfs_command {
	int func;
	int cmd_size;

	union func_arg {
		struct func_open {
			int flags;
			int mode;
		} func_open_args;

		struct func_send {
			int fd;
			int count;
		} func_read_args;

		struct func_recv {
			int fd;
			int count;
		} func_write_args;

		struct func_ioctl {
			int fd;
			int cmd;
			int arg;
		} func_ioctl_args;

		struct func_close {
			int fd;
		} func_close_args;

		struct func_trunc {
			int fd;
			int length;
		} func_trunc_args;

		struct func_lseek {
			int fd;
			int offset;
			int origin;
		} func_lseek_args;

		struct func_mkdir {
			int mode;
		} func_mkdir_args;

	} args;

};

union TEEI_vfs_response {
	int value;
};

extern char *daulOS_VFS_share_mem;
