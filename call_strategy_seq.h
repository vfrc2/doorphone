#ifndef _CALL_STRATEGY_SEQ_H
#define _CALL_STRATEGY_SEQ_H

#include <stdlib.h>
#include <linphone/linphonecore.h>

#include "./call_strategy.h"

call_strategy_base_t* call_strategy_seq_create(char * phones[], const int phones_count, LinphoneCore* lc);

#endif
