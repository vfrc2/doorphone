#ifndef _SIP_H
#define _SIP_H

typedef void sip_call_answer_cb();
typedef void sip_call_end_cb(int reason);

typedef struct
{
  sip_call_answer_cb *answerCb;
  sip_call_end_cb *callEnd;
  char *linphoneConfig;
} sip_options;

int sip_init(sip_options *options);

int sip_call(char *phone);

int sip_hangup();

int sip_loop();

void sip_destroy();

#endif