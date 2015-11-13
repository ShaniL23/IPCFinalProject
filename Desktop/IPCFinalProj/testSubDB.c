/**************************************************************************************************
	Author: 				Tom Vizel
	Creation date: 			10.11.15
	Last modified date: 	10.11.15
	
	Description: Unit test module for subscriber database.
**************************************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#include "ADTErr.h"
#include "cdr.h"
#include "Subscriber.h"
#include "SubscriberDB.h"

#define PRINT_STATEMENT(_statement) PrintStatement(_statement, __FUNCTION__)

static void PrintStatement(int _statement, const char* _funcName)
{
	printf("%s ", _funcName);
	printf(_statement ? "PASS!\n" : "FAIL!\n");
}

CDR* CDR1Init(void)
{
	CDR* cdr1 = NULL;
	
	cdr1 = CDRCreate(NULL);
	CDRInsertIMSI(cdr1, "111111111");
	CDRInsertOpCode(cdr1, "Cellcom");
	CDRInsertCallType(cdr1, MOC);
	CDRInsertCallDuration(cdr1, 300);
	
	return cdr1;
}

CDR* CDR2Init(void)
{
	CDR* cdr2 = NULL;
	
	cdr2 = CDRCreate(NULL);
	CDRInsertIMSI(cdr2, "111111111");
	CDRInsertOpCode(cdr2, "Cellcom");
	CDRInsertCallType(cdr2, SMS_MO);
	
	return cdr2;
}

CDR* CDR3Init(void)
{
	CDR* cdr3 = NULL;
	
	cdr3 = CDRCreate(NULL);
	CDRInsertIMSI(cdr3, "111111112");
	CDRInsertOpCode(cdr3, "Orange");
	CDRInsertCallType(cdr3, GPRS);
	CDRInsertDownloadedMB(cdr3, 1.3);
	CDRInsertUploadedMB(cdr3, 0.42);
	
	return cdr3;
}

static void CreateOK(void)
{
	ADTErr err;
	SubscriberDB* sdb = SubscriberDBCreate(&err);
	
	PRINT_STATEMENT( (NULL != sdb) && (ERR_OK == err) );
	SubscriberDBDestroy(sdb);
}

static void InsertNULL(void)
{
	SubscriberDB* sdb = SubscriberDBCreate(NULL);
	
	PRINT_STATEMENT( ERR_ILLEGAL_INPUT == SubscriberDBInsert(sdb, NULL) );
	SubscriberDBDestroy(sdb);
}

static void InsertOK(void)
{
	CDR* cdr1 = CDR1Init();
	Subscriber* sub1 = SubscriberCreate(cdr1, NULL);
	SubscriberDB* sdb = SubscriberDBCreate(NULL);
	
	PRINT_STATEMENT( ERR_OK == SubscriberDBInsert(sdb, sub1) );
	/*SubscriberDBPrint(sdb);*/
	SubscriberDBDestroy(sdb);
	CDRDestroy(cdr1);
}

static void InsertAlreadyExists(void)
{
	CDR* cdr1 = CDR1Init();
	CDR* cdr2 = CDR2Init();
	Subscriber* sub1 = SubscriberCreate(cdr1, NULL);
	Subscriber* sub2 = SubscriberCreate(cdr2, NULL);
	SubscriberDB* sdb = SubscriberDBCreate(NULL);
	
	SubscriberDBInsert(sdb, sub1);
	PRINT_STATEMENT( ERR_ALREADY_EXISTS == SubscriberDBInsert(sdb, sub2) );
	/*SubscriberDBPrint(sdb);*/
	SubscriberDBDestroy(sdb);
	/*SubscriberDestroy(sub1);
	SubscriberDestroy(sub2);*/
	CDRDestroy(cdr1);
	CDRDestroy(cdr2);
}

static void InsertDifferent(void)
{
	CDR* cdr1 = CDR1Init();
	CDR* cdr3 = CDR3Init();
	Subscriber* sub1 = SubscriberCreate(cdr1, NULL);
	Subscriber* sub3 = SubscriberCreate(cdr3, NULL);
	SubscriberDB* sdb = SubscriberDBCreate(NULL);
	
	SubscriberDBInsert(sdb, sub1);
	PRINT_STATEMENT( ERR_OK == SubscriberDBInsert(sdb, sub3) );
	/*SubscriberDBPrint(sdb);*/
	SubscriberDBDestroy(sdb);
	/*SubscriberDestroy(sub1);
	SubscriberDestroy(sub3);*/
	CDRDestroy(cdr1);
	CDRDestroy(cdr3);
}

static void GetOK(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	Subscriber* sub1 = SubscriberCreate(cdr1, NULL);
	SubscriberDB* sdb = SubscriberDBCreate(NULL);
	Subscriber* get = NULL;
	
	SubscriberDBInsert(sdb, sub1);
	err = SubscriberDBGet(sdb, "111111111", &get);
	/*SubscriberPrint(get);*/
	PRINT_STATEMENT( (ERR_OK == err) && SubscriberIsSame(sub1, get) );
	SubscriberDBDestroy(sdb);
	CDRDestroy(cdr1);
}

static void RemoveOK(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	CDR* cdr3 = CDR3Init();
	Subscriber* sub1 = SubscriberCreate(cdr1, NULL);
	Subscriber* sub3 = SubscriberCreate(cdr3, NULL);
	SubscriberDB* sdb = SubscriberDBCreate(NULL);
	Subscriber* get = NULL;
	
	SubscriberDBInsert(sdb, sub1);
	SubscriberDBInsert(sdb, sub3);
	err= SubscriberDBRemove(sdb, "111111111", &get);
	/*SubscriberPrint(get);*/
	/*SubscriberDBPrint(sdb);*/
	PRINT_STATEMENT( (ERR_OK == err) && SubscriberIsSame(sub1, get) );
	SubscriberDBDestroy(sdb);
	CDRDestroy(cdr1);
	CDRDestroy(cdr3);
}

static void PrintToFileOK(void)
{
	CDR* cdr1 = CDR1Init();
	CDR* cdr3 = CDR3Init();
	Subscriber* sub1 = SubscriberCreate(cdr1, NULL);
	Subscriber* sub3 = SubscriberCreate(cdr3, NULL);
	SubscriberDB* sdb = SubscriberDBCreate(NULL);
	
	SubscriberDBInsert(sdb, sub1);
	SubscriberDBInsert(sdb, sub3);
	PRINT_STATEMENT( ERR_OK == SubscriberDBPrintToFile(sdb, "Test.txt") );
	SubscriberDBDestroy(sdb);
	CDRDestroy(cdr1);
	CDRDestroy(cdr3);
}

static void GetNotInitialized(void)
{
	PRINT_STATEMENT( ERR_NOT_INITIALIZED == SubscriberDBGet(NULL, NULL, NULL) );
}

static void GetIllegalInput(void)
{
	ADTErr err1;
	ADTErr err2;
	char key[10] = "111";
	Subscriber* sub = NULL;
	SubscriberDB* sdb = SubscriberDBCreate(NULL);
	
	err1 = SubscriberDBGet(sdb, key, NULL);
	err2 = SubscriberDBGet(sdb, NULL, &sub);
	PRINT_STATEMENT( (ERR_ILLEGAL_INPUT == err1) && (ERR_ILLEGAL_INPUT == err2) );
	SubscriberDBDestroy(sdb);
}

static void RemoveNotInitialized(void)
{
	PRINT_STATEMENT( ERR_NOT_INITIALIZED == SubscriberDBRemove(NULL, NULL, NULL) );
}

static void RemoveIllegalInput(void)
{
	ADTErr err1;
	ADTErr err2;
	char key[10] = "111";
	Subscriber* sub = NULL;
	SubscriberDB* sdb = SubscriberDBCreate(NULL);
	
	err1 = SubscriberDBRemove(sdb, key, NULL);
	err2 = SubscriberDBRemove(sdb, NULL, &sub);
	PRINT_STATEMENT( (ERR_ILLEGAL_INPUT == err1) && (ERR_ILLEGAL_INPUT == err2) );
	SubscriberDBDestroy(sdb);
}

static void GetNotFound(void)
{
	CDR* cdr1 = CDR1Init();
	Subscriber* sub1 = SubscriberCreate(cdr1, NULL);
	SubscriberDB* sdb = SubscriberDBCreate(NULL);
	Subscriber* get = NULL;
	
	SubscriberDBInsert(sdb, sub1);
	PRINT_STATEMENT( ERR_NOT_FOUND == SubscriberDBGet(sdb, "111111112", &get) );
	SubscriberDBDestroy(sdb);
	CDRDestroy(cdr1);
}

static void RemoveNotFound(void)
{
	CDR* cdr1 = CDR1Init();
	Subscriber* sub1 = SubscriberCreate(cdr1, NULL);
	SubscriberDB* sdb = SubscriberDBCreate(NULL);
	Subscriber* get = NULL;
	
	SubscriberDBInsert(sdb, sub1);
	PRINT_STATEMENT( ERR_NOT_FOUND == SubscriberDBRemove(sdb, "111111112", &get) );
	SubscriberDBDestroy(sdb);
	CDRDestroy(cdr1);
}

int main()
{
	CreateOK();
	
	InsertNULL();
	InsertOK();
	InsertAlreadyExists();
	InsertDifferent();
	
	GetNotInitialized();
	GetIllegalInput();
	GetOK();
	GetNotFound();
	
	RemoveNotInitialized();
	RemoveIllegalInput();
	RemoveOK();
	RemoveNotFound();
	
	PrintToFileOK();
	
	return 0;
}
