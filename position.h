  {
    assert(trade_buffer_amt < Trade_buffer_size);
    trades[trade_buffer_amt] = CreateTrade(BUY, OPTION, 1, GetAmt::getAmt(6.25), CALL, GetAmt::getAmt(95.00));
    trade_buffer_amt ++;
  }
  {
    assert(trade_buffer_amt < Trade_buffer_size);
    trades[trade_buffer_amt] = CreateTrade(SELL, OPTION, 1, GetAmt::getAmt(1.75), CALL, GetAmt::getAmt(105.00));
    trade_buffer_amt ++;
  }
  {
    assert(trade_buffer_amt < Trade_buffer_size);
    trades[trade_buffer_amt] = CreateTrade(SELL, OPTION, 2, GetAmt::getAmt(7.75), PUT, GetAmt::getAmt(105.00));
    trade_buffer_amt ++;
  }
  {
    assert(trade_buffer_amt < Trade_buffer_size);
    trades[trade_buffer_amt] = CreateTrade(SELL, STOCK, 2, GetAmt::getAmt(98.00));
    trade_buffer_amt ++;
  }
