#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cutils/properties.h>
#include <pthread.h>
#include <android/log.h>
#include <sys/mman.h>
#include "md5_utils.h"
#include "utos_version.h"
//#include <cutils/klog.h>
#include "klog.h"
#define LOAD_TZ_DRIVER 0

#if LOAD_TZ_DRIVER
#include <linux/kernel.h>
#include "module.h"
#endif
#include "TEEI.h"

#define BUFFER_SIZE	(512 * 1024)
#define DEV_FILE	"/dev/tz_vfs"

#define TEEI_CONFIG_IOC_MAGIC 0x5B777E

#define TEEI_CONFIG_IOCTL_INIT_TEEI \
    _IOWR(TEEI_CONFIG_IOC_MAGIC, 3, int)
extern int init_module(void *, unsigned long, const char *);
static int do_mkdir(int nargs, char **args);
static void do_ota_cert_update();
static int do_link(char *fn, char *ln);

#define TAG "[mTEE]"
#define LOG_DEFAULT_LEVEL  3  /* messages <= this level are logged */
#  define  LOGD(fmt,...)     \
    do {KLOG_DEBUG(TAG,fmt"\n",##__VA_ARGS__);__android_log_print( ANDROID_LOG_DEBUG, TAG, fmt, ##__VA_ARGS__ );} while (0)

#  define  LOGI(fmt,...)     \
    do {KLOG_INFO(TAG,fmt"\n",##__VA_ARGS__);__android_log_print( ANDROID_LOG_INFO, TAG, fmt, ##__VA_ARGS__ );} while (0)

#  define  LOGW(fmt,...)     \
    do {KLOG_WARNING(TAG,fmt"\n",##__VA_ARGS__);__android_log_print( ANDROID_LOG_WARN, TAG, fmt, ##__VA_ARGS__ );} while (0)

#  define  LOGE(fmt,...)     \
    do {KLOG_ERROR(TAG,fmt"\n",##__VA_ARGS__);__android_log_print( ANDROID_LOG_ERROR, TAG, fmt, ##__VA_ARGS__ );} while (0)

#if 0
#  define  LOGD(...)     \
    do {__android_log_print( ANDROID_LOG_DEBUG, TAG, __VA_ARGS__ ); } while (0)

#  define  LOGI(...)     \
    do {__android_log_print( ANDROID_LOG_INFO, TAG, __VA_ARGS__ ); } while (0)

#  define  LOGW(...)     \
    do {__android_log_print( ANDROID_LOG_WARN, TAG, __VA_ARGS__ ); } while (0)

#  define  LOGE(...)     \
    do {__android_log_print( ANDROID_LOG_ERROR, TAG, __VA_ARGS__ ); } while (0)
#define COPY_FILE(name,to) {char * name []={"","/vendor/thh/"#name,to}; \
                                LOGI("copy %s\n",#name); \
                                int name##_ret=do_copy(3,name);if(name##_ret < 0) \
                                    LOGE("%s copy failed[%d] to %s\n",#name,name##_ret,to);}
#endif
#define COPY_FILE_FULLPATH(name,from,to) {char * name []={"",from,to}; \
                                LOGI("copy %s\n",from); \
                                int name##_ret=do_copy(3,name);if(name##_ret < 0) \
                                    LOGE("%s copy failed[%d] to %s\n",from,name##_ret,to);}

static int do_copy(int nargs, char **args);
#define OTA_REBOOT 1
#define OTA_NO 0
static int get_ota_status()
{
    //char *log_path = "/data/thh/vendor/ota.log";
    //int logfd = open(log_path,O_CREAT|O_WRONLY,0660);
    //if (logfd < 0)
    //{
    //    LOGI("file no exists : %s",log_path);
    //    return OTA_NO;
    //}

    int ret = OTA_NO;
    char * path = "/cache/ota/updateResult";
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        //write(logfd,"OTA OPEN FAILED\n",sizeof("OTA OPEN FAILED\n"));
        LOGI("file no exists : %s", path);
        //close(logfd);
        return OTA_NO;
    }


    //write(logfd,"OTA OPEN OK\n",sizeof("OTA OPEN OK\n"));

    char ch = -1;
    int rd_count = 0;

    rd_count = read(fd, &ch, 1);
    close(fd);

    if (rd_count != 1)
	return OTA_NO;

    LOGI("status : %d", ch);
    if ('0' == ch) {
        LOGI("OTA has been updated OK");
        ret = OTA_REBOOT;
    } else {
        LOGI("OTA has been updated FAILED, %d", errno);
        ret = OTA_NO;
    }

    return ret;
    //close(logfd);
}
#if 0
static int needUpdate(char * file)
{
    (void)file;
    return 1;
}
static void do_ota_update()
{
    if (needUpdate("soter"))
    {
        COPY_FILE_FULLPATH(soter_raw,"/vendor/thh/soter.raw","/data/thh/vendor/soter.raw");
    }
    if (needUpdate("alipay"))
    {
        COPY_FILE(alipayapp,"/data/thh/tee_01/tee");
    }

}
#endif
static void do_ota_cert_update()
{
    // /vendor/thh/cert_update to /data/local/thh_update
    char * upt_dir[] = {"mkdir","/data/local/thh_update","0775"};
    char * upt_dir1[] = {"mkdir","/data/local/thh_update/tee1","0775"};
    char * upt_dir2[] = {"mkdir","/data/local/thh_update/tee2","0775"};
    char * upt_dir3[] = {"mkdir","/data/local/thh_update/tee3","0775"};
    char * upt_dir4[] = {"mkdir","/data/local/thh_update/tee4","0775"};
    do_mkdir(3,upt_dir);
    do_mkdir(3,upt_dir1);
    do_mkdir(3,upt_dir2);
    do_mkdir(3,upt_dir3);
    do_mkdir(3,upt_dir4);

    char* thhCert[] = {"thh.cert","/vendor/thh/cert_update/thh.cert","/data/local/thh_update/thh.cert"};
    do_copy(3, thhCert);

    char* tee1RootCert[] = {"root.cert","/vendor/thh/cert_update/tee1/root.cert","/data/local/thh_update/tee1/root.cert"};
    char* tee2RootCert[] = {"root.cert","/vendor/thh/cert_update/tee2/root.cert","/data/local/thh_update/tee2/root.cert"};
    char* tee3RootCert[] = {"root.cert","/vendor/thh/cert_update/tee3/root.cert","/data/local/thh_update/tee3/root.cert"};
    char* tee4RootCert[] = {"root.cert","/vendor/thh/cert_update/tee4/root.cert","/data/local/thh_update/tee4/root.cert"};
    do_copy(3, tee1RootCert);
    do_copy(3, tee2RootCert);
    do_copy(3, tee3RootCert);
    do_copy(3, tee4RootCert);

    char* tee1SignCert[] = {"sign.cert","/vendor/thh/cert_update/tee1/sign.cert","/data/local/thh_update/tee1/sign.cert"};
    char* tee2SignCert[] = {"sign.cert","/vendor/thh/cert_update/tee2/sign.cert","/data/local/thh_update/tee2/sign.cert"};
    char* tee3SignCert[] = {"sign.cert","/vendor/thh/cert_update/tee3/sign.cert","/data/local/thh_update/tee3/sign.cert"};
    char* tee4SignCert[] = {"sign.cert","/vendor/thh/cert_update/tee4/sign.cert","/data/local/thh_update/tee4/sign.cert"};
    do_copy(3, tee1SignCert);
    do_copy(3, tee2SignCert);
    do_copy(3, tee3SignCert);
    do_copy(3, tee4SignCert);

    //do_link("/vendor/thh/cert_update/thh.cert","/data/local/thh_update/thh.cert");
    //do_link("/vendor/thh/cert_update/tee1/root.cert","/data/local/thh_update/tee1/root.cert");
    //do_link("/vendor/thh/cert_update/tee1/sign.cert","/data/local/thh_update/tee1/sign.cert");
    //do_link("/vendor/thh/cert_update/tee2/root.cert","/data/local/thh_update/tee2/root.cert");
    //do_link("/vendor/thh/cert_update/tee2/sign.cert","/data/local/thh_update/tee2/sign.cert");
    //do_link("/vendor/thh/cert_update/tee3/root.cert","/data/local/thh_update/tee3/root.cert");
    //do_link("/vendor/thh/cert_update/tee3/sign.cert","/data/local/thh_update/tee3/sign.cert");
    //do_link("/vendor/thh/cert_update/tee4/root.cert","/data/local/thh_update/tee4/root.cert");
    //do_link("/vendor/thh/cert_update/tee4/sign.cert","/data/local/thh_update/tee4/sign.cert");


}


static int get_image_md5(char *filepath, unsigned char *ret)
{
    unsigned char dest[16] = {0};
    struct stat status;
    char *data = NULL;
    int fd = 0;
    int retVal = 0;

    fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        LOGI("file can not be open %s ", filepath);
        return 0;
    }


    retVal = fstat(fd, &status);
    if (retVal != 0) {
        close(fd);
        return 0;
    }

    data = mmap(0, status.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (NULL == data) {
        LOGI("file can not be mmap %s", filepath);
        close(fd);
        return 0;
    }

    MD5_CTX context ;
    MD5Init(&context);
    MD5Update(&context, (md5byte *)data, status.st_size);
    MD5Final(dest, &context);
    munmap(data, status.st_size);
    close(fd);

    if (dest[0] == 0 && dest[1] == 0 && dest[2] == 0 && dest[3] == 0 && dest[4] == 0 && dest[5] == 0 && dest[6] == 0 && dest[7] == 0 &&
        dest[8] == 0 && dest[9] == 0 && dest[10] == 0 && dest[11] == 0 && dest[12] == 0 && dest[13] == 0 && dest[14] == 0 && dest[15] == 0)
    {
        LOGE("file %s md5 == 0", filepath);
        return 0;
    }

    sprintf((void *)ret, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            dest[0],dest[1],dest[2],dest[3],dest[4],dest[5],dest[6],dest[7],dest[8],dest[9],dest[10],dest[11],dest[12],dest[13],dest[14],dest[15]);

    return 1;
}

static int is_file_exist(char * path)
{
    if (access(path,0) == 0)
    {
        return 1;
    }else{
        return 0;
    }
}


static int mkdir_err_exist = 0;
int make_dir(const char *path, mode_t mode)
{
    int rc;
    rc = mkdir(path, mode);
    LOGD("creat:%s %d",path ,rc);
    return rc;
}

static int _chmod(const char *path, mode_t mode)
{
    int ret;

    struct stat p_statbuf;

    ret = lstat(path, &p_statbuf);
    if( ret < 0) {
        return -1;
    }

    if (S_ISLNK(p_statbuf.st_mode) == 1) {
        errno = EINVAL;
        return -1;
    }

    ret = chmod(path, mode);

    return ret;
}
static int do_mkdir(int nargs, char **args)
{
    mode_t mode = 0755;
    int ret;

    /* mkdir <path> [mode] [owner] [group] */

    if (nargs >= 3) {
        mode = strtoul(args[2], 0, 8);
    }

    LOGD("creat:%s",args[1]);
    ret = make_dir(args[1], mode);
    /* chmod in case the directory already exists */
    if (ret == -1 && errno == EEXIST) {
        ret = _chmod(args[1], mode);
        LOGW("creat:%s faild ",args[1]);
        mkdir_err_exist = 1;
    }else{
        mkdir_err_exist = 0;
    }
    if (ret == -1) {
        return -errno;
    }
    return 0;
}

static int do_link(char *fn, char *to)
{
  int file_descriptor;
  int ret = 0;
  if ((file_descriptor = open(fn, O_RDONLY)) < 0)
    LOGW("file no exists : %s, %d", fn, file_descriptor);
  else {
    close(file_descriptor);
    if ((ret = symlink(fn, to)) != 0) {
      LOGE("link error %s, %d", fn, errno);
      //unlink(fn);
      ret = -1;
    }
  }
  return ret;
}

static int do_copy(int nargs, char **args)
{
    char *buffer = NULL;
    int rc = 0;
    int fd1 = -1, fd2 = -1;
    struct stat info;
    int brtw, brtr;
    char *p;

    if (nargs != 3)
    {
        LOGD("do_copy: nargs = %d, need to be 3!\n",nargs);
        return -1;
    }

    if (stat(args[1], &info) < 0)
    {

        LOGD("do_copy: stat(args[1], &info) <0!\n");
        return -1;
    }

    if ((fd1 = open(args[1], O_RDONLY)) < 0)
    {

        LOGD("do_copy: open %s failed!\n",args[1]);
        goto out_err;
    }
    if ((fd2 = open(args[2], O_WRONLY|O_CREAT|O_TRUNC, 0660)) < 0)
    {
        LOGD("do_copy: open %s failed! fd=%d errno=%d from=%s\n",args[2],fd2,errno,args[1]);
        goto out_err;
    }
    if (!(buffer = malloc(info.st_size)))
    {

        LOGD("do_copy: malloc failed\n");
        goto out_err;
    }
    p = buffer;
    brtr = info.st_size;
    while(brtr) {
        rc = read(fd1, p, brtr);
        if (rc < 0)
            goto out_err;
        if (rc == 0)
            break;
        p += rc;
        brtr -= rc;
    }

    p = buffer;
    brtw = info.st_size;
    while(brtw) {
        rc = write(fd2, p, brtw);
        if (rc < 0)
            goto out_err;
        if (rc == 0)
            break;
        p += rc;
        brtw -= rc;
    }

    rc = 0;
    goto out;
out_err:
    rc = -1;
out:
    if (buffer)
        free(buffer);
    if (fd1 >= 0)
        close(fd1);
    if (fd2 >= 0)
        close(fd2);
    return rc;
}
#if 0
static void copy_files()
{
    COPY_FILE_FULLPATH(soter_raw,"/vendor/thh/soter.raw","/data/thh/vendor/soter.raw");
    COPY_FILE(alipayapp,"/data/thh/tee_01/tee");
}
#endif
char * dir5[]={"mkdir","/data/thh","0770"};
char * dirs[][3]={{"mkdir","/data/thh/system","0770"},
/*char * dir1[]=*/{"mkdir","/data/thh/tee_01","0770"},
/*char * dir2[]=*/{"mkdir","/data/thh/tee_02","0770"},
/*char * dir3[]=*/{"mkdir","/data/thh/tee_03","0770"},
/*char * dir4[]=*/{"mkdir","/data/thh/tee_04","0770"},
                  {"mkdir","/data/thh/tee_05","0770"},
                  {"mkdir","/data/thh/tee_00","0770"},
		  {"mkdir","/data/thh/tee_06","0770"}};

#define SOTER_WORK_PATH "/data/thh/system"
#define IMAGE_ORI_PATH  "/vendor/thh"
#define IMG_ORI 0
#define IMG_DST_DIR 1
#define IMG_DST_NAME 2
#define IMG_MD5_ORI 3
#define IMG_MD5_CUR 4

char * table[4][6] = {{"/vendor/thh/soter.raw","/data/thh/system","/data/thh/vendor/soter.raw","/data/thh/vendor/soter.ori.md5","/data/thh/vendor/soter.cur.md5"},
/*char * table_tee_01[] =*/ {"/vendor/thh/alipayapp","/data/thh/tee_01","/data/thh/tee_01/tee","/data/thh/tee_01/tee.ori.md5","/data/thh/tee_01/tee.cur.md5"},
                            {"/vendor/thh/fp_server","/data/thh/tee_05","/data/thh/tee_05/tee","/data/thh/tee_05/tee.ori.md5","/data/thh/tee_05/tee.cur.md5"},
			    {"/vendor/thh/uTAgent","/data/thh/tee_00","/data/thh/tee_00/tee","/data/thh/thh_00/tee.ori.md5","/data/thh/tee_00/tee.cur.md5"}};

static void showMD5(unsigned char *data,char * des)
{
    //int i = 0;
    char md5[33] = {0};
    memcpy(md5,data,32);
    LOGI("[MD5](%s:%s)",des,md5);
    //sprintf(md5,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
    //        data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15]);
    //LOGI("[%s]",md5);
}
static int get_image_md5_from_file(char *path, unsigned char *data)
{
    int fd = 0;
    int cnt = 0;

    fd = open(path, O_RDONLY, 0775);
    if (0 > fd) {
        LOGE("can't read md5 file :%s",path);
        return 0;
    }

    cnt = read(fd, data, 32);

    if (cnt == 0 || (data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 0 && data[4] == 0 && data[5] == 0 && data[6] == 0 && data[7] == 0 &&
        data[8] == 0 && data[9] == 0 && data[10] == 0 && data[11] == 0 && data[12] == 0 && data[13] == 0 && data[14] == 0 && data[15] == 0))
    {
        LOGE("file %s md5 == 0",path);
        close(fd);
        return 0;
    }

    //LOGI("cnt %d",cnt);
    showMD5(data,path);
    close(fd);
    return 1;
}

static void write_md5_file(char *path, unsigned char *data)
{
    int fd = 0;

    fd = open(path, O_CREAT|O_WRONLY, 0775);
    if (0 > fd) {
        LOGE("can't create md5 file :%s", path);
        return;
    }

    write(fd, data, 32);
    close(fd);

    return;
}
static int is_equal_md5(unsigned char *src, unsigned char *dst)
{
    int i = 0;

    if (NULL == src || dst == NULL) {
        LOGE("md5 must not empty !");
        return 0;
    }

    for ( i = 0; i < 32; i++) {
        if (src[i] != dst[i])
            return 0;
    }

    return 1;
}

static int is_ota_update()
{
   //for lenovo, only lenovo will create /cache/ota/updateResult after ota.
   //return (get_ota_status()==OTA_REBOOT);
   //for others
    LOGI("==== Force check whether the origin image is updated , if need restore it ====");
    return 1;
}

static void create_tee_storage(char *table[], int index)
{
    unsigned char md5_ori_image[33]={0};
    unsigned char ori_file[33]={0};
    unsigned char md5_work[33] = {0};
    unsigned char md5_cur_file[33] = {0};

    LOGI("-----------------------------");
    if (is_file_exist(table[IMG_DST_DIR])) {
        /* caclulate md5 from file */
        LOGI("%s is exists", table[IMG_DST_DIR]);
        LOGI("get md5 from image");
        if (!get_image_md5(table[IMG_DST_NAME], md5_work)) {
            goto restore_image;
        }

        //showMD5(md5_work,table[IMG_DST_NAME]);
        LOGI("get md5 from cur file");
        if (!get_image_md5_from_file(table[IMG_MD5_CUR], md5_cur_file)) {
            goto restore_image;
        }
        //showMD5(md5_cur_file,table[IMG_MD5_CUR]);
        if (is_equal_md5(md5_work, md5_cur_file)) {
            LOGI("md5 == table.cur.md5");
            if (is_ota_update()) {
               LOGI("OTA == Y");
               //showMD5(ori_file,table[IMG_MD5_ORI]);
               if (!get_image_md5_from_file(table[IMG_MD5_ORI], ori_file)) {
                   goto restore_image;
               }
               showMD5(ori_file,table[IMG_MD5_ORI]);
               if (!get_image_md5(table[IMG_ORI], md5_ori_image)) {
                   goto restore_image;
               }
               showMD5(ori_file, table[IMG_MD5_ORI]);
               //showMD5(ori_file,table[IMG_MD5_ORI]);
               //showMD5(md5_ori_image,table[IMG_ORI]);
               if (is_equal_md5(ori_file, md5_ori_image)) {
                   LOGI("*.ori.md5 == *.ori.md5");
                   goto creat_done;
               } else {
                   LOGI("*.ori.md5 != *.ori.md5");
                   goto restore_image;
               }

            } else {
                LOGI("OTA == N");
                goto creat_done;
            }
        } else {
            LOGI("md5 != table.cur.md5");
            goto restore_image;
        }

    } else {
        LOGI("%s is  not exists", table[IMG_DST_DIR]);
        goto restore_image;
    }
restore_image:
    LOGI("need restore_images");
    do_mkdir(3, dir5);
    do_mkdir(3, dirs[index]);
    unsigned char md5_src[32] = {0};
    get_image_md5(table[IMG_ORI],md5_src);
    write_md5_file(table[IMG_MD5_ORI], md5_src);
    write_md5_file(table[IMG_MD5_CUR], md5_src);
    COPY_FILE_FULLPATH(image_raw, table[IMG_ORI], table[IMG_DST_NAME]);
creat_done:
    LOGI("-----------------------------");
    return ;


}
#if 0

static void creat_tee_storage()
{
    char * dir5[]={"mkdir","/data/thh","0777"};
    char * dir0[]={"mkdir","/data/thh/system","0777"};
    char * dir1[]={"mkdir","/data/thh/tee_01","0777"};
    char * dir2[]={"mkdir","/data/thh/tee_02","0777"};
    char * dir3[]={"mkdir","/data/thh/tee_03","0777"};
    char * dir4[]={"mkdir","/data/thh/tee_04","0777"};

    int need_copy_files = 0;
    do_mkdir(3,dir5);
    errno = 0;
    mkdir_err_exist = 0;
    do_mkdir(3,dir0);
    //if thh/system already exists, it is not init stage.
    //Do not copy files.
    if(mkdir_err_exist)
    {
        need_copy_files = 0;
    }else{
        need_copy_files = 1;
    }
    do_mkdir(3,dir1);
    do_mkdir(3,dir2);
    do_mkdir(3,dir3);
    do_mkdir(3,dir4);
    if(need_copy_files)copy_files();

}
#endif
#if LOAD_TZ_DRIVER
static void load_tz_driver()
{
    char opts[1024] = {'\0'};
    int a = argc;
    char b = argv[0][0];
    int ret = init_module(vfsFun_ko, vfsFun_ko_len, opts);

    LOGD("start teei loading ...");

    if (ret != 0) {
        LOGD("failed (%s)", strerror(errno));
    }

    LOGD("loading result:%d", ret);
    ret = init_module(teei_ko, teei_ko_len, opts);
    if (ret != 0) {
        LOGD("failed (%s)", strerror(errno));
    }

    LOGD("loading result:%d", ret);

    return ret;

}
#endif

int analysis_command(unsigned char* p_buffer)
{

    long retVal = 0;
    int responseLen = 0;
    struct TEEI_vfs_command *tz_command = NULL;
    union TEEI_vfs_response tz_response;
    char *pathname = NULL;
    char *param_address = NULL;
    int namelength = 0;

    /* unsigned char *ipAddr = NULL; */
    /* struct hostent *hostEntry = NULL; */

    tz_command = (struct TEEI_vfs_command *)p_buffer;
    responseLen = sizeof(tz_response);

    switch (tz_command->func) {
        case TZ_VFS_OPEN:
            LOGD(" come into the TZ_VFS_OPEN function\n");

            pathname = (char *)((unsigned long)(tz_command) + PAGE_SIZE_4K);
            LOGD("pathname = %s\n", pathname);
            LOGD("flags = %d\n", tz_command->args.func_open_args.flags);
            LOGD("mode = %d\n", tz_command->args.func_open_args.mode);

            retVal = open(pathname,
                          tz_command->args.func_open_args.flags,
                          tz_command->args.func_open_args.mode);

            if(retVal == -1)
            {
                LOGD(" errno = %d\n", errno);
                retVal = -(errno);
            }
            LOGD("retVal = %ld\n", retVal);

            memset((void *)p_buffer, '\0', responseLen);
            ((union TEEI_vfs_response *)p_buffer)->value = retVal;
            return responseLen;

        case TZ_VFS_READ:

            LOGD(" come into the TZ_VFS_READ function\n");

            param_address = (char *)((unsigned long)(tz_command) + PAGE_SIZE_4K);

            retVal = read(tz_command->args.func_read_args.fd, (char *)param_address, tz_command->args.func_read_args.count);

            if(retVal == -1)
            {
                retVal = -(errno);
            }

            memset((void *)p_buffer, '\0', responseLen);
            ((union TEEI_vfs_response *)p_buffer)->value = retVal;

            if(retVal == -1)
            {
                return responseLen;
            }
            else
            {
                return PAGE_SIZE_4K + retVal;
            }

        case TZ_VFS_WRITE:

            LOGD(" come into the TZ_VFS_WRITE function\n");
            LOGD("fd = %d\n", tz_command->args.func_write_args.fd);
            LOGD("count = %d\n", tz_command->args.func_write_args.count);

            param_address = (char *)((unsigned long)(tz_command) + PAGE_SIZE_4K);

            retVal = write(tz_command->args.func_write_args.fd,
                           (const char *)param_address,
                           tz_command->args.func_write_args.count);

            if(retVal == -1)
            {
                retVal = -(errno);
            }

            memset((void *)p_buffer, '\0', responseLen);
            ((union TEEI_vfs_response *)p_buffer)->value = retVal;

            LOGD(" come into the TZ_VFS_WRITE function retVal = %ld\n", retVal);

            if(retVal > 0)
            {
                return PAGE_SIZE_4K + retVal;
            }
            else
            {
                return responseLen;
            }

        case TZ_VFS_IOCTL:

            LOGD(" come into the TZ_VFS_IOCTL function\n");
            LOGD(" fd = %d\n", tz_command->args.func_ioctl_args.fd);
            LOGD(" cmd = %d\n", tz_command->args.func_ioctl_args.cmd);

            param_address = (char *)((unsigned long)(tz_command) + PAGE_SIZE_4K);

            retVal = ioctl(tz_command->args.func_ioctl_args.fd,
                           tz_command->args.func_ioctl_args.cmd,
                           (void*)param_address);

            if(retVal == -1)
            {
                retVal = -(errno);
            }

            LOGD(" retVal = %x\n", *((int *)param_address));
            memset((void *)p_buffer, '\0', responseLen);
            ((union TEEI_vfs_response *)p_buffer)->value = retVal;
            return PAGE_SIZE_4K + 0x9000;

        case TZ_VFS_CLOSE:

            LOGD(" come into the TZ_VFS_CLOSE function\n");
            LOGD(" fd = %d\n", tz_command->args.func_close_args.fd);

            retVal = close(tz_command->args.func_close_args.fd);

            if(retVal == -1)
            {
                retVal = -(errno);
            }

            LOGD(" retVal = %ld\n", retVal);
            memset((void*)p_buffer, '\0', responseLen);
            ((union TEEI_vfs_response *)p_buffer)->value = retVal;
            return responseLen;

        case TZ_VFS_TRUNC:

            LOGD(" come into the TZ_VFS_TRUNC function\n");

            retVal = ftruncate(tz_command->args.func_trunc_args.fd,
                               tz_command->args.func_trunc_args.length);

            if(retVal == -1)
            {
                retVal = -(errno);
            }

            memset((void *)p_buffer, '\0', responseLen);
            ((union TEEI_vfs_response *)p_buffer)->value = retVal;
            return responseLen;

        case TZ_VFS_UNLINK:

            LOGD(" come into the TZ_VFS_UNLINK function\n");

            param_address = (char *)((unsigned long)(tz_command) + PAGE_SIZE_4K);

            retVal = unlink((char *)param_address);

            if(retVal == -1)
            {
                retVal = -(errno);
            }

            memset((void *)p_buffer, '\0', responseLen);
            ((union TEEI_vfs_response *)p_buffer)->value = retVal;
            return responseLen;

        case TZ_VFS_LSEEK:

            LOGD(" come into the TZ_VFS_LSEEK function\n");

            retVal = lseek(tz_command->args.func_lseek_args.fd,
                           tz_command->args.func_lseek_args.offset,
                           tz_command->args.func_lseek_args.origin);

            if(retVal == -1)
            {
                retVal = -(errno);
            }

            memset((void *)p_buffer, '\0', responseLen);
            ((union TEEI_vfs_response *)p_buffer)->value = retVal;
            return responseLen;

        case TZ_VFS_RENAME:

            LOGD(" come into the TZ_VFS_RENAME function\n");

            param_address = (char *)((unsigned long)(tz_command) + PAGE_SIZE_4K);

            namelength = strlen((char *)param_address);

            LOGD(" namelength = %d\n", namelength);
            LOGD(" 1st name = %s\n", (char *)param_address);
            LOGD(" 2nd name = %s\n", (char *)((char *)param_address + namelength + 1));

            retVal = rename((char *)param_address, (char *)(param_address + namelength + 1));

            if(retVal == -1)
            {
                retVal = -(errno);
            }
            LOGD(" retVal = %ld\n", retVal);

            memset((void *)p_buffer, '\0', responseLen);
            ((union TEEI_vfs_response *)p_buffer)->value = retVal;
            return responseLen;

         case TZ_VFS_MKDIR:

            LOGD(" come into the TZ_VFS_MKDIR function\n");

            param_address = (char *)((unsigned long)(tz_command) + PAGE_SIZE_4K);

            LOGD(" 1st name = %s\n", (char *)param_address);
            LOGD(" mode = %d\n", tz_command->args.func_mkdir_args.mode);

            retVal = mkdir((char *)param_address, tz_command->args.func_mkdir_args.mode);

            if(retVal == -1)
            {
                retVal = -(errno);
            }
            LOGD(" retVal = %ld\n", retVal);

            memset((void*)p_buffer, '\0', responseLen);
            ((union TEEI_vfs_response *)p_buffer)->value = retVal;
            return responseLen;

         case TZ_VFS_RMDIR:

            LOGD(" come into the TZ_VFS_RMDIR function\n");

            param_address = (char *)((unsigned long)(tz_command) + PAGE_SIZE_4K);
            LOGD(" 1st name = %s\n", (char *)param_address);

            retVal = rmdir((char *)param_address);

            if(retVal == -1)
            {
                retVal = -(errno);
            }
            LOGD(" retVal = %ld\n", retVal);

            memset((void *)p_buffer, '\0', responseLen);
            ((union TEEI_vfs_response *)p_buffer)->value = retVal;
            return responseLen;

         default:

            LOGD(" Come into the default branch!!!\n");

            memset(&tz_response, '\0', responseLen);
            tz_response.value = -EOPNOTSUPP;
            memset((void *)p_buffer, '\0', responseLen);
            memcpy((void *)p_buffer, &tz_response, responseLen);
            return responseLen;

     }

    return 0;

}

static void init_TEEI_OS()
{
    int fd = 0;
    int flag = 0;
    int ret = 0;

    fd = open("/dev/teei_config", O_RDWR);
    if (fd < 0) {
        LOGE("Can not open teei_confit ,please fix it !");
        return;
    }

    ret = ioctl(fd, TEEI_CONFIG_IOCTL_INIT_TEEI, &flag);
    if (0 != ret) {
        LOGE("Can not init Soter OS ,please fix it !");
    } else {
        LOGI("begin to init TEEI OS OK");
    }
    close(fd);
    return;
}

#if 1
static int init_OS_fn(void)
{

    LOGI("begin to init TEEI OS");
    init_TEEI_OS();
    property_set("soter.teei.init", "INIT_OK");

    return 0;
}
#endif

/****************************KEYMASTER START***************************/

#define CMD_NOTIFY_UTD  _IO(0x5B777E, 0x3)

/* for unlocking keymaster,and then can load tee in (kernel/tz_driver) */

static void keymaster_unlock(void)
{
    int fd = 0;
    int flag = 0;
    int ret = 0;

    fd = open("/dev/ut_keymaster", O_RDWR);
    if (fd < 0) {
        LOGE("Can not open keymaster");
        return;
    }

    ret = ioctl(fd,CMD_NOTIFY_UTD,&flag);
    if (0 != ret) {
        LOGE("Can not load tees ,please fix it !");
    } else {
        LOGI("begin to load tees");
    }

    close(fd);
    return;
}

/* for the system first time boot, then the keymaster can store the key in rpmb block 38, this rpmb place is just for all disk encrypt rsa key */
#define CMD_FIRST_TIME_BOOT  _IO(0x5B777E, 0x4)
#define CMD_ID_FIREST_TIME_BOOT 101
typedef struct{
	unsigned int command_id;
	unsigned int command_data;  //if command_data=0xaf015838, this indicates that the phone is the first time boot.
}keymaster_command_t;

static void keymaster_firstboot(void)
{
	unsigned char *temp_buffer;
	temp_buffer = malloc(sizeof(keymaster_command_t));
	keymaster_command_t keymaster_command;
	keymaster_command.command_id = CMD_ID_FIREST_TIME_BOOT;
	keymaster_command.command_data = 0x0;

	char value[PROPERTY_VALUE_MAX];
    property_get("ro.crypto.state", value, "");
	if(strcmp("",value) ==0)
    {
		keymaster_command.command_data = 0xaf015838;
		memcpy(temp_buffer, &keymaster_command, sizeof(keymaster_command_t));
		int fd=open("/dev/ut_keymaster",O_RDWR);
		int flag = 0;
		int ret = ioctl(fd,CMD_FIRST_TIME_BOOT,temp_buffer);
		if (0 != ret) {
		    LOGE("Can not load tees ,please fix it !");
		}
		else {
		    LOGI("begin to load tees");
		}
		memcpy( &keymaster_command, temp_buffer, sizeof(keymaster_command_t));
		close(fd);
	}

	free(temp_buffer);
}

static int keymaster_first_time_fn(void)
{

    keymaster_firstboot();
    return 0;
}
static int loadtee_fn(void)
{
    LOGI("running in loadtee_fn.");

    char value[PROPERTY_VALUE_MAX];
    char encrypt_state[PROPERTY_VALUE_MAX];
    property_get("ro.crypto.state", value, "");
    if(strcmp("unencrypted",value) != 0 && strcmp("unsupported",value) != 0) {
        /*data encrypted, wait for decrption.*/
        property_get("vold.decrypt", value, "");
        while(strcmp("trigger_restart_framework", value) != 0) {
            /*still decrypting... wait one second.*/
            sleep(1);
            LOGI("====wait for all data encrypt, based keymaster function===!");
            property_get("vold.decrypt", value, "");
        }
    }

    //restorecon sepolicy
    property_get("soter.encrypt.state", encrypt_state, "");
    while(strcmp("OK", encrypt_state) != 0){
      sleep(1);
      property_get("soter.encrypt.state", encrypt_state, "");
    }

    LOGI("create tee storage ...");
    /*create_tee_storage(table[0],0);*/
    create_tee_storage(table[1], 1);
    create_tee_storage(table[2], 5);
    create_tee_storage(table[3], 6);

    do_mkdir(3, dirs[2]);
    do_mkdir(3, dirs[3]);
    do_mkdir(3, dirs[4]);
    do_mkdir(3, dirs[7]);

    LOGI("create tee storage OK ...");
    LOGI("link ota cert update ...");
    if(is_ota_update()) {
        LOGI("link cert file to update ...");
        do_ota_cert_update();
        LOGI("link cert file to done.");
    }

    keymaster_unlock();
//	gatekeeper_unlock();

    return 0;
}
/****************************KEYMASTER END***************************/

//char *check_point = "/data"; //ATTENTION: DIR NOT WORK. MUST BE FILE.
#if 0
char *check_point = "/data/vendor/appops.xml";
#endif
int main(int argc, char **argv)
{
    int vfs_fd = 0;
    int len = 0;
    int retVal = 0;
    int writeLen = 0;
    unsigned char* rw_buffer = NULL;

    pthread_t ntid = 0;
    pthread_t loadtee_id =0;
    pthread_t first_time_boot_id = 0;

    int debug = 0;
    (void)argc;(void)argv;
    klog_init();

    LOGE("TEEI Daemon VERSION [%s]", UTOS_VERSION);
    LOGI("TEEI Daemon VERSION [%s]", UTOS_VERSION);
    LOGI("TEEI Daemon start ...");
    property_set("soter.teei.init", "INIT_START");

    //sleep(15);

#if 0
    if(0){
		int fd = open(check_point,O_RDONLY|O_NONBLOCK);
		#if 1
		while(fd<=0)
		{
			sleep(1);
		    fd = open(check_point,O_RDONLY|O_NONBLOCK);
	    };
	    #endif
		LOGD("[teei_daemon]fd = %d \n",fd);
		char temp[64] = {0};
		read(fd,&temp,63);
		LOGD("[teei_daemon]read  = %s \n",temp);
		close(fd);
	}
#endif


#if 0
    LOGI("check ota update ...");
    if (get_ota_status() == OTA_REBOOT)
    {
        do_ota_update();
    }
#endif

    //pthread_create(&ntid, NULL, (void*)init_OS_fn, NULL);

#if 0
    if (!__teei_debug__)
    {
        init_TEEI_OS();
    }
    chmod_dev_node();
    LOGI("chmod dev node OK ...");

    property_set("soter.teei.init", "INIT_OK");
#endif



#if 1


    rw_buffer = (unsigned char *)malloc(BUFFER_SIZE);
    if(rw_buffer == NULL) {
        LOGD("[E] %s : %d Can not malloc enough memory.\n", __func__, __LINE__);
        return -1;
    }


    while(1) {
        vfs_fd = open(DEV_FILE, O_RDWR);
        if(vfs_fd < 0) {
            LOGD("[E] %s : %d Can not open the device node.\n", __func__, __LINE__);
            continue;
        }
        break;
    }

    pthread_create(&ntid, NULL, (void *)init_OS_fn, NULL);
    pthread_create(&first_time_boot_id, NULL, (void *)keymaster_first_time_fn, NULL);
    /*create a thread for start data area working*/
    pthread_create(&loadtee_id, NULL, (void *)loadtee_fn, NULL);

    while(1) {
	len = 31232;
        len = read(vfs_fd, rw_buffer, BUFFER_SIZE);
	LOGD("[D] %s : %d read result = %d.\n", __func__, __LINE__, len);
        if(len < 0) {
            LOGD("[E] %s : %d Can not read the VFS device node, len = [%d] errno = [%d].\n", __func__, __LINE__, len, errno);
            continue;
        }
        retVal = analysis_command(rw_buffer);
        if(retVal < 0) {
            LOGD("[E] %s : %d Invail command read from VFS device node.\n", __func__, __LINE__);
            continue;
        }
        writeLen = retVal;
        retVal = write(vfs_fd, rw_buffer, writeLen);
        if(retVal < 0) {
            LOGD("[E] %s : %d Can not write to VFS device node.\n", __func__, __LINE__);
            continue;
        }
    }

    free(rw_buffer);
    close(vfs_fd);
#endif

    LOGI("TEEI Daemon start OK ...");
    return 0;
}
