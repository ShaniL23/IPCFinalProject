/**************************************************************************************************
	Author: 				Shani Levinkind
	Creation date: 			9.11.15
	Last modified date: 	13.11.15
	
	Description: Implementation module for Billing functions.
					SIGUSR1 - print subscriber bills.
					SIGUSR2 - print operator bills
**************************************************************************************************/

#include <pthread.h>
#include <signal.h>
#include <string.h> /*for logger*/
#include <errno.h> /*for logger*/
#include <stdio.h> /*for logger*/

#include "ADTErr.h"
#include "GData.h"
#include "logger.h"
#include "logger_pub.h"
#include "safeQueue.h"
#include "cdr.h"
#include "Operator.h"
#include "Subscriber.h"
#include "OperatorDB.h"
#include "SubscriberDB.h"
#include "DataManager.h"
#include "Billing.h"

#define SUB_DB_FILE "SubscribersInfos.txt"
#define OPR_DB_FILE "OperatorsInfos.txt"
#define ERR_STR_LENGTH 100

typedef struct
{
	pthread_mutex_t m_DBMutex;
	SubscriberDB* 	m_subDB;
	OperatorDB*		m_oprDB;
}Billing;

static pthread_t s_billThread;
static pthread_cond_t s_waitForSig = PTHREAD_COND_INITIALIZER;
static int s_flag = 0;

static void handler (int sig)
{
	if (SIGUSR1 == sig)
	{
		s_flag = 1;
	}	
	if (SIGUSR2 == sig)
	{
		s_flag = 2;
	}
	pthread_cond_signal(&s_waitForSig);
}

static ADTErr PrepParams (DBManagerParams* _mngParams, Billing* _billing)
{
	ADTErr errorCheck;
	char errorStr[ERR_STR_LENGTH];
	LOG_DEBUG_PRINT("%s\n", "Starting to prep params");
	LOG_DEBUG_PRINT("%s\n", "Getting mutex");
	errorCheck = GetDBMutex (_mngParams, &_billing->m_DBMutex);
	if (ERR_OK != errorCheck)
	{
		GetError (errorStr, errorCheck);
		LOG_ERROR_PRINT("%s\n", errorStr);
		return ERR_DATA_PREP_FAILED;
	}
	LOG_DEBUG_PRINT("%s\n", "Getting subDB");
	errorCheck = GetSubscriberDB(_mngParams, &_billing->m_subDB);
	if (ERR_OK != errorCheck)
	{
		GetError (errorStr, errorCheck);
		LOG_ERROR_PRINT("%s\n", errorStr);
		return ERR_DATA_PREP_FAILED;
	}
	LOG_DEBUG_PRINT("%s\n", "Getting oprDB");
	errorCheck = GetOperatorDB(_mngParams, &_billing->m_oprDB);
	if (ERR_OK != errorCheck)
	{
		GetError (errorStr, errorCheck);
		LOG_ERROR_PRINT("%s\n", errorStr);
		return ERR_DATA_PREP_FAILED;
	}
	LOG_DEBUG_PRINT("%s\n", "PrepParams finished succesfully");	
	return ERR_OK;
}

static void* ExportBills (void* _params)
{
	Billing billing;
	LOG_DEBUG_PRINT("%s\n", "ExportBills Thread started");
	LOG_DEBUG_PRINT("%s\n", "Trying to prep parameters");
	if (ERR_OK != PrepParams ((DBManagerParams*)_params, &billing))
	{
		LOG_ERROR_PRINT("%s\n", "PrepParams failed");
		return NULL;
	}
	LOG_DEBUG_PRINT("%s\n", "All parameters are ready");			
	while (1)
	{
		LOG_DEBUG_PRINT("%s\n", "Lock DBMutex");
		if (0 != pthread_mutex_lock(&billing.m_DBMutex))
		{
			LOG_ERROR_PRINT("%s\n", "Locking DBMutex failed");		
			pthread_exit (NULL);
		}	
		/*wait for signal*/
		LOG_DEBUG_PRINT("%s\n", "Waiting for signal");		
		if (0 != pthread_cond_wait(&s_waitForSig, &billing.m_DBMutex))
		{
			LOG_ERROR_PRINT("%s\n", "Signal wait failed");					
			LOG_DEBUG_PRINT("%s\n", "Unlocking DBMutex");
			if (0 != pthread_mutex_unlock (&billing.m_DBMutex))
			{
				LOG_ERROR_PRINT("%s\n", "Unlocking DBMutex failed");		
				pthread_exit (NULL);
			}
			pthread_exit (NULL);
		}
		LOG_DEBUG_PRINT("%s\n", "Signal recieved");
		LOG_DEBUG_PRINT("Trying to write bills to file - flag is %d\n",s_flag);		
		/*start exporting bills*/
		if (1 == s_flag)
		{
			SubscriberDBPrintToFile (billing.m_subDB, SUB_DB_FILE);
		}
		else if (2 == s_flag)
		{
			OperatorDBPrintToFile (billing.m_oprDB, OPR_DB_FILE);
		}
		LOG_DEBUG_PRINT("%s\n", "Unlocking DBMutex");
		if (0 != pthread_mutex_unlock (&billing.m_DBMutex))
		{
			LOG_ERROR_PRINT("%s\n", "Unlocking DBMutex failed");		
			pthread_exit (NULL);
		}
		LOG_DEBUG_PRINT("%s\n", "All current Bills written");
	}
	LOG_DEBUG_PRINT("%s\n", "ExportBills finished succesfully");
	pthread_exit (NULL);
}

ADTErr BillingInit (DBManagerParams* _mngParams)
{
	struct sigaction sig;
	LOG_DEBUG_PRINT("%s\n", "BillingInit function started");
	if (!_mngParams)
	{
		LOG_ERROR_PRINT("%s\n", "mngParams not initialized");
		return ERR_NOT_INITIALIZED;
	}
	LOG_DEBUG_PRINT("%s\n", "Trying to initialize default for SIGUSR1");	
	/*set default for signal SIGUSR1 and SIGUSR2*/
	sig.sa_handler = &handler;
	if (-1 == sigaction(SIGUSR1, &sig, NULL))
	{
		LOG_ERROR_PRINT("sigaction: %s\n", strerror(errno));
		return ERR_SIGACTION;
	}
	LOG_DEBUG_PRINT("%s\n", "Defaults for SIGUSR1 initialized");	
	LOG_DEBUG_PRINT("%s\n", "Trying to initialize default for SIGUSR2");
	if (-1 == sigaction(SIGUSR2, &sig, NULL))
	{
		LOG_ERROR_PRINT("sigaction: %s\n", strerror(errno));
		return ERR_SIGACTION;
	}
	LOG_DEBUG_PRINT("%s\n", "Defaults for SIGUSR2 initialized");
	/*start billing thread*/
	LOG_DEBUG_PRINT("%s\n", "Stating billing thread");
	if (0 != pthread_create(&s_billThread, NULL, ExportBills, _mngParams))
	{
		LOG_ERROR_PRINT("%s\n", "Creating billThread failed");	
		return ERR_THREAD_CANT_CREATE;
	}
	LOG_DEBUG_PRINT("%s\n", "billThread created");	
	return ERR_OK;
}

ADTErr EndBilling (void)		
{
	LOG_DEBUG_PRINT("%s\n", "Trying to join billThread");
	if (0 != pthread_join(s_billThread, NULL))
	{
		LOG_ERROR_PRINT("%s\n", "Joining thread failed");
		return ERR_THREAD_CANT_JOIN;
	}
	LOG_DEBUG_PRINT("%s\n", "BillThread is done");	
	return ERR_OK;
}	

