#include <argp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "version.h"

#define TRUE 1
#define FALSE 0

#include "./sip.h"
#include "./app.h"

const char *argp_program_version = PROJECT_VER;
const char *argp_program_bug_address = "<vfrc29@gmail.com>";

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

/* The options we understand. */
static struct argp_option options[] = {
    // {"dest-list", 'l', "CALL_LIST", 0,
    //  "Provide call list from file"},
    {"daemon", 'd', 0, 0,
     "Run in daemon mode, do not exit, answer incoming call, call on signal "
     "SIGUSR1"},
    {"timeout", 't', "TIMEOUT", 0,
     "Answer timeout in seconds, for no timeot -1"},
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  app_config * config = state->input;

  switch (key) {
    case 'd':
      config->daemon = 1;
      break;
    case 'v':
      config->verbose = 1;
      break;
    case 't':
      config->timeout = atoi(arg);
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 10) /* Too many config. */
        argp_usage(state);

      config->dest[state->arg_num] = arg;
      config->destc = state->arg_num + 1;

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

int main(int argc, char **argv) {
  app_config config;
  config.daemon = 0;
  config.verbose = 0;
  config.destc = 0;
  config.timeout = 60;

  argp_parse(&argp, argc, argv, 0, 0, &config);

  int result = app_start(&config);

  exit(result);
}