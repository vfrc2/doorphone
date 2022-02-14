#include <argp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

#include "./app.h"
#include "./sip.h"

#define STATE_INIT 0
#define STATE_WAIT 1
#define STATE_CALL 2
#define STATE_TALKING 3
#define STATE_FIRST 4
#define STATE_NEXT 5
#define STATE_EXIT 6

#define EVENT_ENTER 0
#define EVENT_CALL 1
#define EVENT_ANSWER 2
#define EVENT_HANGUP 3
#define EVENT_TIMEOUT 4
#define EVENT_ERROR 5

int state = STATE_INIT;

app_config *config = NULL;

// INIT     - on enter [!daemon] -> FIRST
//          - on enter -> WAIT

// CALL     - on enter { make call }
//          - on timeout -> NEXT
//          - on answer -> TALKING

// FIRST    - on enter { set first url } -> CALL
// NEXT     - on enter [ cur < count ] { set next url } -> CALL
//          - on enter -> WAIT

// TALKING  - on hang -> WAIT

// WAIT     - on enter [!daemon] -> EXIT
//          - on call -> FIRST

static int writePid(char *filename, int pid);

#define HANDLER_ARGS app_config *opts, void *data

typedef void state_handler(HANDLER_ARGS);

void init_on_enter(HANDLER_ARGS);
void wait_on_enter(HANDLER_ARGS);
void wait_on_call(HANDLER_ARGS);
void call_on_enter(HANDLER_ARGS);
void call_on_answer(HANDLER_ARGS);
void call_on_timeout(HANDLER_ARGS);
void call_on_hangup(HANDLER_ARGS);
void talking_on_hangup(HANDLER_ARGS);
void first_on_enter(HANDLER_ARGS);
void next_on_enter(HANDLER_ARGS);

// clang-format off

state_handler* states[7][6] = {
                 // on enter,     on call,       on answer,            on_hangup,          on_timeout        on_error
  /* INIT */    { &init_on_enter,  NULL,          NULL,                 NULL,               NULL,             NULL,            },
  /* WAIT */    { &wait_on_enter,  &wait_on_call, NULL,                 NULL,               NULL,             NULL,            },
  /* CALL */    { &call_on_enter,  NULL,          &call_on_answer,      &call_on_hangup,    &call_on_timeout, &call_on_timeout },
  /* TALKING */ { NULL,            NULL,          NULL,                 &talking_on_hangup, NULL,             NULL,            },
  /* FIRST */   { &first_on_enter, NULL,          NULL,                 NULL,               NULL,             NULL,            },
  /* NEXT */    { &next_on_enter,  NULL,          NULL,                 NULL,               NULL,             NULL,            },
  /* EXIT */    { NULL,            NULL,          NULL,                 NULL,               NULL,             NULL,            }
};

// clang-format on

void dispatch(int event, void *data);

void changeState(int newState) {
  int prevState = state;
  state = newState;
  dispatch(EVENT_ENTER, &prevState);
}

void setTimeoutSec(int delay);

void clearTimeout();

int phoneIndex = 0;

void init_on_enter(app_config *opts, void *data) {
  if (!opts->daemon) {
    changeState(STATE_FIRST);
    return;
  }
  changeState(STATE_WAIT);
}

void wait_on_enter(app_config *opts, void *data) {
  if (!opts->daemon) {
    changeState(STATE_EXIT);
  }
};

void wait_on_call(app_config *opts, void *data) { changeState(STATE_FIRST); };

void call_on_enter(app_config *opts, void *data) {
  printf("Calling to %i...\n", phoneIndex);
  int result = sip_call(opts->dest[phoneIndex]);
  if (result == 0) {
    if (opts->timeout > NO_TIMEOUT) {
      setTimeoutSec(opts->timeout);
    }
  } else {
    changeState(STATE_NEXT);
  }
}

void call_on_answer(app_config *opts, void *data) {
  changeState(STATE_TALKING);
  clearTimeout();
}

void call_on_timeout(app_config *opts, void *data) { sip_hangup(); }

void call_on_hangup(app_config *opts, void *data) { changeState(STATE_NEXT); }

void talking_on_hangup(app_config *opts, void *data) {
  changeState(STATE_WAIT);
}

void first_on_enter(app_config *opts, void *data) {
  phoneIndex = 0;
  changeState(STATE_CALL);
  printf("Starting call sequence...\n");
}

void next_on_enter(app_config *opts, void *data) {
  phoneIndex++;
  if (phoneIndex >= opts->destc)
    changeState(STATE_WAIT);
  else
    changeState(STATE_CALL);
}

void dispatch(int event, void *data) {
  state_handler *handler = states[state][event];
  if (handler) handler(config, data);
}

void do_dispatch_exit() {
  printf("Call SIG received...\n");
  // all states no event
  changeState(STATE_EXIT);
}

void do_dispatch_call() { dispatch(EVENT_CALL, NULL); }

void do_dispatch_call_end(int error) {
  if (error > 0)
    dispatch(EVENT_ERROR, &error);
  else
    dispatch(EVENT_HANGUP, NULL);
}

void do_dispatch_answer() { dispatch(EVENT_ANSWER, NULL); }

int timer = 0;
int timerEnabled = FALSE;
int prevClock = 0;

void setTimeoutSec(int delay) {
  timer = delay * CLOCKS_PER_SEC;
  timerEnabled = TRUE;
  prevClock = (int)clock();
}

void clearTimeout() {
  timerEnabled = FALSE;
  timer = 0;
}

int app_start(app_config *_config) {
  config = _config;
  sip_options opts;

  char *pidFileName = getenv(PID_FILE_ENV_NAME);
  if (pidFileName == 0) {
    pidFileName = "./doorphone.pid";
  }

  printf("PID: %d\n", getpid());
  writePid(pidFileName, getpid());

  opts.callEnd = &do_dispatch_call_end;
  opts.answerCb = &do_dispatch_answer;

  sip_init(&opts);

  signal(SIGINT, do_dispatch_exit);
  signal(SIGQUIT, do_dispatch_exit);
  signal(SIGTSTP, do_dispatch_exit);

  if (config->daemon) {
    signal(SIGUSR1, do_dispatch_call);
  }

  changeState(STATE_INIT);

  while (state != STATE_EXIT) {
    sip_loop();
    usleep(50000);

    if (timerEnabled) {
      timer -= (int)clock() - prevClock;
      if (timer < 0) {
        clearTimeout();
        dispatch(EVENT_TIMEOUT, NULL);
      }
    }
  }

  printf("Exiting\n");
  sip_destroy();
  writePid(pidFileName, -1);
  exit(0);
}

static int writePid(char *filename, int pid) {
  FILE *fp;

  fp = fopen(filename, "w+");
  if (pid > 0) {
    fprintf(fp, "%d", pid);
  }
  fclose(fp);
  return 0;
}
