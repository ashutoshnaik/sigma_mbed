/*************************************************************************************************/
/*!
 *  \file   wsf_buf.h
 *        
 *  \brief  Buffer pool service.
 *
 *          $Date: 2013-05-13 15:20:24 -0700 (Mon, 13 May 2013) $
 *          $Revision: 612 $
 *  
 *  Copyright (c) 2009 Wicentric, Inc., all rights reserved.
 *  Wicentric confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact Wicentric, Inc. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/
#ifndef WSF_BUF_H
#define WSF_BUF_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! Length of the buffer statistics array */
#define WSF_BUF_STATS_MAX_LEN       128

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! Buffer pool descriptor structure */
typedef struct
{
  uint16_t   len;                  /*! length of buffers in pool */
  uint8_t    num;                  /*! number of buffers in pool */
} wsfBufPoolDesc_t;


/**************************************************************************************************
  Function Declarations
**************************************************************************************************/

/*************************************************************************************************/
/*!
 *  \fn     WsfBufInit
 *        
 *  \brief  Initialize the buffer pool service.  This function should only be called once
 *          upon system initialization.
 *
 *  \param  bufMemLen Length in bytes of memory pointed to by pBufMem.
 *  \param  pBufMem   Memory in which to store the pools used by the buffer pool service.
 *  \param  numPools  Number of buffer pools.
 *  \param  pDesc     Array of buffer pool descriptors, one for each pool.
 *
 *  \return TRUE if initialization was successful, FALSE otherwise.
 */
/*************************************************************************************************/
bool_t WsfBufInit(uint16_t bufMemLen, uint8_t *pBufMem, uint8_t numPools, wsfBufPoolDesc_t *pDesc);

/*************************************************************************************************/
/*!
 *  \fn     WsfBufAlloc
 *        
 *  \brief  Allocate a buffer.
 *
 *  \param  len     Length of buffer to allocate.
 *
 *  \return Pointer to allocated buffer or NULL if allocation fails.
 */
/*************************************************************************************************/
void *WsfBufAlloc(uint16_t len);

/*************************************************************************************************/
/*!
 *  \fn     WsfBufFree
 *        
 *  \brief  Free a buffer.
 *
 *  \param  pBuf    Buffer to free.
 *
 *  \return None.
 */
/*************************************************************************************************/
void WsfBufFree(void *pBuf);

/*************************************************************************************************/
/*!
 *  \fn     WsfBufGetMaxAlloc
 *        
 *  \brief  Diagnostic function to get maximum allocated buffers from a pool.
 *
 *  \param  pool    Buffer pool number.
 *
 *  \return Number of allocated buffers.
 */
/*************************************************************************************************/
uint8_t WsfBufGetMaxAlloc(uint8_t pool);

/*************************************************************************************************/
/*!
 *  \fn     WsfBufGetNumAlloc
 *        
 *  \brief  Diagnostic function to get the number of currently allocated buffers in a pool.
 *
 *  \param  pool    Buffer pool number.
 *
 *  \return Number of allocated buffers.
 */
/*************************************************************************************************/
uint8_t WsfBufGetNumAlloc(uint8_t pool);

/*************************************************************************************************/
/*!
 *  \fn     WsfBufGetAllocStats
 *        
 *  \brief  Diagnostic function to get the buffer allocation statistics.
 *
 *  \return Buffer allocation statistics array.
 */
/*************************************************************************************************/
uint8_t *WsfBufGetAllocStats(void);


#ifdef __cplusplus
};
#endif

#endif /* WSF_BUF_H */
