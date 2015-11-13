/*******************************************************************************************************
Author:				Arkadi Zavurov
Description:		* Get CDR string and convert to CDR struct
					* Insert CDR to queue
					* Send "END" message to queue 
					---  Parser Header File ---
*******************************************************************************************************/
#ifndef __PARSER_H__
#define __PARSER_H__

ADTErr Parse(char* _cdrString, CDR** _cdr);
ADTErr SendCDR2Queue(CDR* _cdr, SafeQueue* _queue);
ADTErr SendEndMsg2Queue(SafeQueue* _queue);

#endif /* __PARSER_H__ */
