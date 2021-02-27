#include "./main.h"

#include <linphone/linphonecore.h>
#include "./call_strategy_seq.h"

static error_t parse_opt(int key, char *arg, struct argp_state *state);
static void halt(int signum);
static int write_pid(char *, int);

static void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message);
static void call_strategy();

#define RUN_STATE_RUNNING 1
#define RUN_STATE_HALT 0

LinphoneCore *lc;
call_strategy_base_t *strategy;
struct arguments arguments;

volatile int running = RUN_STATE_RUNNING;

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv)
{
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  LinphoneCoreVTable vtable = {0};

  vtable.call_state_changed = call_state_changed;

  lc = linphone_core_new(&vtable, "./user.conf", NULL, NULL);

  strategy = call_strategy_seq_create(arguments.dest, arguments.destc, lc);

  char *pidFileName = getenv(PID_FILE_ENV_NAME);
  if (pidFileName == 0)
  {
    pidFileName = "./doorphone.pid";
  }

  printf("PID: %d\n", getpid());
  write_pid(pidFileName, getpid());

  signal(SIGINT, &halt);
  signal(SIGUSR1, &call_strategy);

  running = RUN_STATE_RUNNING;

  if (arguments.daemon > 0)
  {
    call_strategy();
  }

  while (running == RUN_STATE_RUNNING)
  {
    linphone_core_iterate(lc);
    usleep(50000);
    // doorphone_hangup(currentCall);
  }

  printf("Exiting\n");
  linphone_core_destroy(lc);
  write_pid(pidFileName, -1);
  exit(0);
}

static void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message) {
  switch (cstate)
  {
  case LinphoneCallIncomingReceived:
  {
    // char address = 'a';
    char *address = linphone_call_get_remote_address_as_string(call);
    printf("Incoming call from %s\n", address);
    int res = linphone_core_accept_call(lc, call);
    printf("Answer phone %u\n", res);
    break;
  }
  default:
    break;
  }
  strategy->call_cb(strategy, lc, call, cstate, message);
}

static void call_strategy()
{
  strategy->call(strategy, lc);
}

static void halt(int signum)
{
  running = RUN_STATE_HALT;
}

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
  {
  case 'd':
    arguments->daemon = 1;
    break;
  case 'v':
    arguments->verbose = 1;
    break;

  case ARGP_KEY_ARG:
    if (state->arg_num >= 10)
      /* Too many arguments. */
      argp_usage(state);

    arguments->dest[state->arg_num] = arg;
    arguments->destc = state->arg_num + 1;

    break;

  case ARGP_KEY_END:
    if (state->arg_num < 1)
      /* Not enough arguments. */
      argp_usage(state);
    break;

  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static int write_pid(char *filename, int pid)
{
  FILE *fp;

  fp = fopen(filename, "w+");
  if (pid > 0)
  {
    fprintf(fp, "%d", pid);
  }
  fclose(fp);
  return 0;
}
