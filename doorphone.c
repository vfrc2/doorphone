#include <linphone/linphonecore.h>
#include "./doorphone.h"

LinphoneCore *lc;
LinphoneCall *call = NULL;
struct doorphone_options options;

/*
 * Call state notification callback
 */
static void call_state_changed(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *msg)
{
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
  case LinphoneCallOutgoingRinging:
    printf("It is now ringing remotely !\n");
    break;
  case LinphoneCallOutgoingEarlyMedia:
    printf("Receiving some early media\n");
    break;
  case LinphoneCallConnected:
    printf("We are connected !\n");
    break;
  case LinphoneCallStreamsRunning:
    printf("Media streams established !\n");
    break;
  case LinphoneCallEnd:
    printf("Call is terminated.\n");
    break;
  case LinphoneCallError:
    printf("Call failure !");
    break;
  default:
    printf("Unhandled notification %i\n", cstate);
  }
}

static void dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf) {
  char key = (char)dtmf;
  printf("Received dtmf code %c\n", key);
  // options.dtmfCbdtmfCb(dtmf);
}

int doorphone_init(struct doorphone_options *opts)
{
  LinphoneCoreVTable vtable = {0};
  options = *opts;

#ifdef DEBUG_LOGS
  linphone_core_enable_logs(NULL); /*enable liblinphone logs.*/
#endif
  /* 
	 Fill the LinphoneCoreVTable with application callbacks.
	 All are optional. Here we only use the call_state_changed callbacks
	 in order to get notifications about the progress of the call.
	 */
  vtable.call_state_changed = call_state_changed;
  vtable.dtmf_received = dtmf_received;

  lc = linphone_core_new(&vtable, "./user.conf", NULL, NULL);
}

int doorphone_sequentialCall(int phonesc, char *phones[], int timeout, doorphone_call_end_cb *cb)
{
  if (!call || linphone_call_get_state(call) == LinphoneCallEnd)
  {
    if (phonesc > 0)
    {
      LinphoneAddress *dest = linphone_core_interpret_url(lc, phones[0]);
      if (dest == NULL)
      {
        return 1;
      }
      call = linphone_core_invite_address(lc, dest);
      if (call == NULL)
      {
        printf("Could not place call to %s\n", linphone_address_as_string(dest));
        return 2;
      }
      else
      {
        printf("Call to %s is in progress...\n", linphone_address_as_string_uri_only(dest));
        linphone_call_ref(call);
      }
    }
    return 0;
  }
  printf("Call already in progress\n");
  return 1;
}

int doorphone_loop()
{
  linphone_core_iterate(lc);
}

int doorphone_hangup()
{
  printf("Terminating the call...\n");
  linphone_core_terminate_call(lc, call);
  /*at this stage we don't need the call object */
  linphone_call_unref(call);
}

void doorphone_destroy()
{
  linphone_core_destroy(lc);
}