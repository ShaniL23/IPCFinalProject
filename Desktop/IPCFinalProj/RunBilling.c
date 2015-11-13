/**************************************************************************************************************************************
Author : Erez Atar	
Creation Date : 9.11.2015
Last modified date 9.11.2015
Description : Test - CDR - Insight project 
**************************************************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ADTErr.h"
#include "logger.h"
#include "logger_pub.h"
#include "safeQueue.h"
#include "cdr.h"
#include "Subscriber.h"
#include "SubscriberDB.h"
#include "Operator.h"
#include "OperatorDB.h"
#include "DataManager.h"
#include "Billing.h"
#include "parser.h"
#include "FilesReader.h"

#define Q_SIZE 10
#define STR_ERR_SIZE 60
#define SIZE_PATH 100

int main ()
{
	SafeQueue* safeQ;
	DBManagerParams* params;
	ADTErr err;
	char strErr[STR_ERR_SIZE];
	char path[SIZE_PATH] = "./Storage/";
	LogCreate(LOG_ERROR | LOG_WARN | LOG_DEBUG,"FILE_LOGGER");
	safeQ = SafeQueueInit(Q_SIZE);
	if(NULL == safeQ)
	{
		LOG_ERROR_PRINT("%s","safeQ create - failed");
		return -1;
	}
	if(ERR_OK != (err = InitReaders (safeQ,path)))
	{
		SafeQueueDestroy(safeQ);
		GetError(strErr,err);
		LOG_ERROR_PRINT("%s",strErr);
		return -1;
	}
	params = InitDBManager (safeQ, &err); 
	if(NULL == params)
	{
		SafeQueueDestroy(safeQ);
		GetError(strErr,err);
		LOG_ERROR_PRINT("%s",strErr);
		return -1;
	}
	if(ERR_OK != (err = BillingInit (params)))
	{
		SafeQueueDestroy(safeQ);
		GetError(strErr,err);
		LOG_ERROR_PRINT("%s",strErr);
		return -1;
	}
	if(ERR_OK != (err = EndReaders(safeQ)))
	{
		SafeQueueDestroy(safeQ);
		GetError(strErr,err);
		LOG_ERROR_PRINT("%s",strErr);
		return -1;
	}
	if(ERR_OK != (err = EndDBManager(params)))
	{
		GetError(strErr,err);
		LOG_ERROR_PRINT("%s",strErr);
		return -1;
	}
	if(ERR_OK != (err = EndBilling()))
	{
		GetError(strErr,err);
		LOG_ERROR_PRINT("%s",strErr);
		return -1;
	}
	LogDestroy();
	return 0;
}
