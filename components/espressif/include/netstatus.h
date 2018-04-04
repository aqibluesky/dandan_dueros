#ifndef _EVENT_NET_STATUS_H

typedef enum _EzhNetStatus{
	ezhNetStat_NoNet,
	ezhNetStat_NET_OK
}EzhNetStatus;

void		setCurNetStatus(EzhNetStatus ezh);
EzhNetStatus		getCurNetStatus();

#define _EVENT_NET_STATUS_H
#endif