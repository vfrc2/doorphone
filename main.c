#include <stdlib.h>
#include <argp.h>

const char *argp_program_version =
  "0.1.0";
const char *argp_program_bug_address =
  "<vfrc29@gmail.com>";

/* Program documentation. */
static char doc[] =
    "Auto answer phone to use as doorphone, can be run as daemon\v\n"
    "To call dest phones can send SIGUSR1 signal to process\n"
    "Use env for write process pid into file\n"
    "DOORPHONE_PID_FILE=/var/doorphone.pid";

static struct argp argp = { 0, 0, 0, doc };

int main(int argc, char** argv) {
    argp_parse(&argp, argc, argv, 0, 0, 0);
    exit(0);
}
