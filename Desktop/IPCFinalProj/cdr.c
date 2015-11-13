/*******************************************************************************************************
Author:				Arkadi Zavurov
Description:		---  CDR Struct ADT implementation file ---
*******************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ADTErr.h"
#include "logger_pub.h"
#include "logger.h"
#include "cdr.h"

#define STR_ERROR_SIZE 100
#define SIZE_OF_TEXT 32

struct CDR
{
	char 			m_imsi[SIZE_OF_TEXT];
	char 			m_msisdn[SIZE_OF_TEXT];
	char 			m_imei[SIZE_OF_TEXT];
	char 			m_operatorCode[SIZE_OF_TEXT];
	e_callType 		m_callType;
	unsigned int 	m_callDuration;
	double			m_downloaded;
	double 			m_uploaded;
	char			m_partyMSISDN[SIZE_OF_TEXT];
	char 			m_partyOperator[SIZE_OF_TEXT];
};

CDR* CDRCreate(ADTErr* _error)
{
	char strErr[STR_ERROR_SIZE] = "";
	CDR* cdr = calloc(1, sizeof(CDR));
	if (NULL == cdr)
	{
		if(NULL != _error)
		{
			*_error = ERR_ALLOCATION_FAILED;
		}
		GetError(strErr, ERR_ALLOCATION_FAILED);
		LOG_ERROR_PRINT("%s", strErr);

		return NULL;
	}

	cdr->m_callType = LAST;
	
	if (NULL != _error)
	{
		*_error = ERR_OK;
	}
	LOG_DEBUG_PRINT("%s", "CDR was created successfully");

	return cdr;
}

void CDRDestroy(CDR* _cdr)
{
	if (NULL == _cdr)
	{
		LOG_WARN_PRINT("%s", "Attempted to destroy uninitialized CDR");
		return; 
	}

	free(_cdr);
	LOG_DEBUG_PRINT("%s", "CDR was destroyed successfully");
}

ADTErr CDRInsertIMSI(CDR* _cdr, const char* _imsi)
{
	char strErr[STR_ERROR_SIZE] = "";
		
	if (NULL == _cdr || NULL == _imsi)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_cdr->m_imsi, _imsi); 

	LOG_DEBUG_PRINT("IMSI field: %s was entered to CDR struct successfully", _cdr->m_imsi);
	return ERR_OK;
}

ADTErr CDRInsertMSISDN(CDR* _cdr, const char* _msisdn)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _msisdn)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_cdr->m_msisdn, _msisdn); 

	LOG_DEBUG_PRINT("MSISDN field: %s was entered to CDR struct successfully", _cdr->m_msisdn);
	return ERR_OK;
}

ADTErr CDRInsertIMEI(CDR* _cdr, const char* _imei)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _imei)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_cdr->m_imei, _imei); 

	LOG_DEBUG_PRINT("IMEI field: %s was entered to CDR struct successfully", _cdr->m_imei);
	return ERR_OK;
}

ADTErr CDRInsertOpCode(CDR* _cdr, const char* _operatorCode)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _operatorCode)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_cdr->m_operatorCode, _operatorCode); 

	LOG_DEBUG_PRINT("operatorCode field: %s was entered to CDR struct successfully", _cdr->m_operatorCode);
	return ERR_OK;
}

ADTErr CDRInsertCallType(CDR* _cdr, const e_callType _callType)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || LAST == _callType)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	_cdr->m_callType = _callType;
	
	LOG_DEBUG_PRINT("callType field: %d was entered to CDR struct successfully", _cdr->m_callType);
	return ERR_OK;
}

ADTErr CDRInsertCallDuration(CDR* _cdr, const unsigned int _callDuration)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	_cdr->m_callDuration = _callDuration; 

	LOG_DEBUG_PRINT("callDuration field: %u was entered to CDR struct successfully", _cdr->m_callDuration);	
	return ERR_OK;
}

ADTErr CDRInsertDownloadedMB(CDR* _cdr, const double _downloaded)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	_cdr->m_downloaded = _downloaded; 

	LOG_DEBUG_PRINT("downloaded field: %lf was entered to CDR struct successfully", _cdr->m_downloaded);		
	return ERR_OK;
}

ADTErr CDRInsertUploadedMB(CDR* _cdr, const double _uploaded)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	_cdr->m_uploaded = _uploaded; 

	LOG_DEBUG_PRINT("uploaded field: %lf was entered to CDR struct successfully", _cdr->m_uploaded);			
	return ERR_OK;
}

ADTErr CDRInsertPartyMSISDN(CDR* _cdr, const char* _partyMSISDN)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _partyMSISDN)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_cdr->m_partyMSISDN, _partyMSISDN); 

	LOG_DEBUG_PRINT("partyMSISDN field: %s was entered to CDR struct successfully", _cdr->m_partyMSISDN);			
	return ERR_OK;
}

ADTErr CDRInsertPartyOperator(CDR* _cdr, const char* _partyOperator)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _partyOperator)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_cdr->m_partyOperator, _partyOperator); 

	LOG_DEBUG_PRINT("partyOperator field: %s was entered to CDR struct successfully", _cdr->m_partyOperator);			
	return ERR_OK;
}

/******** Get functions ********/
ADTErr CDRGetIMSI(const CDR* _cdr, char* _imsi)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _imsi)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_imsi, _cdr->m_imsi); 

	LOG_DEBUG_PRINT("Got IMSI field: %s from CDR struct successfully", _imsi);			
	return ERR_OK;
}

ADTErr CDRGetMSISDN(const CDR* _cdr, char* _msisdn)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _msisdn)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_msisdn, _cdr->m_msisdn); 

	LOG_DEBUG_PRINT("Got MSISDN field: %s from CDR struct successfully", _msisdn);			
	return ERR_OK;
}


ADTErr CDRGetIMEI(const CDR* _cdr, char* _imei)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _imei)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_imei, _cdr->m_imei); 

	LOG_DEBUG_PRINT("Got IMEI field: %s from CDR struct successfully", _imei);			
	return ERR_OK;
}

ADTErr CDRGetOpCode(const CDR* _cdr, char* _operatorCode)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _operatorCode)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_operatorCode, _cdr->m_operatorCode); 

	LOG_DEBUG_PRINT("Got operatorCode field: %s from CDR struct successfully", _operatorCode);			
	return ERR_OK;
}

ADTErr CDRGetCallType(const CDR* _cdr, e_callType* _callType)
{
	char strErr[STR_ERROR_SIZE] = "";		

	if (NULL == _cdr || NULL == _callType)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	*_callType = _cdr->m_callType; 

	LOG_DEBUG_PRINT("Got callType field: %d from CDR struct successfully", *_callType);			
	return ERR_OK;
}

ADTErr CDRGetCallDuration(const CDR* _cdr, unsigned int* _callDuration)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _callDuration)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	*_callDuration = _cdr->m_callDuration; 	

	LOG_DEBUG_PRINT("Got callDuration field: %u from CDR struct successfully", *_callDuration);			
	return ERR_OK;
}

ADTErr CDRGetDownloadedMB(const CDR* _cdr, double* _downloaded)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _downloaded)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	*_downloaded = _cdr->m_downloaded; 	

	LOG_DEBUG_PRINT("Got downloaded field: %lf from CDR struct successfully", *_downloaded);			
	return ERR_OK;
}

ADTErr CDRGetUploadedMB(const CDR* _cdr, double* _uploaded)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _uploaded)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	*_uploaded = _cdr->m_uploaded; 	

	LOG_DEBUG_PRINT("Got uploaded field: %lf from CDR struct successfully", *_uploaded);			
	return ERR_OK;
}

ADTErr CDRGetPartyMSISDN(const CDR* _cdr, char* _partyMSISDN)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _partyMSISDN)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_partyMSISDN, _cdr->m_partyMSISDN); 

	LOG_DEBUG_PRINT("Got partyMSISDN field: %s from CDR struct successfully", _partyMSISDN);			
	return ERR_OK;	
}

ADTErr CDRGetPartyOperator(const CDR* _cdr, char* _partyOperator)
{
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _partyOperator)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	strcpy(_partyOperator, _cdr->m_partyOperator); 

	LOG_DEBUG_PRINT("Got partyOperator field: %s from CDR struct successfully", _partyOperator);			
	return ERR_OK;
}

#ifdef _DEBUG
void PrintCDR(CDR* _cdr)
{
	if (NULL == _cdr)
	{
		printf("Nothing to print\n");
		return;
	}

	printf("%s\n", _cdr->m_imsi);
	printf("%s\n", _cdr->m_msisdn);
	printf("%s\n", _cdr->m_imei);
	printf("%s\n", _cdr->m_operatorCode);
	printf("%d\n", _cdr->m_callType);
	printf("%u\n", _cdr->m_callDuration);
	printf("%g\n", _cdr->m_downloaded);
	printf("%g\n", _cdr->m_uploaded);
	printf("%s\n", _cdr->m_partyMSISDN);
	printf("%s\n\n", _cdr->m_partyOperator);
}
#endif /* _DEBUG */
