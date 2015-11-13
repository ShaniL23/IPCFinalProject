/**************************************************************************************************
	Author: 				Tom Vizel
	Creation date: 			18.10.15
	Last modified date: 	18.10.15
	
	Description: Header file for logger macro definitions.
**************************************************************************************************/

#ifdef _LOGGER

#ifndef __logger_pub_h__
#define __logger_pub_h__

#define LOCAL_BUFFER 1024
char msg[LOCAL_BUFFER];
char preMsg[LOCAL_BUFFER];
	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"

#define LOG_ERROR_PRINT(fmt,...)\
	LOG_PRINT(LOG_ERROR, "ERROR", fmt, __VA_ARGS__)
	
#define LOG_WARN_PRINT(fmt,...)\
	LOG_PRINT(LOG_WARN, "WARNING", fmt, __VA_ARGS__)
	
#define LOG_DEBUG_PRINT(fmt,...)\
	LOG_PRINT(LOG_DEBUG, "DEBUG", fmt, __VA_ARGS__)
	
#define LOG_RMG_PRINT(fmt,...)\
	LOG_PRINT(LOG_RMG, "RMG", fmt, __VA_ARGS__)

#define LOG_PRINT(mode, type, fmt, ...)\
	if ( LogTestMode((mode)) && (NULL != (fmt)) )\
	{\
		snprintf(preMsg, LOCAL_BUFFER, "%s::%s[%d]:%s", __FILE__, __FUNCTION__, __LINE__, (type));\
		snprintf(msg, LOCAL_BUFFER, (fmt), __VA_ARGS__);\
		LogRegister((mode), preMsg, msg);\
	}
	
#pragma GCC diagnostic pop

#endif /*__logger_pub_h__*/

#endif /*_LOGGER*/
