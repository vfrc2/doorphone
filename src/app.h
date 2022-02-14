#ifndef _APP_H
#define _APP_H

#define PID_FILE_ENV_NAME "DOORPHONE_PID_FILE"

#define MAX_DEST_COUNT 10

#define NO_TIMEOUT -1

typedef struct {
  char *dest[MAX_DEST_COUNT];
  char destc;
  int daemon;
  int timeout;
  int verbose;
} app_config;

int app_start(app_config * config);

#endif