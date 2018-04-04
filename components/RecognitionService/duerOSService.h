#ifndef _DuerOS_SERVICE_H_
#define _DuerOS_SERVICE_H_


typedef enum {
    UploadEvt_Unknown,
    UploadEvt_Init,
    UploadEvt_Start,
    UploadEvt_Data,
    UploadEvt_Stop,
    UploadEvt_Quit,  //Add for log out
} UploadEventType;

void duerOSEvtSend(UploadEventType type, void* data, int len, int dir);



typedef void PF_DUEROS_PLAY_CALLBACK (char* url,char*text,char *payload);

void initDuerOS(PF_DUEROS_PLAY_CALLBACK*pf);

#endif
