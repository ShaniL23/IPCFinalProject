/**************************************************************************************************
	Author: 				Shani Levinkind
	Creation date: 			9.11.15
	Last modified date: 	12.11.15
	
	Description: Implementation module for Database Manager functions.
**************************************************************************************************/

#include <pthread.h>
#include <stdio.h> /*for logger*/
#include <stdlib.h> /*for malloc*/
#include <string.h> /*for strcmp*/
#include <fcntl.h> /*for open*/
#include <unistd.h> /*for close*/
#include <sys/stat.h> /*for flags*/

#include "ADTErr.h"
#include "logger.h"
#include "logger_pub.h"
#include "GData.h"
#include "safeQueue.h"
#include "cdr.h"
#include "Operator.h"
#include "Subscriber.h"
#include "OperatorDB.h"
#include "SubscriberDB.h"
#include "DataManager.h"

#define Q_IS_EMPTY_KEY "END"
/*if m_magic == MAGIC then struct is valid*/
#define MAGIC (void*)0xDEADBEEF
#define INVALID_MNGR_PRMS(params) !params || MAGIC != params->m_magic
#define FAILED_DATA_LOG_NAME "FailedData.txt"
#define ERR_STR_LENGTH 100
#define KEY_STR_LENGTH 30

struct DBManagerParams
{
	SubscriberDB* m_subDB;
	OperatorDB* m_oprDB;
	pthread_mutex_t m_DBMutex;
	SafeQueue* m_safeQ;
	void* m_magic;
}; 

static pthread_t s_feederThread;

static ADTErr PrepData (CDR* _cdr, Subscriber** _newSubscriber, Operator** _newOperator);
static ADTErr InsertSub2DB (SubscriberDB* _subDB, Subscriber* _newSubscriber, char* _key);
static ADTErr InsertOpr2DB (OperatorDB* _oprDB, Operator* _newOperator, char* _key);
static void* DBFeeder(void* _params);
/*if only one is needed send the other with NULL*/
static void LogFailedData (Subscriber* _sub, Operator* _opr);

DBManagerParams* InitDBManager (SafeQueue* _safeQ, ADTErr* _error)
{	
	DBManagerParams* params;
	ADTErr errorCheck;
	LOG_DEBUG_PRINT("%s\n", "InitDBManager has started");
	LOG_DEBUG_PRINT("%s %p %s %p\n", "_safeQ:",(void*)_safeQ,"_error:",(void*)_error);
	if (!_safeQ)
	{
		if (_error)
		{
			*_error = ERR_NOT_INITIALIZED;
		}
		LOG_ERROR_PRINT ("%s\n", "safeQ not initialized");
		return NULL;
	}
	LOG_DEBUG_PRINT("%s\n", "Trying to allocate memory for params");
	/*FREE IN DESTROY FUNC*/
	params = malloc(sizeof(DBManagerParams));
	if (!params)
	{
		if (_error)
		{
			*_error = ERR_ALLOCATION_FAILED;
		}
		LOG_ERROR_PRINT ("%s\n", "Params allocation failed");
		return NULL;
	}
	params->m_safeQ = _safeQ;
	params->m_magic = MAGIC;
	/*create SBI data base*/
	LOG_DEBUG_PRINT("%s\n", "Trying to create SBI database");
	params->m_subDB = SubscriberDBCreate (&errorCheck);
	if (ERR_OK != errorCheck || !params->m_subDB)
	{
		if (_error)
		{
			*_error = ERR_INTERNAL_DB_FAIL;
		}
		LOG_ERROR_PRINT ("%s\n", "Creating SBI database failed");
		return NULL;
	}
	LOG_DEBUG_PRINT("%s\n", "SBI database creation was successful");
	/*create OBI data base*/
	LOG_DEBUG_PRINT("%s\n", "Trying to create OBI database");
	params->m_oprDB = OperatorDBCreate (&errorCheck);
	if (ERR_OK != errorCheck || !params->m_oprDB)
	{
		SubscriberDBDestroy (params->m_subDB);
		if (_error)
		{
			*_error = ERR_INTERNAL_DB_FAIL;
		}
		LOG_ERROR_PRINT ("%s\n", "Creating OBI database failed");
		return NULL;
	}
	LOG_DEBUG_PRINT("%s\n", "OBI database creation was successful");
	/*create DBMutex*/
	LOG_DEBUG_PRINT("%s\n", "Trying to create database mutex");
	if (0 != pthread_mutex_init(&params->m_DBMutex, NULL))
	{
		SubscriberDBDestroy (params->m_subDB);
		OperatorDBDestroy (params->m_oprDB);
		if (_error)
		{		
			*_error = ERR_INTERNAL_DB_FAIL;
		}
		LOG_ERROR_PRINT("%s\n", "DBMutex creation failed");
		return NULL;
	}
	LOG_DEBUG_PRINT("%s\n", "Database mutex creation was successful");
	LOG_DEBUG_PRINT("%s\n", "Trying to create feederThread");
	if (0 != pthread_create(&s_feederThread, NULL, DBFeeder, params))
	{
		SubscriberDBDestroy (params->m_subDB);
		OperatorDBDestroy (params->m_oprDB);
		pthread_mutex_destroy (&params->m_DBMutex);
		if (_error)
		{		
			*_error = ERR_INTERNAL_DB_FAIL;
		}
		LOG_ERROR_PRINT("%s\n", "feederThread creation failed");
		return NULL;
	}
	LOG_DEBUG_PRINT("%s\n", "feederThread creation was successful");
	if (_error)
	{
		*_error = ERR_OK;
	}
	LOG_DEBUG_PRINT("%s\n", "InitDBManager finished succesfully");	
	return params;
}	

static void* DBFeeder(void* _params)
{
	ADTErr errorCheck;
	Subscriber* newSubscriber = NULL;
	Operator* newOperator = NULL;
	char errorStr[ERR_STR_LENGTH];
	char imsi[KEY_STR_LENGTH];
	char oprName[KEY_STR_LENGTH];
	DBManagerParams* params = _params;
	CDR* cdr;
	LOG_DEBUG_PRINT("%s\n", "DBFeeder thread has started");
	while (1)
	{
		/*get CDR*/
		LOG_DEBUG_PRINT("%s\n", "Trying to get CDR from Q");
		errorCheck = SafeQueuePop (params->m_safeQ, (void**)&cdr);
		if (ERR_OK != errorCheck)
		{
			GetError (errorStr, errorCheck);
			LOG_ERROR_PRINT("%s\n", errorStr);
			pthread_exit (NULL);
		}
		LOG_DEBUG_PRINT("%s\n", "Getting CDR from Q was succesful");
		LOG_DEBUG_PRINT("%s\n", "Get cdr IMSI");
		errorCheck = CDRGetIMSI(cdr, imsi);
		if (ERR_OK != CDRGetIMSI(cdr, imsi))
		{
			CDRDestroy (cdr);
			GetError (errorStr, errorCheck);
			LOG_WARN_PRINT("%s\n", errorStr);
			continue;
		}	
		/*check if IMSI is an ending signal*/
		LOG_DEBUG_PRINT("%s\n", "Checking if IMSI is an end signal");
		if (0 == strcmp (imsi, Q_IS_EMPTY_KEY))
		{
			LOG_DEBUG_PRINT("%s\n", "End key recieved");
			CDRDestroy (cdr);
			break;
		}
		LOG_DEBUG_PRINT("%s %s\n", "Get IMSI was successful. IMSI:", imsi);
		LOG_DEBUG_PRINT("%s\n", "Starting to prep data");
		/*if data is invalid then skip to next in Q*/
		errorCheck = PrepData (cdr, &newSubscriber, &newOperator);
		if (ERR_OK != errorCheck)
		{
			GetError (errorStr, errorCheck);
			LOG_WARN_PRINT("%s\n", errorStr);
			continue;
		}
		LOG_DEBUG_PRINT("%s\n", "Prep data was successful");
		LOG_DEBUG_PRINT("%s\n", "Trying to get operator name");
		errorCheck = OperatorGetName(newOperator, oprName);
		if (ERR_OK != errorCheck)
		{
			LogFailedData (newSubscriber, newOperator);
			SubscriberDestroy (newSubscriber);
			OperatorDestroy (newOperator);
			GetError (errorStr, errorCheck);
			LOG_WARN_PRINT("%s\n", errorStr);
			continue;
		}
		LOG_DEBUG_PRINT("%s\n", "Locking mutex");
		if (0 != pthread_mutex_lock(&params->m_DBMutex))
		{
			LogFailedData (newSubscriber, newOperator);
			SubscriberDestroy (newSubscriber);
			OperatorDestroy (newOperator);			
			LOG_ERROR_PRINT("%s\n", "Locking mutex failed");
			pthread_exit (NULL);
		}
		LOG_DEBUG_PRINT("%s\n", "Mutex locked");
		LOG_DEBUG_PRINT("%s\n", "Trying to insert to subscriber DB");
		errorCheck = InsertSub2DB (params->m_subDB, newSubscriber, imsi);
		if (ERR_OK != errorCheck)	
		{
			LogFailedData (newSubscriber, newOperator);
			SubscriberDestroy (newSubscriber);
			OperatorDestroy (newOperator);
			if (0 != pthread_mutex_unlock (&params->m_DBMutex))
			{
				pthread_exit (NULL);
			}		
			GetError (errorStr, errorCheck);
			LOG_WARN_PRINT("%s\n", errorStr);
			continue;
		}
		LOG_DEBUG_PRINT("%s\n", "Insert to subscriber DB was succesfull");
		LOG_DEBUG_PRINT("%s\n", "Trying to insert to operator DB");
		errorCheck = InsertOpr2DB (params->m_oprDB, newOperator, oprName);
		if (ERR_OK != errorCheck)
		{
			LogFailedData (NULL, newOperator);
			OperatorDestroy (newOperator);
			GetError (errorStr, errorCheck);
			LOG_WARN_PRINT("%s\n", errorStr);	
			if (0 != pthread_mutex_unlock (&params->m_DBMutex))
			{
				LOG_ERROR_PRINT("%s\n", "Unlocking mutex failed");
				pthread_exit (NULL);
			}	
			continue;
		}
		LOG_DEBUG_PRINT("%s\n", "Insert to subscriber DB was succesfull");
		LOG_DEBUG_PRINT("%s\n", "Unlocking mutex");
		if (0 != pthread_mutex_unlock (&params->m_DBMutex))
		{
			LOG_ERROR_PRINT("%s\n", "Unlocking mutex failed");
			pthread_exit (NULL);
		}
		LOG_DEBUG_PRINT("%s\n", "Mutex unlocked");
	}
	LOG_DEBUG_PRINT("%s\n", "DBFeeder finished succesfully");	
	pthread_exit (NULL);
}

static ADTErr PrepData (CDR* _cdr, Subscriber** _newSubscriber, Operator** _newOperator)
{
	ADTErr errorCheck;
	char errorStr[ERR_STR_LENGTH];
	LOG_DEBUG_PRINT("%s\n", "PrepData has started");
	/*seperate CDR to subscriber and operator*/
	LOG_DEBUG_PRINT("%s\n", "Creating subscriber from cdr");
	*_newSubscriber = SubscriberCreate (_cdr, &errorCheck);
	if (ERR_OK != errorCheck || !*_newSubscriber)
	{
		CDRDestroy (_cdr);
		GetError (errorStr, errorCheck);
		LOG_ERROR_PRINT("%s\n", errorStr);	
		return ERR_DATA_PREP_FAILED;
	}
	LOG_DEBUG_PRINT("%s\n", "Subscriber created succesfully");	
	LOG_DEBUG_PRINT("%s\n", "Creating operator from cdr");	
	*_newOperator = OperatorCreate (_cdr, &errorCheck);		
	if (ERR_OK != errorCheck || !*_newOperator)
	{
		LogFailedData (*_newSubscriber, NULL);
		SubscriberDestroy (*_newSubscriber);
		CDRDestroy (_cdr);
		GetError (errorStr, errorCheck);
		LOG_ERROR_PRINT("%s\n", errorStr);
		return ERR_DATA_PREP_FAILED;
	}
	LOG_DEBUG_PRINT("%s\n", "Operator created succesfully");	
	CDRDestroy (_cdr);
	LOG_DEBUG_PRINT("%s\n", "PrepData finished succesfully");	
	return ERR_OK;
}

static ADTErr InsertSub2DB (SubscriberDB* _subDB, Subscriber* _newSubscriber, char* _key)
{
	ADTErr errorCheck;
	char errorStr[ERR_STR_LENGTH];
	LOG_DEBUG_PRINT("%s %p %s %p %s %s\n", "InsertSub2DB has started. DB:",(void*)_subDB,"new sub:", (void*)_newSubscriber, "key:",_key);
	Subscriber* existSubscriber = NULL;
	/*update if exists*/
	LOG_DEBUG_PRINT("%s\n", "Looking if subscriber already exist in DB");
	if (ERR_OK == SubscriberDBGet (_subDB, _key, &existSubscriber))
	{
		LOG_DEBUG_PRINT("%s\n", "Subscriber exists");
		LOG_DEBUG_PRINT("%s\n", "Trying to update subscriber");
		errorCheck = SubscriberUpdate (existSubscriber, _newSubscriber);
		if (ERR_OK != errorCheck)
		{
			GetError (errorStr, errorCheck);
			LOG_ERROR_PRINT("%s\n", errorStr);
			return ERR_DB_UPDATE_FAILED;
		}
		SubscriberDestroy (_newSubscriber);
		LOG_DEBUG_PRINT("%s\n", "Subscriber Updated succesfully");		
	}
	else
	{	
		LOG_DEBUG_PRINT("%s\n", "Subscriber doesn't already exist in DB");
		/*if doesn't exist, needs insert*/
		errorCheck = SubscriberDBInsert(_subDB, _newSubscriber);
		if (ERR_OK != errorCheck)
		{
			GetError (errorStr, errorCheck);
			LOG_ERROR_PRINT("%s\n", errorStr);	
			return ERR_DB_UPDATE_FAILED;
		}
	}
	LOG_DEBUG_PRINT("%s\n", "Subscriber added to DB");
	return ERR_OK;
}

static ADTErr InsertOpr2DB (OperatorDB* _oprDB, Operator* _newOperator, char* _key)
{
	ADTErr errorCheck;
	char errorStr[ERR_STR_LENGTH];
	Operator* existOperator = NULL;
	LOG_DEBUG_PRINT("%s %p %s %p %s %s\n", "InsertOpr2DB has started. DB:",(void*)_oprDB,"new opr:", (void*)_newOperator, "key:",_key);
	LOG_DEBUG_PRINT("%s\n", "Looking if operator already exist in DB");
	if (ERR_OK == OperatorDBGet (_oprDB, _key, &existOperator))
	{
		LOG_DEBUG_PRINT("%s\n", "Operator exists");
		LOG_DEBUG_PRINT("%s\n", "Trying to update operator");
		errorCheck = OperatorUpdate (existOperator, _newOperator);
		if (ERR_OK != errorCheck)
		{
			GetError (errorStr, errorCheck);
			LOG_ERROR_PRINT("%s\n", errorStr);
			return ERR_DB_UPDATE_FAILED;
		}
		OperatorDestroy (_newOperator);
		LOG_DEBUG_PRINT("%s\n", "Operator Updated succesfully");		
		return ERR_OK;
	}
	LOG_DEBUG_PRINT("%s\n", "Operator doesn't already exist in DB");
	/*if get didn't succeed, needs insert*/
	errorCheck = OperatorDBInsert(_oprDB, _newOperator);
	if (ERR_OK != errorCheck)
	{
		GetError (errorStr, errorCheck);
		LOG_ERROR_PRINT("%s\n", errorStr);	
		return ERR_DB_UPDATE_FAILED;
	}
	LOG_DEBUG_PRINT("%s\n", "Operator added to DB");
	return ERR_OK;
}

/*If want to print only one- the other should be marked as NULL*/
static void LogFailedData (Subscriber* _sub, Operator* _opr)
{
	int fileDesc;
	ADTErr errorCheck;
	char errorStr[ERR_STR_LENGTH];
	LOG_DEBUG_PRINT("%s %p %s %p\n", "LogFailedData has started. sub:", (void*)_sub,"opr",(void*)_opr);
	if (!_sub && !_opr)
	{
		LOG_ERROR_PRINT("%s\n", "Both parameters are not initialized");
		return;
	}
	LOG_DEBUG_PRINT("%s\n", "Trying to open file");
	fileDesc = open (FAILED_DATA_LOG_NAME, O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if (fileDesc < 0)
	{
		LOG_ERROR_PRINT("%s %d\n", "Opening file failed. FileDesc:",fileDesc);
		return;
	}
	if (_sub)
	{
		LOG_DEBUG_PRINT("%s\n", "Printing subscriber to file");
		errorCheck = SubscriberPrintToFile (_sub, fileDesc);
		if (ERR_OK != errorCheck)
		{
			GetError (errorStr, errorCheck);
			LOG_ERROR_PRINT("%s\n", errorStr);
		}
	}
	if (_opr)
	{
		LOG_DEBUG_PRINT("%s\n", "Printing operator to file");
		OperatorPrintToFile (_opr, fileDesc);
	}
	LOG_DEBUG_PRINT("%s\n", "Trying to close file");
	if (-1 == close (fileDesc))
	{
		LOG_ERROR_PRINT("%s\n", "closing file failed");
		return;
	}
	LOG_DEBUG_PRINT("%s\n", "LogFailedData finished succesfully");	
}

ADTErr EndDBManager (DBManagerParams* _params)		
{
	LOG_DEBUG_PRINT("%s\n", "EndDBManager has started");
	if (INVALID_MNGR_PRMS(_params))
	{
		LOG_ERROR_PRINT("%s\n", "Params is not initialized");		
		return ERR_NOT_INITIALIZED;
	}
	LOG_DEBUG_PRINT("%s\n", "Joining feederThread");	
	if (0 != pthread_join(s_feederThread, NULL))
	{
		LOG_ERROR_PRINT("%s\n", "Joining feederThread failed");
		return ERR_THREAD_CANT_JOIN;
	}
	LOG_DEBUG_PRINT("%s\n", "Joining finished succesfully");	
	return ERR_OK;
}		
		
ADTErr DestroyDBManager (DBManagerParams* _params)
{
	LOG_DEBUG_PRINT("%s\n", "DestroyDBManager has started");
	if (INVALID_MNGR_PRMS(_params))
	{
		LOG_ERROR_PRINT("%s\n", "Params is not initialized");
		return ERR_NOT_INITIALIZED;
	}
	LOG_DEBUG_PRINT("%s\n", "Destroying subDB");	
	SubscriberDBDestroy (_params->m_subDB);
	LOG_DEBUG_PRINT("%s\n", "Destroying oprDB");	
	OperatorDBDestroy (_params->m_oprDB);
	LOG_DEBUG_PRINT("%s\n", "Destroying DBMutex");
	pthread_mutex_destroy (&_params->m_DBMutex);
	_params->m_magic = NULL;
	LOG_DEBUG_PRINT("%s\n", "Destroying params");
	free (_params);
	LOG_DEBUG_PRINT("%s\n", "DestroyDBManager finished succesfully");	
	return ERR_OK;
}		
		
ADTErr GetSubscriberDB (const DBManagerParams* _params, SubscriberDB** _subDB)
{
	LOG_DEBUG_PRINT("%s\n", "GetSubscriberDB has started");
	if (INVALID_MNGR_PRMS(_params) || !_subDB)
	{
		LOG_ERROR_PRINT("%s\n", "A paramater is not initialized");
		return ERR_NOT_INITIALIZED;
	}
	*_subDB = _params->m_subDB;
	LOG_DEBUG_PRINT("%s\n", "GetSubscriberDB finished succesfully");	
	return ERR_OK;
}

ADTErr GetOperatorDB (const DBManagerParams* _params, OperatorDB** _oprDB)
{
	LOG_DEBUG_PRINT("%s\n", "GetOperatorDB has started");
	if (INVALID_MNGR_PRMS(_params) || !_oprDB)
	{
		LOG_ERROR_PRINT("%s\n", "A paramater is not initialized");
		return ERR_NOT_INITIALIZED;
	}
	*_oprDB = _params->m_oprDB;
	LOG_DEBUG_PRINT("%s\n", "GetOperatorDB finished succesfully");	
	return ERR_OK;
}

ADTErr GetDBMutex (const DBManagerParams* _params, pthread_mutex_t* _DBMutex)
{
	LOG_DEBUG_PRINT("%s\n", "GetDBMutex has started");
	if (INVALID_MNGR_PRMS(_params) || !_DBMutex)
	{
		LOG_ERROR_PRINT("%s\n", "A paramater is not initialized");
		return ERR_NOT_INITIALIZED;
	}
	*_DBMutex = _params->m_DBMutex;
	LOG_DEBUG_PRINT("%s\n", "GetDBMutex finished succesfully");	
	return ERR_OK;
}

