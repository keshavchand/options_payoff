#include <stdio.h>
#define TRADE_CREATION_IMPLEMENTATION
#include "trade_creation.h"
#include <assert.h>
#include <windows.h>

static int getAmt(float a){
  return (int) a * 100;
}

//__declspec(dllexport) 
extern "C" void InitSampleTrades(Trade *trades, int trade_buffer_size){
  trade_buffer_amt = 0;
  {
    assert(trade_buffer_amt < trade_buffer_size);
    trades[trade_buffer_amt] = CreateTrade(BUY, OPTION, 1, getAmt(6.25), CALL, getAmt(95.00));
    trade_buffer_amt ++;
  }
  {
    assert(trade_buffer_amt < trade_buffer_size);
    trades[trade_buffer_amt] = CreateTrade(SELL, OPTION, 1, getAmt(1.75), CALL, getAmt(105.00));
    trade_buffer_amt ++;
  }
  {
    assert(trade_buffer_amt < trade_buffer_size);
    trades[trade_buffer_amt] = CreateTrade(BUY, OPTION, 2, getAmt(0.0), CALL, getAmt(115.84));
    trade_buffer_amt ++;
  }
  {
    assert(trade_buffer_amt < trade_buffer_size);
    trades[trade_buffer_amt] = CreateTrade(SELL, OPTION, 2, getAmt(7.75), PUT, getAmt(105.00));
    trade_buffer_amt ++;
  }
  {
    assert(trade_buffer_amt < trade_buffer_size);
    trades[trade_buffer_amt] = CreateTrade(SELL, STOCK, 2, getAmt(98.00));
    trade_buffer_amt ++;
  }
  {
    assert(trade_buffer_amt < trade_buffer_size);
    trades[trade_buffer_amt] = CreateTrade(SELL, STOCK, 2, getAmt(105.00));
    trade_buffer_amt ++;
  }
  {
    assert(trade_buffer_amt < trade_buffer_size);
    trades[trade_buffer_amt] = CreateTrade(BUY, STOCK, 2, getAmt(100.00));
    trade_buffer_amt ++;
  }
}


BOOL DllMain( HANDLE hModule,	   // Handle to DLL module 
   DWORD ul_reason_for_call, 
   LPVOID lpReserved )     // Reserved
{
  return 1;
}
