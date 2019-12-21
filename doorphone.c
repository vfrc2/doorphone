/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @defgroup basic_call_tutorials Basic call
 * @ingroup tutorials
 This program is a _very_ simple usage example of liblinphone.
 It just takes a sip-uri as first argument and attempts to call it

 @include helloworld.c
 */

#include <linphone/linphonecore.h>
#include <argp.h>
#include <signal.h>

static bool_t running = TRUE;
static bool_t makeCall = FALSE;

static void stop(int signum)
{
	running = FALSE;
}

static void sigCall(int signum)
{
	// printf("Receive USR1");
	makeCall = TRUE;
}

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
		printf("Answer phone %u", res);
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

static void dtmf_received(LinphoneCore *lc, LinphoneCall *call, int dtmf)
{
	printf("Receive dtmf %d\n", dtmf);
}

int main(int argc, char *argv[])
{
	LinphoneCoreVTable vtable = {0};
	LinphoneCore *lc;
	LinphoneCall *call = NULL;
	const char *dest = NULL;

	/* take the destination sip uri from the command line arguments */
	if (argc > 1)
	{
		dest = argv[1];
		printf("Dest %s\n", dest);
		makeCall = strcmp(argv[0],"true");
	}

	signal(SIGINT, stop);
	signal(SIGUSR1, sigCall);

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
	/*
	 Instanciate a LinphoneCore object given the LinphoneCoreVTable
	*/
	lc = linphone_core_new(&vtable, NULL, NULL, NULL);

	/* main loop for receiving notifications and doing background linphonecore work: */
	while (running)
	{
		if (makeCall)
		{
			/*
		 	 * Place an outgoing call
			 */
			call = linphone_core_invite(lc, dest);
			if (call == NULL)
			{
				printf("Could not place call to %s\n", dest);
				goto end;
			}
			else
				printf("Call to %s is in progress...", dest);
			linphone_call_ref(call);
			makeCall = FALSE;
		}

		linphone_core_iterate(lc);
		ms_usleep(50000);
	}

	if (call && linphone_call_get_state(call) != LinphoneCallEnd)
	{
		/* terminate the call */
		printf("Terminating the call...\n");
		linphone_core_terminate_call(lc, call);
		/*at this stage we don't need the call object */
		linphone_call_unref(call);
	}

end:
	printf("Shutting down...\n");
	linphone_core_destroy(lc);
	printf("Exited\n");
	return 0;
}