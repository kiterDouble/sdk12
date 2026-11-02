/**
 * @brief 
 * 
 * @file sr_cache_tsp.h
 * @author lujiefeng
 * @date 2018-08-28
 */

#ifndef _SR_CACHE_QUEUE_H
#define _SR_CACHE_QUEUE_H
#include <stdint.h>
#include <glib.h>
#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief CacheData
     * 
     * @param[in]	u8buff	
     * @param[in]	u32length	the len of u8buff
     */
    void CacheDataToQueue(GAsyncQueue *queue, uint8_t *u8buff, uint32_t u32length);

    /**
     * @brief Get the Data From Queue object
     * 
     * @param[in]	queue	
     * @param[out]	u8buff	
     * @param[out]	u32length	
     * @retval	
     */
    int32_t GetDataFromQueue(GAsyncQueue *queue, uint8_t *u8buff, uint32_t *u32length);

    /**
     * @brief Get the Data With Time Out From Queue object
     * 
     * @param[in]	queue	
     * @param[out]	u8buff	
     * @param[out]	u32length	
     * @param[in]	ms	
     * @retval	
     */
    int32_t GetDataWithTimeOutFromQueue(GAsyncQueue *queue, uint8_t *u8buff, uint32_t *u32length, uint32_t ms);

    /**
     * @brief 
     * 
     * @retval	
     */
    GAsyncQueue *CreadNewCacheQueue();
    /**
     * @brief Get the Data With Time Out From Queue object
     * 
     * @param[in]	queue	
     * @retval	
     */
    int32_t GetLengthFromQueue(GAsyncQueue *queue);
    /**
     * @brief CacheDataToQueueWithoutMalloc
     * 
     * @param[in] queue 
     * @param[in] pData 
     */
    void CacheDataToQueueWithoutMalloc(GAsyncQueue *queue, gpointer pData);
    /**
     * @brief Get the Data From Queue Without Free object
     * 
     * @param[in] queue 
     * @param[out] pData 
     * @return int32_t 
     */
    int32_t GetDataFromQueueWithoutFree(GAsyncQueue *queue, gpointer *pData);
    /**
     * @brief Get the Data With Time Out From Queue Without Free object
     * 
     * @param[in] queue 
     * @param[out] pData 
     * @param[in] ms 
     * @return int32_t 
     */
    int32_t GetDataWithTimeOutFromQueueWithoutFree(GAsyncQueue *queue, gpointer *pData, uint32_t ms);
#ifdef __cplusplus
}
#endif
#endif