#include <stdlib.h>
#include <argp.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

const char *argp_program_version =
    "0.1.0";
const char *argp_program_bug_address =
    "<vfrc29@gmail.com>";

#define PID_FILE_ENV_NAME "DOORPHONE_PID_FILE"

/* Program documentation. */
static char doc[] =
    "Call sip phones from provided list, wait call ends and exit.\n\n"
    "DEST - list of sip phones, which call to (max 10)\n\n"
    "Can be run as daemon, when don't call list immediately, "
    "wait for incoming calls and auto answer.\n"
    "To call dest phones send SIGUSR1 signal to process\n"
    "Use env for write process pid into file " PID_FILE_ENV_NAME "=/var/doorphone.pid";

static char args_doc[] = "[DEST...]";

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