#include "./doorphone.h"

#include <linphone/linphonecore.h>

LinphoneCore *lc;
LinphoneCall *call;
struct doorphone_options options;

/*
 * Call state notification callback
 */
static void call_state_changed(LinphoneCore *lc, LinphoneCall *_call,
                               LinphoneCallState cstate, const char *msg) {
  switch (cstate) {
    case LinphoneCallIncomingReceived: {
      // char address = 'a';
      char *address = linphone_call_get_remote_address_as_string(_call);
      printf("Incoming call from %s\n", address);
      int res = linphone_core_accept_call(lc, _call);
      printf("Answer phone %u\n", res);
      break;
    }
    case LinphoneCallEnd:
    // case LinphoneCallReleased:
      printf("Call is terminated.\n");
      linphone_call_unref(_call);
      call = NULL;
      if (options.callEnd) {
        options.callEnd(0);
      }
      break;
    case LinphoneCallError:
      printf("Call failure!\n");
      if (options.callEnd) {
        options.callEnd(cstate);
      }
      break;
    default:
      printf("Unhandled notification %i\n", cstate);
  }
}

int doorphone_init(struct doorphone_options *opts) {
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

  lc = linphone_core_new(&vtable, "./user.conf", NULL, NULL);
  return 0;
}

int doorphone_call(char *phone) {
  if (!(call)) {
    LinphoneAddress *dest = linphone_core_interpret_url(lc, phone);
    if (dest == NULL) {
      return 1;
    }
    call = linphone_core_invite_address(lc, dest);
    if (call == NULL) {
      printf("Could not place call to %s\n", linphone_address_as_string(dest));
      return 2;
    } else {
      printf("Call to %s is in progress...\n",
             linphone_address_as_string_uri_only(dest));
      linphone_call_ref(call);
    }
    return 0;
  } else {
    printf("Call already in progress (%i)\n", linphone_call_get_state(call));
    return 1;
  }
}

int doorphone_loop() {
  linphone_core_iterate(lc);
  return 0;
}

int doorphone_hangup() {
  if (call == NULL) {
    printf("No call in progress...");
    return 1;
  }
  printf("Terminating the call...\n");
  linphone_core_terminate_call(lc, call);
  return 0;
}

void doorphone_destroy() {
  linphone_core_terminate_all_calls(lc); 
  // linphone_core_destroy(lc);

  printf("doorphone terminated cleanly\n");
}