/**************************************************************************************************
	Author: 				Tom Vizel
	Creation date: 			18.10.15
	Last modified date: 	18.10.15
	
	Description: Header file for logger functions.
**************************************************************************************************/

#ifndef __LOGGER_H__
#define __LOGGER_H__

typedef enum e_LogLevel 
{						 /* Logging levels 				*/
    LOG_NONE  = 0x00,	 /* Silent mode					*/
    LOG_ERROR = 0x01,    /* Error     messages			*/
    LOG_WARN  = 0x02,    /* Warning   messages			*/
    LOG_DEBUG = 0x04,    /* Debugging messages			*/
    LOG_RMG   = 0x08     /* Resource manager messages	*/
} e_LogLevel;

/*
 * Create logfile
 *
 * 	Input:	_modes: 	Logging level modes
 *  		_logName:	Logger file name
 *
 *  Output:	ERR_OK (success) / 
 */
ADTErr  LogCreate   ( unsigned char _modes, const char* _logName );

/*
 * Stop logging operation
 */
void	LogDestroy	( void );

/*
 * Store log message
 *
 *	Note: Only registers with appropriate level permission
 *
 *  Input: 	_mode:     Logging level
 *   		_message:  The message itself
 */
void	LogRegister (unsigned char _mode, const char* _preMessage, const char* _message);

/*
 * Tests if _mode is currently active in logger.
 *
 * Returns true/false accordingly.
 */
int 	LogTestMode(unsigned char _mode);

#endif /* __LOGGER_H__ */
