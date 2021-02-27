#ifndef _CALL_STRATEGY_H
#define _CALL_STRATEGY_H

struct call_strategy_base;

typedef void call_strategy_make_call(struct call_strategy_base *me, char *phone);
typedef void call_strategy_call(struct call_strategy_base *me, LinphoneCore *lc);
typedef void call_strategy_call_cb(struct call_strategy_base *me, LinphoneCore *lc, LinphoneCall *_call, LinphoneCallState cstate, const char *message);

struct call_strategy_base
{
    call_strategy_make_call *make_call;
    call_strategy_call *call;
    call_strategy_call_cb* call_cb;

};

typedef struct call_strategy_base call_strategy_base_t;

#endif