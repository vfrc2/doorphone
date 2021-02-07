#ifndef _DOORPHONE_H
#define _DOORPHONE_H

typedef void doorphone_dtmf_cb(int dtmf_code);
typedef void doorphone_call_end_cb(int result);

struct doorphone_options
{
  doorphone_dtmf_cb *dtmfCb;
  doorphone_call_end_cb *callEnd;
  char *linphoneConfig;
};

#define DOORPHONE_ERROR_BAD_ADDRESS 1
#define DOORPHONE_ERROR_CANT_PLACE_CALL 2
#define DOORPHONE_ERROR_CALL_ALREADY_IN_PROGESS 3

#define DOORPHONE_CALL_END_CB_OK 0
#define DOORPHONE_CALL_END_CB_ERROR 1

int doorphone_init(struct doorphone_options *options);

int doorphone_call(char *phone, int timeout, doorphone_call_end_cb *cb);

int doorphone_hangup();

int doorphone_loop();

void doorphone_destroy();

#endif