#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "property_util.h"

#define CREATE_DIR_PATH_MAX_SIZE 512
#define CREATE_DIR_DEFAULT_MODE  0755

/*
* @brief 在父目录 fd 下创建目录，使用 openat(O_DIRECTORY|O_NOFOLLOW) 避免 TOCTOU
* @param parentFd: 父目录文件描述符
* @param name: 待创建目录名
* @return 成功返回新目录 fd，失败返回 -1
*/
static int CreateDirAt(int parentFd, const char* name)
{
    int fd = -1;
    struct stat st;

    if (mkdirat(parentFd, name, CREATE_DIR_DEFAULT_MODE) == 0)
    {
        fd = openat(parentFd, name, O_DIRECTORY | O_RDONLY | O_NOFOLLOW);
        if (fd < 0)
        {
            return -1;
        }
        return fd;
    }

    if (errno != EEXIST)
    {
        return -1;
    }

    /* 已存在，验证是目录且不是符号链接 */
    fd = openat(parentFd, name, O_DIRECTORY | O_RDONLY | O_NOFOLLOW);
    if (fd < 0)
    {
        return -1;
    }

    if (fstat(fd, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        close(fd);
        return -1;
    }

    return fd;
}

/*
* @brief 检查并创建输入文件/文件夹 所在路径是否存在，不存在则创建
* @param filePath: 文件/文件夹路径
*/
int CreateFileDirectory(const char* filePath)
{
    char dirPath[CREATE_DIR_PATH_MAX_SIZE] = {0};
    char* component = NULL;
    char* saveptr = NULL;
    char* p = NULL;
    const char* baseDir = NULL;
    int dirFd = -1;
    int newFd = -1;

    if (filePath == NULL)
    {
        return -1;
    }

    if (strlen(filePath) >= CREATE_DIR_PATH_MAX_SIZE)
    {
        return -1;
    }

    /* 复制文件路径到目录路径缓冲区 */
    strncpy(dirPath, filePath, sizeof(dirPath) - 1);

    /* 查找最后一个路径分隔符，仅保留目录部分 */
    p = strrchr(dirPath, '/');
    if (p != NULL)
    {
        *p = '\0';
    }
    else
    {
        /* 没有路径分隔符，无需创建目录 */
        return 0;
    }

    /* 如果目录部分为空（例如路径为 "/file"），根目录已存在 */
    if (dirPath[0] == '\0')
    {
        return 0;
    }

    /* 根据绝对/相对路径选择基准目录，全部使用 openat 避免 TOCTOU */
    if (dirPath[0] == '/')
    {
        baseDir = "/";
        p = dirPath + 1;  /* 跳过开头的 '/'，由 strtok_r 继续处理 */
    }
    else
    {
        baseDir = ".";
        p = dirPath;
    }

    dirFd = open(baseDir, O_DIRECTORY | O_RDONLY | O_NOFOLLOW);
    if (dirFd < 0)
    {
        return -1;
    }

    /* 使用 strtok_r 分割路径组件，自动跳过连续斜杠和空组件 */
    component = strtok_r(p, "/", &saveptr);
    while (component != NULL)
    {
        if (component[0] == '\0')
        {
            component = strtok_r(NULL, "/", &saveptr);
            continue;
        }

        newFd = CreateDirAt(dirFd, component);
        close(dirFd);
        if (newFd < 0)
        {
            return -1;
        }
        dirFd = newFd;

        component = strtok_r(NULL, "/", &saveptr);
    }

    close(dirFd);
    return 0;
}
