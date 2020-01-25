#include <stdlib.h>
#include <argp.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "./doorphone.h"

const char *argp_program_version =
    "0.1.0";
const char *argp_program_bug_address =
    "<vfrc29@gmail.com>";

/* Program documentation. */
static char doc[] =
    "Call sip phones from provide list, wait call ends and exit.\n,"
    "DEST - list of sip phones, which call to (max 10)\n\v"
    "Can be run as daemon, when don't call list immediately, "
    "wait for incoming calls and auto answer.\n"
    "To call dest phones send SIGUSR1 signal to process\n"
    "Use env for write process pid into file\n"
    "DOORPHONE_PID_FILE=/var/doorphone.pid";

static char args_doc[] = "[DEST...]";

static void stop(int signum);
static void call();
static void sequentialCallEnd();
static void phoneCmd(int dtmf_cmd);

struct arguments
{
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
     "Run in daemon mode, do not exit, answer incoming call, call on signal SIGUSR1"},
    {"verbose", 'v', 0, 0,
     "Produce verbose output"},
    {0}};

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
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
        argp_usage (state);

      arguments->dest[state->arg_num] = arg;
      arguments->destc = state->arg_num + 1;

      break;

    case ARGP_KEY_END:
      if (state->arg_num < 1)
        /* Not enough arguments. */
        argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int running = 1;
int calling = 0;
struct arguments arguments;

int main(int argc, char **argv)
{
  arguments.daemon = 0;
  arguments.verbose = 0;
  arguments.destc = 0;
  arguments.timeout = 120;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  struct doorphone_options opts;

  opts.dtmfCb = &phoneCmd;

  doorphone_init(&opts);

  if (arguments.daemon == 0) {
    doorphone_sequentialCall(arguments.destc, arguments.dest, arguments.timeout, &sequentialCallEnd);
  }

  signal(SIGINT, stop);
	signal(SIGUSR1, call);

  while (running)
  {
    doorphone_loop();

    if (calling) {
      calling = 0;
      printf("Make call to %s\n", arguments.dest[0]);
      doorphone_sequentialCall(arguments.destc, arguments.dest, arguments.timeout, &sequentialCallEnd);
    }

    usleep(50000);
  }

  doorphone_destroy();
  exit(0);
}

static void phoneCmd(int dtmf_cmd) {
  printf("Receive cmd %i", dtmf_cmd);
}

static void sequentialCallEnd() {
  if (arguments.daemon == 0) {
    stop(-1);
  }
}

static void call() {
  calling = 1;
}

static void stop(int signum)
{
	running = 0;
}

