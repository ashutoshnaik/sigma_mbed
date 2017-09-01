/** \file fit_main.c ******************************************************
 *
 *             Project: MAXREFDES73
 *             Filename: fit_main.c
 *         Description: This module contains the implementation of the BLE profile:
 *         Heart Rate Profile. The MAXREFDES73# GSR device communicates with a
 *         device for Android based on BLE heart rate profile.
 *
 *    Revision History:
 *\n                    06-02-2015    Rev 02.00    MG    Initial release.
 *
 *
 * Copyright (c) 2011 Wicentric, Inc., all rights reserved.
 * Wicentric confidential and proprietary.
 *
 * Copyright (C) 2012 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED PRODUCTS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated Products
 * shall not be used except as stated in the Maxim Integrated Products
 * Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products retains all ownership rights.
 *
 ***************************************************************************/

#include "mxc_config.h"

#include <string.h>
#include "wsf_types.h"
#include "bstream.h"
#include "wsf_msg.h"
#include "wsf_trace.h"
#include "hci_api.h"
#include "dm_api.h"
#include "att_api.h"
#include "app_api.h"
#include "app_db.h"
#include "app_ui.h"
#include "app_hw.h"
#include "svc_ch.h"
#include "svc_core.h"
#include "svc_hrs.h"
#include "svc_dis.h"
#include "svc_batt.h"
#include "bas_api.h"
#include "hrps_api.h"
#include "gpio.h"
#include "rtc.h"

/*************************************************************************************************
  Macros
**************************************************************************************************/
extern uint8_t BLEadvertising, BLEconnected;


/* LED pin mapping on the MAXREFDES73 */
#define LED_ON          0
#define LED_OFF         1

#define LED_PORT        0
#define RED_LED_PIN     7
#define GREEN_LED_PIN   6

/*! WSF message event starting value */
#define FIT_MSG_START               0xA0

/*! WSF message event enumeration */
enum {
    FIT_HR_TIMER_IND = FIT_MSG_START,       /*! Heart rate measurement timer expired */
    FIT_BATT_TIMER_IND                      /*! Battery measurement timer expired */
};

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! Application message type */
typedef union
{
    wsfMsgHdr_t     hdr;
    dmEvt_t         dm;
    attsCccEvt_t    ccc;
} fitMsg_t;

/**************************************************************************************************
  Configurable Parameters
**************************************************************************************************/

/*! configurable parameters for slave */
static const appSlaveCfg_t fitSlaveCfg = {
    {
        {  5000,   5000,     0},          /*! advertising durations in ms, connectable mode */
        {  5000,   5000,     0}           /*! advertising durations in ms, discoverable mode */
    },
    {
        {    800,     800,   800},          /*! advertising intervals in 0.625 ms units, connectable mode */
        {    800,     800,   800}           /*! advertising intervals in 0.625 ms units, discoverable mode */
    },
    TRUE                                    /*! TRUE to use the white list in connectable mode */
};

/*! configurable parameters for security */
static const appSecCfg_t fitSecCfg = {
    DM_AUTH_BOND_FLAG,                      /*! Authentication and bonding flags */
    0,                                      /*! Initiator key distribution flags */
    DM_KEY_DIST_LTK,                        /*! Responder key distribution flags */
    FALSE,                                  /*! TRUE if Out-of-band pairing data is present */
    FALSE                                   /*! TRUE to initiate security upon connection */
};

/*! configurable parameters for connection parameter update */
static const appUpdateCfg_t fitUpdateCfg = {
    0,                                      /*! Connection idle period in ms before attempting
                                                connection parameter update; set to zero to disable */
    640,                                    /*! Minimum connection interval in 1.25ms units */
    800,                                    /*! Maximum connection interval in 1.25ms units */
    0,                                      /*! Connection latency */
    600,                                    /*! Supervision timeout in 10ms units */
    5                                       /*! Number of update attempts before giving up */
};

/*! heart rate measurement configuration */
static const hrpsCfg_t fitHrpsCfg = {
    1000                                    /*! Measurement timer expiration period in ms */
};

/*! battery measurement configuration */
static const basCfg_t fitBasCfg = {
    1,                                     /*! Battery measurement timer expiration period in seconds */
    1,                                     /*! Perform battery measurement after this many timer periods */
    300                                    /*! Send battery level notification to peer when below this level. */
};

/**************************************************************************************************
  Advertising Data
**************************************************************************************************/

/*! advertising data, discoverable mode */
static const uint8_t fitAdvDataDisc[] =
    {
        /*! flags */
        2,                                      /*! length */
        DM_ADV_TYPE_FLAGS,                      /*! AD type */
        DM_FLAG_LE_GENERAL_DISC |               /*! flags */
        DM_FLAG_LE_BREDR_NOT_SUP,
  
        /*! tx power */
        2,                                      /*! length */
        DM_ADV_TYPE_TX_POWER,                   /*! AD type */
        0,                                      /*! tx power */

        /*! service UUID list */
        7,                                      /*! length */
        DM_ADV_TYPE_16_UUID,                    /*! AD type */
        UINT16_TO_BYTES(ATT_UUID_HEART_RATE_SERVICE),
        UINT16_TO_BYTES(ATT_UUID_DEVICE_INFO_SERVICE),
        UINT16_TO_BYTES(ATT_UUID_BATTERY_SERVICE)  
    };

/*! scan data, discoverable mode */
static const uint8_t fitScanDataDisc[] =
    {
        /*! device name */
        13,                                     /*! length */
        DM_ADV_TYPE_LOCAL_NAME,                 /*! AD type */
        'M',
        'A',
        'X',
        'R',
        'E',
        'F',
        'D',
        'E',
        'S',
        '7',
        '3',
        '#'
    };

/**************************************************************************************************
  Client Characteristic Configuration Descriptors
**************************************************************************************************/

/*! enumeration of client characteristic configuration descriptors */
enum
    {
        FIT_GATT_SC_CCC_IDX,                    /*! GATT service, service changed characteristic */
        FIT_HRS_HRM_CCC_IDX,                    /*! Heart rate service, heart rate monitor characteristic */
        FIT_BATT_LVL_CCC_IDX,                   /*! Battery service, battery level characteristic */
        FIT_NUM_CCC_IDX
    };

/*! client characteristic configuration descriptors settings, indexed by above enumeration */
static const attsCccSet_t fitCccSet[FIT_NUM_CCC_IDX] =
    {
        /* cccd handle          value range               security level */
        {GATT_SC_CH_CCC_HDL,    ATT_CLIENT_CFG_INDICATE,  DM_SEC_LEVEL_NONE},   /* FIT_GATT_SC_CCC_IDX */
        {HRS_HRM_CH_CCC_HDL,    ATT_CLIENT_CFG_NOTIFY,    DM_SEC_LEVEL_NONE},   /* FIT_HRS_HRM_CCC_IDX */
        {BATT_LVL_CH_CCC_HDL,   ATT_CLIENT_CFG_NOTIFY,    DM_SEC_LEVEL_NONE}    /* FIT_BATT_LVL_CCC_IDX */
    };

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! WSF handler ID */
wsfHandlerId_t fitHandlerId;

/*************************************************************************************************/
/*!
 *  \fn     fitDmCback
 *        
 *  \brief  Application DM callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void fitDmCback(dmEvt_t *pDmEvt)
{
    dmEvt_t *pMsg;
  
    if ((pMsg = WsfMsgAlloc(sizeof(dmEvt_t))) != NULL)
    {
        memcpy(pMsg, pDmEvt, sizeof(dmEvt_t));
        WsfMsgSend(fitHandlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     fitAttCback
 *        
 *  \brief  Application ATT callback.
 *
 *  \param  pEvt    ATT callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void fitAttCback(attEvt_t *pEvt)
{

	return;
}

/*************************************************************************************************/
/*!
 *  \fn     fitCccCback
 *        
 *  \brief  Application ATTS client characteristic configuration callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void fitCccCback(attsCccEvt_t *pEvt)
{
    attsCccEvt_t  *pMsg;
    appDbHdl_t    dbHdl;
  
    /* if CCC not set from initialization and there's a device record */
    if ((pEvt->handle != ATT_HANDLE_NONE) &&
        ((dbHdl = AppDbGetHdl((dmConnId_t) pEvt->hdr.param)) != APP_DB_HDL_NONE))
    {
        /* store value in device database */  
        AppDbSetCccTblValue(dbHdl, pEvt->idx, pEvt->value);
    }
  
    if ((pMsg = WsfMsgAlloc(sizeof(attsCccEvt_t))) != NULL)
    {
        memcpy(pMsg, pEvt, sizeof(attsCccEvt_t));
        WsfMsgSend(fitHandlerId, pMsg);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     fitProcCccState
 *        
 *  \brief  Process CCC state change.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void fitProcCccState(fitMsg_t *pMsg)
{
    APP_TRACE_INFO3("ccc state ind value:%d handle:%d idx:%d", pMsg->ccc.value,
                    pMsg->ccc.handle, pMsg->ccc.idx);

    /* handle heart rate measurement CCC */
    if (pMsg->ccc.idx == FIT_HRS_HRM_CCC_IDX)
    {
        if (pMsg->ccc.value == ATT_CLIENT_CFG_NOTIFY)
        {
            HrpsMeasStart((dmConnId_t) pMsg->ccc.hdr.param, FIT_HR_TIMER_IND, FIT_HRS_HRM_CCC_IDX);
        }
        else
        {
            HrpsMeasStop();
        }
        return;
    }
  
    /* handle battery level CCC */
    if (pMsg->ccc.idx == FIT_BATT_LVL_CCC_IDX)
    {
        if (pMsg->ccc.value == ATT_CLIENT_CFG_NOTIFY)
        {
            BasMeasBattStart((dmConnId_t) pMsg->ccc.hdr.param, FIT_BATT_TIMER_IND, FIT_BATT_LVL_CCC_IDX);
        }
        else
        {
            BasMeasBattStop();
        }
        return;
    }
}

/*************************************************************************************************/
/*!
 *  \fn     fitClose
 *        
 *  \brief  Perform UI actions on connection close.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void fitClose(fitMsg_t *pMsg)
{
    /* stop heart rate measurement */
    HrpsMeasStop();
  
    /* stop battery measurement */
    BasMeasBattStop();  
}

/*************************************************************************************************/
/*!
 *  \fn     fitSetup
 *        
 *  \brief  Set up advertising and other procedures that need to be performed after
 *          device reset.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void fitSetup(fitMsg_t *pMsg)
{
    bdAddr_t randAddr;
    uint32_t random_num = RTC_GetVal();

    /* set random address to a unique value */
    randAddr[0] = random_num & 0xff;
    randAddr[1] = (random_num >> 8) & 0xff;
    randAddr[2] = (random_num >> 16) & 0xff;
    randAddr[3] = (random_num >> 24) & 0xff;

    random_num /= 2;
    randAddr[4] = random_num & 0xff;
    randAddr[5] = (random_num >> 8) & 0xff;
    
    /* set as static address; this may alter the value of addr[5] */
    DM_RAND_ADDR_SET(randAddr, DM_RAND_ADDR_STATIC);

    /* use random address */
    DmDevSetRandAddr(randAddr);
    DmAdvSetAddrType(DM_ADDR_RANDOM);

    /* set advertising and scan response data for discoverable mode */
    AppAdvSetData(APP_ADV_DATA_DISCOVERABLE, sizeof(fitAdvDataDisc), (uint8_t *) fitAdvDataDisc);
    AppAdvSetData(APP_SCAN_DATA_DISCOVERABLE, sizeof(fitScanDataDisc), (uint8_t *) fitScanDataDisc);

    /* set advertising and scan response data for connectable mode */
    AppAdvSetData(APP_ADV_DATA_CONNECTABLE, 0, NULL);
    AppAdvSetData(APP_SCAN_DATA_CONNECTABLE, 0, NULL);
  
    /* start advertising; automatically set connectable/discoverable mode and bondable mode */
    AppAdvStart(APP_MODE_AUTO_INIT);
}

/*************************************************************************************************/
/*!
 *  \fn     fitBtnCback
 *        
 *  \brief  Button press callback.
 *
 *  \param  btn    Button press.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void fitBtnCback(uint8_t btn)
{
    dmConnId_t      connId;
    static uint8_t  heartRate = 78;    /* for testing/demonstration */
  
    /* button actions when connected */
    if ((connId = AppConnIsOpen()) != DM_CONN_ID_NONE)
    {
        switch (btn)
        {
            case APP_UI_BTN_1_SHORT:
                /* increment the heart rate */
                AppHwHrmTest(++heartRate);
                break;
        
            case APP_UI_BTN_1_MED:
                break;
        
            case APP_UI_BTN_1_LONG:
                AppConnClose(connId);
                break;

            case APP_UI_BTN_2_SHORT:
                /* decrement the heart rate */
                AppHwHrmTest(--heartRate);
                break;
        
            default:
                break;
        }    
    }
    /* button actions when not connected */
    else
    {
        switch (btn)
        {
            case APP_UI_BTN_1_SHORT:
                /* start or restart advertising */
                AppAdvStart(APP_MODE_AUTO_INIT);
                break;
        
            case APP_UI_BTN_1_MED:
                /* enter discoverable and bondable mode mode */
                AppSetBondable(TRUE);
                AppAdvStart(APP_MODE_DISCOVERABLE);
                break;
        
            case APP_UI_BTN_1_LONG:
                /* clear bonded device info and restart advertising */
                AppDbDeleteAllRecords();
                AppAdvStart(APP_MODE_AUTO_INIT);
                break;
        
            default:
                break;
        }
    }
}

/*************************************************************************************************/
/*!
 *  \fn     fitProcMsg
 *        
 *  \brief  Process messages from the event handler.
 *
 *  \param  pMsg    Pointer to message.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void fitProcMsg(fitMsg_t *pMsg)
{
    uint8_t uiEvent = APP_UI_NONE;
  
    switch(pMsg->hdr.event)
    {
        case FIT_HR_TIMER_IND:
            HrpsProcMsg(&pMsg->hdr);
            break;

        case FIT_BATT_TIMER_IND:
            BasProcMsg(&pMsg->hdr);
            break;
      
        case ATTS_CCC_STATE_IND:
            fitProcCccState(pMsg);
            break;
    
        case DM_RESET_CMPL_IND:
            fitSetup(pMsg);
            uiEvent = APP_UI_RESET_CMPL;
            break;

        case DM_ADV_START_IND:
            uiEvent = APP_UI_ADV_START;
            GPIO_SetOutVal(LED_PORT, GREEN_LED_PIN, LED_ON);
            BLEadvertising = 1;
            break;
         
        case DM_ADV_STOP_IND:
            uiEvent = APP_UI_ADV_STOP;
            GPIO_SetOutVal(LED_PORT, GREEN_LED_PIN, LED_OFF);
            BLEadvertising = 0;
            break;
          
        case DM_CONN_OPEN_IND:
            uiEvent = APP_UI_CONN_OPEN;
            GPIO_SetOutVal(LED_PORT, RED_LED_PIN, LED_ON);
            BLEconnected = 1;
            //MG added
            GPIO_SetOutVal(LED_PORT, GREEN_LED_PIN, LED_OFF);
            break;
         
        case DM_CONN_CLOSE_IND:
            fitClose(pMsg);
            uiEvent = APP_UI_CONN_CLOSE;
            GPIO_SetOutVal(LED_PORT, RED_LED_PIN, LED_OFF);
            BLEconnected = 0;
            break;
       
        case DM_SEC_PAIR_CMPL_IND:
            uiEvent = APP_UI_SEC_PAIR_CMPL;
            break;
     
        case DM_SEC_PAIR_FAIL_IND:
            uiEvent = APP_UI_SEC_PAIR_FAIL;
            break;
     
        case DM_SEC_ENCRYPT_IND:
            uiEvent = APP_UI_SEC_ENCRYPT;
            break;
       
        case DM_SEC_ENCRYPT_FAIL_IND:
            uiEvent = APP_UI_SEC_ENCRYPT_FAIL;
            break;

        case DM_SEC_AUTH_REQ_IND:
            AppHandlePasskey(&pMsg->dm.authReq);
            break;

        default:
            break;
    }
  
    if (uiEvent != APP_UI_NONE)
    {
        AppUiAction(uiEvent);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     FitHandlerInit
 *        
 *  \brief  Application handler init function called during system initialization.
 *
 *  \param  handlerID  WSF handler ID.
 *
 *  \return None.
 */
/*************************************************************************************************/
void FitHandlerInit(wsfHandlerId_t handlerId)
{
    APP_TRACE_INFO0("FitHandlerInit");
  
    /* store handler ID */
    fitHandlerId = handlerId;
  
    /* Set configuration pointers */
    pAppSlaveCfg = (appSlaveCfg_t *) &fitSlaveCfg;
    pAppSecCfg = (appSecCfg_t *) &fitSecCfg;
    pAppUpdateCfg = (appUpdateCfg_t *) &fitUpdateCfg;
  
    /* Initialize application framework */
    AppSlaveInit();

    /* initialize battery service server */
    BasInit(handlerId, (basCfg_t *) &fitBasCfg);
 
    /* initialize heart rate profile sensor */
    HrpsInit(handlerId, (hrpsCfg_t *) &fitHrpsCfg);
    HrpsSetFlags(CH_HRM_FLAGS_VALUE_8BIT | CH_HRM_FLAGS_ENERGY_EXP);
}

/*************************************************************************************************/
/*!
 *  \fn     FitHandler
 *        
 *  \brief  WSF event handler for application.
 *
 *  \param  event   WSF event mask.
 *  \param  pMsg    WSF message.
 *
 *  \return None.
 */
/*************************************************************************************************/
void FitHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{ 
    if (pMsg != NULL)
    {
        APP_TRACE_INFO1("Fit got evt %d", pMsg->event);
  
        if (pMsg->event >= DM_CBACK_START && pMsg->event <= DM_CBACK_END)
        {
            /* process advertising and connection-related messages */
            AppSlaveProcDmMsg((dmEvt_t *) pMsg);
      
            /* process security-related messages */
            AppSlaveSecProcDmMsg((dmEvt_t *) pMsg);
        }
        
        /* perform profile and user interface-related operations */
        fitProcMsg((fitMsg_t *) pMsg);    
    }
}

/*************************************************************************************************/
/*!
 *  \fn     FitStart
 *        
 *  \brief  Start the application.
 *
 *  \return None.
 */
/*************************************************************************************************/
void FitStart(void)
{  
    /* Register for stack callbacks */
    DmRegister(fitDmCback);
    DmConnRegister(DM_CLIENT_ID_APP, fitDmCback);
    AttRegister(fitAttCback);
    AttConnRegister(AppServerConnCback);
    AttsCccRegister(FIT_NUM_CCC_IDX, (attsCccSet_t *) fitCccSet, fitCccCback);
  
    /* Register for app framework callbacks */
    AppUiBtnRegister(fitBtnCback);
  
    /* Initialize attribute server database */
    SvcCoreAddGroup();
    SvcHrsCbackRegister(NULL, HrpsWriteCback);
    SvcHrsAddGroup();
    SvcDisAddGroup();
    SvcBattCbackRegister(BasReadCback, NULL);
    SvcBattAddGroup();

    /* Reset the device */
    DmDevReset();  
}
