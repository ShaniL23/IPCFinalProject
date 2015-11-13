/*******************************************************************************************************
Author:				Arkadi Zavurov
Description:		* Get CDR string and convert to CDR struct
					* Insert CDR to queue
					* Send "END" message to queue 
					---  Parser Implementation File ---
*******************************************************************************************************/
#include <stdio.h>
#include <string.h>

#include "ADTErr.h"
#include "logger_pub.h"
#include "logger.h"
#include "GData.h"   
#include "safeQueue.h" 
#include "cdr.h"
#include "parser.h"

/* TODO: make static function instead of MACRO */
#define STR_ERROR_SIZE 100
#define RETURN_PARSING_ERROR_AND_DESTROY_CDR_IF_FAILED if (errorStatus != ERR_OK)      			  \
													   {							   			  \
															CDRDestroy(*_cdr);         			  \
															*_cdr = NULL;              			  \
									 						GetError(strErr, ERR_PARSING_FAILED); \
															LOG_ERROR_PRINT("%s", strErr);		  \
															return ERR_PARSING_FAILED; 		      \
													   }
													   
static int Convert2ChosenCallType(const char* _callTypeStr)
{
	e_callType callType; 

	if (strcmp(_callTypeStr, "MOC") == 0)
	{
		callType = MOC;
	}
	else if (strcmp(_callTypeStr, "MTC") == 0)
	{
		callType = MTC;
	}
	else if (strcmp(_callTypeStr, "SMS_MO") == 0)
	{
		callType = SMS_MO;
	}
	else if (strcmp(_callTypeStr, "SMS_MT") == 0)
	{
		callType = SMS_MT;
	}
	else if (strcmp(_callTypeStr, "GPRS") == 0)
	{
		callType = GPRS;
	}
	else
	{
		callType = LAST; /* invalid option */
		LOG_WARN_PRINT("%s", "Invalid Calltype option chosen");
	}
	
	return callType;	
}

/* Get CDR string, convert to CDR struct */
ADTErr Parse(char* _cdrString, CDR** _cdr)
{
	char* savePtr = NULL; /* for strtok_r function, which is thread safe */
	ADTErr errorStatus;
	char* cdrDataStr = NULL;
	e_callType callType;
	unsigned int callDuration;
	double downloaded;
	double uploaded;
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdrString || NULL == _cdr)
	{	
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	*_cdr = CDRCreate(&errorStatus);
	if (errorStatus != ERR_OK)
	{
		/* TODO: also this change to internal error */
		GetError(strErr, ERR_ALLOCATION_FAILED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_ALLOCATION_FAILED;
	}

	LOG_DEBUG_PRINT("%s", "CDR Successfuly created");
	/* Field: IMSI */ 
	cdrDataStr = strtok_r(_cdrString, "|", &savePtr);
	errorStatus = CDRInsertIMSI(*_cdr, cdrDataStr);
	RETURN_PARSING_ERROR_AND_DESTROY_CDR_IF_FAILED;
	LOG_DEBUG_PRINT("Field %s inserted to IMSI successfully", cdrDataStr);
	/* Field: MSISDN */
	cdrDataStr = strtok_r(NULL, "|", &savePtr);
	errorStatus = CDRInsertMSISDN(*_cdr, cdrDataStr);
	RETURN_PARSING_ERROR_AND_DESTROY_CDR_IF_FAILED;
	LOG_DEBUG_PRINT("Field %s inserted to MSISDN successfully", cdrDataStr);
	/* Field: IMEI */
	cdrDataStr = strtok_r(NULL, "|", &savePtr);
	errorStatus = CDRInsertIMEI(*_cdr, cdrDataStr);
	RETURN_PARSING_ERROR_AND_DESTROY_CDR_IF_FAILED;
	LOG_DEBUG_PRINT("Field %s inserted to IMEI successfully", cdrDataStr);
	/* Field: operator code */
	cdrDataStr = strtok_r(NULL, "|", &savePtr);
	errorStatus = CDRInsertOpCode(*_cdr, cdrDataStr);
	RETURN_PARSING_ERROR_AND_DESTROY_CDR_IF_FAILED;
	LOG_DEBUG_PRINT("Field %s inserted to operatorCode successfully", cdrDataStr);
	/* Field: call type */
	cdrDataStr = strtok_r(NULL, "|", &savePtr);
	callType = Convert2ChosenCallType(cdrDataStr);
	errorStatus = CDRInsertCallType(*_cdr, callType);
	RETURN_PARSING_ERROR_AND_DESTROY_CDR_IF_FAILED;
	LOG_DEBUG_PRINT("Field %s inserted to callType successfully", cdrDataStr);

	/* Field: call date: do nothing */
	cdrDataStr = strtok_r(NULL, "|", &savePtr);
	LOG_DEBUG_PRINT("%s", "Successfully 'ignored' Call Date");
	/* Field: call time: do nothing */
	cdrDataStr = strtok_r(NULL, "|", &savePtr);
	LOG_DEBUG_PRINT("%s", "Successfully 'ignored' Call Time");
	
	/* Field: call duration */
	cdrDataStr = strtok_r(NULL, "|", &savePtr);
	if(sscanf(cdrDataStr, "%u", &callDuration) != 1) 
	{ 
		CDRDestroy(*_cdr);
		GetError(strErr, ERR_PARSING_FAILED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_PARSING_FAILED;
    }
	LOG_DEBUG_PRINT("Field %s inserted to callDuration successfully", cdrDataStr);
	errorStatus = CDRInsertCallDuration(*_cdr, callDuration);
	RETURN_PARSING_ERROR_AND_DESTROY_CDR_IF_FAILED;
	/* Field: downloaded MB */
	cdrDataStr = strtok_r(NULL, "|", &savePtr);
	if(sscanf(cdrDataStr, "%lf", &downloaded) != 1) 
	{ 
		CDRDestroy(*_cdr);
		GetError(strErr, ERR_PARSING_FAILED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_PARSING_FAILED;
    }
	errorStatus = CDRInsertDownloadedMB(*_cdr, downloaded);
	RETURN_PARSING_ERROR_AND_DESTROY_CDR_IF_FAILED;
	LOG_DEBUG_PRINT("Field %s inserted to downloaded", cdrDataStr);
	/* Field: uploaded MB */
	cdrDataStr = strtok_r(NULL, "|", &savePtr);
	if(sscanf(cdrDataStr, "%lf", &uploaded) != 1) 
	{ 
		CDRDestroy(*_cdr);
		GetError(strErr, ERR_PARSING_FAILED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_PARSING_FAILED;
    }
	errorStatus = CDRInsertUploadedMB(*_cdr, uploaded);
	RETURN_PARSING_ERROR_AND_DESTROY_CDR_IF_FAILED;
	LOG_DEBUG_PRINT("Field %s inserted to uploaded", cdrDataStr);
	/* Field: party MSISDN */
	cdrDataStr = strtok_r(NULL, "|", &savePtr);
	errorStatus = CDRInsertPartyMSISDN(*_cdr, cdrDataStr);
	RETURN_PARSING_ERROR_AND_DESTROY_CDR_IF_FAILED;
	LOG_DEBUG_PRINT("Field %s inserted to partyMSISDN", cdrDataStr);
	/* Field: party operator */
	cdrDataStr = strtok_r(NULL, "|", &savePtr);
	errorStatus = CDRInsertPartyOperator(*_cdr, cdrDataStr);
	RETURN_PARSING_ERROR_AND_DESTROY_CDR_IF_FAILED;	
	LOG_DEBUG_PRINT("Field %s inserted to partyOperator", cdrDataStr);
	LOG_DEBUG_PRINT("%s", "Parsing Completed successfully");
	
	return ERR_OK;
}

/* TODO: Separate these functions: deliver to Transporter/Bridge module */
/* TODO: add function: IsEndMessage (for Shani to use), returns true:false */
/* insert CDR to queue */
ADTErr SendCDR2Queue(CDR* _cdr, SafeQueue* _queue)
{
	ADTErr errorStatus;
	char strErr[STR_ERROR_SIZE] = "";

	if (NULL == _cdr || NULL == _queue)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	errorStatus = SafeQueuePush(_queue, _cdr);
	if (errorStatus != ERR_OK)
	{
		CDRDestroy(_cdr);
		GetError(strErr, ERR_SENDING2Q_FAILED); /* TODO: SEND TO DB MANAGER : AND failed to send to DB manager in ADTErr */
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_SENDING2Q_FAILED;
	}
	LOG_DEBUG_PRINT("%s", "Sending CDR to Queue Completed successfully");

	return ERR_OK;
}

/* send end message to queue */ /* This will be changed a lot */
ADTErr SendEndMsg2Queue(SafeQueue* _queue)
{
	CDR* cdr = NULL; 
	ADTErr errorStatus;
	char strErr[STR_ERROR_SIZE] = "";		

	if (NULL == _queue)
	{
		GetError(strErr, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_NOT_INITIALIZED;
	}

	cdr = CDRCreate(&errorStatus);
	if (errorStatus != ERR_OK)
	{
		GetError(strErr, ERR_ALLOCATION_FAILED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_ALLOCATION_FAILED;
	}
	LOG_DEBUG_PRINT("%s", "CDR Successfuly created");

	errorStatus = CDRInsertIMSI(cdr, "END");
	if (errorStatus != ERR_OK)
	{
		CDRDestroy(cdr);
		GetError(strErr, ERR_INSERTION2CDR_FAILED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_INSERTION2CDR_FAILED;
	}
	LOG_DEBUG_PRINT("%s", "Insertion of 'END' message to CDR completed successfully");	
		
	errorStatus = SafeQueuePush(_queue, cdr);
	if (errorStatus != ERR_OK)
	{
		CDRDestroy(cdr);
		GetError(strErr, ERR_SENDING2Q_FAILED);
		LOG_ERROR_PRINT("%s", strErr);
		return ERR_SENDING2Q_FAILED;
	}
	LOG_DEBUG_PRINT("%s", "Sending 'END' message from CDR to Queue Completed successfully");

	return ERR_OK;		
}
