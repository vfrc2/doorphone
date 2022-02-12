#ifndef _DOORPHONE_H
#define _DOORPHONE_H

typedef void doorphone_call_answer_cb();
typedef void doorphone_call_end_cb(int reason);

struct doorphone_options
{
  doorphone_call_answer_cb *answerCb;
  doorphone_call_end_cb *callEnd;
  char *linphoneConfig;
};

int doorphone_init(struct doorphone_options *options);

int doorphone_call(char *phone);

int doorphone_hangup();

int doorphone_loop();

void doorphone_destroy();

#endif