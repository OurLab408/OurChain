#include "ourcontract.h"
#include <stdio.h>

static struct
{
  int is_freezed;
  int user_count;
} state;
// note every print should add '\n' or it will not print
int contract_main(int argc, char **argv)
{
  // string print (should add '\n')
  str_printf("sample contract\n", 17 * sizeof(char), "%s");
  // error print (should add '\n'),can check in storage err.log
  err_printf("error log (sample)\n");
  // state access (in this simulator, struct can not read success)
  state_read(&state, sizeof(state));
  // do something on state
  state.is_freezed = 1;
  state.user_count = 10;
  // save back to state
  state_write(&state, sizeof(state));
  return 0;
}
