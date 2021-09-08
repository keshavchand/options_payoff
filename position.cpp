#include <stdio.h>
#define TRADE_CREATION_IMPLEMENTATION
#include "trade_creation.h"
#include <assert.h>
#include <windows.h>
 
extern "C" __declspec(dllexport) void InitSampleTrades(Trade *trades, int* trade_buffer_amt, int trade_buffer_size){
  *trade_buffer_amt = 0;
#if 1
  {
    assert(*trade_buffer_amt < trade_buffer_size);
    trades[*trade_buffer_amt] = CreateTrade(SELL, OPTION, 5, getAmt(6.25f), PUT, getAmt(95.00f));
    (*trade_buffer_amt) ++;
  }
  {
    assert(*trade_buffer_amt < trade_buffer_size);
    trades[*trade_buffer_amt] = CreateTrade(SELL, OPTION, 1, getAmt(1.75f), CALL, getAmt(105.00f));
    (*trade_buffer_amt) ++;
  }
  {
    assert(*trade_buffer_amt < trade_buffer_size);
    trades[*trade_buffer_amt] = CreateTrade(BUY, OPTION, 2, getAmt(0.0f), CALL, getAmt(105.84f));
    (*trade_buffer_amt) ++;
  }
  {
    assert(*trade_buffer_amt < trade_buffer_size);
    trades[*trade_buffer_amt] = CreateTrade(SELL, OPTION, 2, getAmt(7.75f), PUT, getAmt(105.00f));
    (*trade_buffer_amt) ++;
  }
#endif
  {
    assert(*trade_buffer_amt < trade_buffer_size);
    trades[*trade_buffer_amt] = CreateTrade(BUY, STOCK, 2, getAmt(98.00f));
    (*trade_buffer_amt) ++;
  }
}


BOOL DllMain( HANDLE hModule,	   // Handle to DLL module 
   DWORD ul_reason_for_call, 
   LPVOID lpReserved )     // Reserved
{
  return 1;
}
