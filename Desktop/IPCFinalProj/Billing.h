/*************************************************************************
Author: Shani Levinkind
Creation Date: 9.11.15
Last edit: 12.11.15
Description: Header file for Billing functions declarations
					SIGUSR1 - print subscriber bills.
					SIGUSR2 - print operator bills
**************************************************************************/

#ifndef __BILLING_H__
#define __BILLING_H__

ADTErr BillingInit (DBManagerParams* _mngParams);
ADTErr EndBilling (void);

#endif /*__BILLING_H__*/
















