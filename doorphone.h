#ifndef _DOORPHONE_H
#define _DOORPHONE_H

typedef void doorphone_dtmf_cb(int dtmf_code);
typedef void doorphone_call_end_cb();

struct doorphone_options
{
  doorphone_dtmf_cb *dtmfCb;
  doorphone_call_end_cb *callEnd;
  char *linphoneConfig;
};

int doorphone_init(struct doorphone_options *options);

int doorphone_sequentialCall(int phonesc, char *phones[], int timeout, doorphone_call_end_cb *cb);

int doorphone_hangup();

int doorphone_loop();

void doorphone_destroy();

#endif