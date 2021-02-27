#include <string.h>

#include "./call_strategy_seq.h"

typedef struct
{
    struct call_strategy_base base;
    int state;
    int call_index;
    char *phones[10];
    int phones_count;
} call_strategy_seq;

#define STATE_WAIT 0
#define STATE_NEXT 1
#define STATE_CALL 2
#define STATE_ERROR 10

static void call(call_strategy_seq *me, LinphoneCore *lc);
static void call_cb(call_strategy_seq *me, LinphoneCore *lc, LinphoneCall *_call, LinphoneCallState cstate, const char *message);
static void reset_sequence(call_strategy_seq *me);

call_strategy_base_t *call_strategy_seq_create(char *phones[], const int phones_count, LinphoneCore * lc)
{
    call_strategy_seq *a = malloc(sizeof(call_strategy_seq));
    a->base.call = (call_strategy_call *)&call;
    a->base.call_cb = (call_strategy_call_cb *)&call_cb;
    for (int i = 0; i < phones_count; i++)
    {
        a->phones[i] = phones[i];
        // strcpy(a->phones[i], phones[i]);
    }
    a->phones_count = phones_count;
    a->call_index = 0;

    return (call_strategy_base_t *)a;
}

static void call(call_strategy_seq *me, LinphoneCore *lc)
{
    if (me->state == STATE_CALL)
    {
        printf("Already calling\n");
        return;
    }
    me->state = STATE_CALL;

    if (me->call_index >= me->phones_count)
    {
        printf("No more dest");
        me->state = STATE_ERROR;
        return;
    }

    char *dest = me->phones[me->call_index];
    printf("Calling %s...\n", dest);
    LinphoneCall *call = linphone_core_invite(lc, dest);

    if (call == NULL)
    {
        printf("Could not place call to %s\n", dest);
        call_cb(me, lc, NULL, LinphoneCallError, NULL);
        return;
    }

    printf("Call to %s is in progress...\n", dest);
    linphone_call_ref(call);
    return;
}

static void call_cb(call_strategy_seq *me, LinphoneCore *lc, LinphoneCall *_call, LinphoneCallState cstate, const char *message)
{
    switch (cstate)
    {
    case LinphoneCallEnd:
    {
        // first success answer reset sequence
        reset_sequence(me);
        printf("Call finished\n");
        break;
    }
    break;

    case LinphoneCallError:
    {
        printf("Error calling %s.", me->phones[me->call_index]);
        me->call_index++;
        // if try all numbers, end
        if (me->call_index >= me->phones_count)
        {
            reset_sequence(me);
            printf(" No more addresses\n");
            break;
        }
        printf(" Try next address\n");
        me->state = STATE_NEXT;
        call(me, lc);
        break;
    }
    break;
    default:
        break;
    }
}

static void reset_sequence(call_strategy_seq *me)
{
    me->state = STATE_WAIT;
    me->call_index = 0;
}
