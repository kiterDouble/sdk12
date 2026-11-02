#ifndef SR_TSP_LOGIN_H
#define SR_TSP_LOGIN_H

#ifdef __cplusplus
extern "C"
{
#endif

#define LOGIN_RESULT_SUCCESS 0
#define LOGIN_RESULT_FAILURE 1

    /**
     * @brief 主入口：执行 TSP 平台登录流程
     *
     * @return int RET_OK 成功, RET_ERR 失败
     */
    int SRTspPlatformLogin(void);

#ifdef __cplusplus
}
#endif
#endif // SRTSPLOGIN_H
