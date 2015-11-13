/**************************************************************************************************
	Author: 				Tom Vizel
	Creation date: 			10.11.15
	Last modified date: 	10.11.15
	
	Description: Unit test module for subscriber.
**************************************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "ADTErr.h"
#include "cdr.h"
#include "Subscriber.h"

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

static void CreateNULL(void)
{
	ADTErr err;
	Subscriber* sub = SubscriberCreate(NULL, &err);
	PRINT_STATEMENT( (ERR_NOT_INITIALIZED == err) && (NULL == sub) );
}

static void CreateOK(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	Subscriber* subscriber = NULL;
	
	subscriber = SubscriberCreate(cdr1, &err);
	/*SubscriberPrint(subscriber);*/
	PRINT_STATEMENT( (NULL != subscriber) && (ERR_OK == err) );
	SubscriberDestroy(subscriber);
}

static void UpdateNULL(void)
{
	PRINT_STATEMENT( ERR_NOT_INITIALIZED == SubscriberUpdate(NULL, NULL) );	
}

static void UpdateOK(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	Subscriber* sub1 = NULL;
	CDR* cdr2 = CDR2Init();
	Subscriber* sub2 = NULL;
	
	sub1 = SubscriberCreate(cdr1, NULL);
	sub2 = SubscriberCreate(cdr2, NULL);
	err = SubscriberUpdate(sub1, sub2);
	/*SubscriberPrint(sub1);*/
	PRINT_STATEMENT( ERR_OK == err );
	SubscriberDestroy(sub1);
	SubscriberDestroy(sub2);
}

static void UpdateIllegal(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	Subscriber* sub1 = NULL;
	CDR* cdr3 = CDR3Init();
	Subscriber* sub3 = NULL;
	
	sub1 = SubscriberCreate(cdr1, NULL);
	sub3 = SubscriberCreate(cdr3, NULL);
	err = SubscriberUpdate(sub1, sub3);
	PRINT_STATEMENT( ERR_ILLEGAL_INPUT == err );
	SubscriberDestroy(sub1);
	SubscriberDestroy(sub3);
}

static void IsSameDifferent(void)
{
	CDR* cdr1 = CDR1Init();
	Subscriber* sub1 = NULL;
	CDR* cdr3 = CDR3Init();
	Subscriber* sub3 = NULL;
	
	sub1 = SubscriberCreate(cdr1, NULL);
	sub3 = SubscriberCreate(cdr3, NULL);
	PRINT_STATEMENT( ! SubscriberIsSame(sub1, sub3) );
	SubscriberDestroy(sub1);
	SubscriberDestroy(sub3);
}

static void IsSameSame(void)
{
	CDR* cdr1 = CDR1Init();
	Subscriber* sub1 = NULL;
	CDR* cdr2 = CDR2Init();
	Subscriber* sub2 = NULL;
	
	sub1 = SubscriberCreate(cdr1, NULL);
	sub2 = SubscriberCreate(cdr2, NULL);
	PRINT_STATEMENT( SubscriberIsSame(sub1, sub2) );
	SubscriberDestroy(sub1);
	SubscriberDestroy(sub2);
}

static void GetIMSINull(void)
{
	PRINT_STATEMENT( ERR_NOT_INITIALIZED == SubscriberGetIMSI(NULL, NULL) );
}

static void GetIMSIOK(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	Subscriber* sub1 = NULL;
	char buf[20];
	
	sub1 = SubscriberCreate(cdr1, NULL);
	err = SubscriberGetIMSI(sub1, buf);
	PRINT_STATEMENT( (ERR_OK == err) && ( ! strcmp(buf, "111111111") ) );
	SubscriberDestroy(sub1);
}

int main()
{
	CreateNULL();
	CreateOK();
	
	UpdateNULL();
	UpdateOK();
	UpdateIllegal();
	
	IsSameDifferent();
	IsSameSame();
	
	GetIMSINull();
	GetIMSIOK();
	
	/* PrintToFile tested in testSubDB */
	
	return 0;
}
