#include <argp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

#include "./doorphone.h"

const char *argp_program_version = "0.1.0";
const char *argp_program_bug_address = "<vfrc29@gmail.com>";

#define PID_FILE_ENV_NAME "DOORPHONE_PID_FILE"

/* Program documentation. */
static char doc[] =
    "Call sip phones from provided list, wait call ends and exit.\n\n"
    "DEST - list of sip phones, which call to (max 10)\n\n"
    "Can be run as daemon, when don't call list immediately, "
    "wait for incoming calls and auto answer.\n"
    "To call dest phones send SIGUSR1 signal to process\n"
    "Use env for write process pid into file " PID_FILE_ENV_NAME
    "=/var/doorphone.pid";

static char args_doc[] = "[DEST...]";

static int writePid(char *, int);

struct arguments {
  char *dest[10];
  char destc;
  int daemon;
  int timeout;
  int verbose;
};

/* The options we understand. */
static struct argp_option options[] = {
    // {"dest-list", 'l', "CALL_LIST", 0,
    //  "Provide call list from file"},
    {"daemon", 'd', 0, 0,
     "Run in daemon mode, do not exit, answer incoming call, call on signal "
     "SIGUSR1"},
    {"timeout", 't', "TIMEOUT", 0,
     "Answer timeout in seconds"
     "SIGUSR1"},
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key) {
    case 'd':
      arguments->daemon = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case 't':
      arguments->timeout = atoi(arg);
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 10) /* Too many arguments. */
        argp_usage(state);

      arguments->dest[state->arg_num] = arg;
      arguments->destc = state->arg_num + 1;

      break;

    case ARGP_KEY_END:
      if (state->arg_num < 1) /* Not enough arguments. */
        argp_usage(state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

struct arguments arguments;

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

#define HANDLER_ARGS struct arguments opts, void *data

typedef void state_handler(HANDLER_ARGS);

void init_on_enter(HANDLER_ARGS);
void wait_on_enter(HANDLER_ARGS);
void wait_on_call(HANDLER_ARGS);
void call_on_enter(HANDLER_ARGS);
void call_on_answer(HANDLER_ARGS);
void call_on_timeout(HANDLER_ARGS);
void talking_on_hangup(HANDLER_ARGS);
void first_on_enter(HANDLER_ARGS);
void next_on_enter(HANDLER_ARGS);

// clang-format off

state_handler* states[7][6] = {
                 // on enter,     on call,       on answer,            on_hangup,          on_timeout        on_error
  /* INIT */    { &init_on_enter,  NULL,          NULL,                 NULL,               NULL,             NULL,            },
  /* WAIT */    { &wait_on_enter,  &wait_on_call, NULL,                 NULL,               NULL,             NULL,            },
  /* CALL */    { &call_on_enter,  NULL,          &call_on_answer,      NULL,               &call_on_timeout, &call_on_timeout },
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

void init_on_enter(struct arguments opts, void *data) {
  if (!opts.daemon) {
    changeState(STATE_FIRST);
    return;
  }
  changeState(STATE_WAIT);
}

void wait_on_enter(struct arguments opts, void *data) {
  if (!opts.daemon) {
    changeState(STATE_EXIT);
  }
};

void wait_on_call(struct arguments opts, void *data) {
  changeState(STATE_FIRST);
};

void call_on_enter(struct arguments opts, void *data) {
  printf("Calling to %i...\n", phoneIndex);
  setTimeoutSec(opts.timeout);
  doorphone_call(opts.dest[phoneIndex]);
}

void call_on_answer(struct arguments opts, void *data) {
  changeState(STATE_TALKING);
  clearTimeout();
}

void call_on_timeout(struct arguments opts, void *data) {
  doorphone_hangup();
  changeState(STATE_NEXT);
}

void talking_on_hangup(struct arguments opts, void *data) {
  changeState(STATE_WAIT);
}

void first_on_enter(struct arguments opts, void *data) {
  phoneIndex = 0;
  changeState(STATE_CALL);
  printf("Starting call sequence...\n");
}

void next_on_enter(struct arguments opts, void *data) {
  phoneIndex++;
  if (phoneIndex >= opts.destc)
    changeState(STATE_WAIT);
  else
    changeState(STATE_CALL);
}

void dispatch(int event, void *data) {
  state_handler *handler = states[state][event];
  if (handler) handler(arguments, data);
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

int main(int argc, char **argv) {
  arguments.daemon = 0;
  arguments.verbose = 0;
  arguments.destc = 0;
  arguments.timeout = 60;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  struct doorphone_options opts;

  char *pidFileName = getenv(PID_FILE_ENV_NAME);
  if (pidFileName == 0) {
    pidFileName = "./doorphone.pid";
  }

  printf("PID: %d\n", getpid());
  writePid(pidFileName, getpid());

  opts.callEnd = &do_dispatch_call_end;
  opts.answerCb = &do_dispatch_answer;

  doorphone_init(&opts);

  signal(SIGINT, do_dispatch_exit);
  signal(SIGQUIT, do_dispatch_exit);
  signal(SIGTSTP, do_dispatch_exit);


  if (arguments.daemon) {
    signal(SIGUSR1, do_dispatch_call);
  }

  changeState(STATE_INIT);

  while (state != STATE_EXIT) {
    doorphone_loop();
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
  doorphone_destroy();
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
