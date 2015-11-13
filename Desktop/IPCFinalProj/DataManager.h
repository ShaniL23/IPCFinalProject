/*************************************************************************
Author: Shani Levinkind
Creation Date: 9.11.15
Last edit: 12.11.15
Description: Header file for Database Manager functions declarations
**************************************************************************/

#ifndef __DATAMNGR_H__
#define __DATAMNGR_H__

typedef struct DBManagerParams DBManagerParams;

DBManagerParams* InitDBManager (SafeQueue* _safeQ, ADTErr* _error);
/*Wait until the DBManager finishes*/
ADTErr EndDBManager 	(DBManagerParams* _params);
ADTErr DestroyDBManager (DBManagerParams* _params);

ADTErr GetSubscriberDB 	(const DBManagerParams* _params, SubscriberDB** _subDB);
ADTErr GetOperatorDB 	(const DBManagerParams* _params, OperatorDB** _oprDB);
ADTErr GetDBMutex 		(const DBManagerParams* _params, pthread_mutex_t* _DBMutex);

#endif /*__DATAMNGR_H__*/
