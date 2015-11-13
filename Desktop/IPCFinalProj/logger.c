/**************************************************************************************************
	Author: 				Tom Vizel
	Creation date: 			30.9.15
	Last modified date: 	30.9.15
	
	Description: Implementation of logger functions.
**************************************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ADTErr.h"
#include "logger.h"
#include "logger_pub.h"

#define BUFFER_SIZE 1024

typedef struct Logger Logger;

struct Logger
{
	int				m_fileDesc;
	unsigned char 	m_modes;
};

/* Log declaration (global) */
Logger g_log = {-1, LOG_NONE};

static char* TimeStamp(char* _buffer)
{
    time_t currentTime;
    struct tm* timeInfo;
    
    currentTime = time(NULL);
    timeInfo = localtime(&currentTime);
    strftime(_buffer, BUFFER_SIZE, "[%Y-%m-%d %H:%M:%S]", timeInfo);
    return _buffer;
}

ADTErr LogCreate( unsigned char _modes, const char* _logName )
{
	if (NULL == _logName)
	{
		return ERR_ILLEGAL_INPUT;
	}
	
	if (g_log.m_fileDesc >= 0)
	{
		close(g_log.m_fileDesc);
		g_log.m_fileDesc = -1;
	}
	
	g_log.m_fileDesc = open(_logName, O_APPEND | O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);
	if (g_log.m_fileDesc < 0)
	{
		return ERR_ALLOCATION_FAILED;
	}
	g_log.m_modes = _modes;
	
	return ERR_OK;
}

void LogDestroy( void )
{
	if (g_log.m_fileDesc < 0)
	{
		return;
	}
	
	close(g_log.m_fileDesc);
	g_log.m_fileDesc = -1;
	g_log.m_modes = LOG_NONE;
}

void LogRegister (unsigned char _mode, const char* _preMessage, const char* _message)
{
	char timeBuff[BUFFER_SIZE] = "";
	char buffer[BUFFER_SIZE] = "";
	int buffSize = 0;
	
	if (g_log.m_fileDesc < 0 || ((_mode & g_log.m_modes) != _mode))
	{
		return;
	}
	
	buffSize = snprintf(buffer, BUFFER_SIZE, "%s %s %s\n", TimeStamp(timeBuff), _preMessage, _message);
	write(g_log.m_fileDesc, buffer, buffSize);
}

int LogTestMode(unsigned char _mode)
{
	return ( (g_log.m_fileDesc >= 0) && ((_mode & g_log.m_modes) == _mode) );
}
