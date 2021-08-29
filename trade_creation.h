#ifndef _TRADE_CREATION_H
#define _TRADE_CREATION_H
enum OptionType{
  UNINIT,
  PUT, CALL,
  __OPTION_TYPES,

#define OPTION_MAX __OPTION_TYPES - UNINIT
};


struct Option{
  OptionType type;
  //(thinking reqd) Oil prices went to -ve in 2020 so maybe signed??
  unsigned int strike_price; // = Rupees * 100 + paisa
#if WILL_HANDLE_EXPIRY
#error "UNIMLEMENTED"
  unsigned int expiry_date;  // Range between 1-30
  unsigned int expiry_month; // Range between 1-12
  unsigned int expiry_year;  // 
#endif
};
static int option_amt = 0;

enum TradeType{ BUY, SELL};
enum ContractType{ OPTION, STOCK};

#define Trade_buffer_size 100
struct Trade{
 TradeType    event;
 ContractType contract_type; 
 size_t       quantity;
 unsigned int money_exchange; 
    // Will be the absolute value of the money exchange
    // When calculating value will be multiplied with 
    // -1 if event is BUY and 1 if event is SELL
 
 union{
  Option option;
 };
} trades[Trade_buffer_size];
static int trade_buffer_amt = 0;

//Calculate option intrinsic value at a specific price
inline int OptionPostionValue(Option o, unsigned int current_price);
//get option value in range
void CalculateOptionValueInRange(Option* options, size_t no_of_options, int start_price, int end_price, int * price_output, int* max_payoff, int* max_payoff_price , int* min_payoff, int* min_payoff_price);
//Print option contract into dst
void OptionRepr(char * dst, int size, Option* opt, int index);

//Calculate Trade intrinsic value at a specific price
int TradeValue(Trade trade, unsigned int current_price);
//get trade value in range
void CalculateTradeValueInRange(Trade* trades, size_t no_of_trades, int start_price, int end_price, int* price_output, int* max_payoff, int* max_payoff_price, int *min_payoff, int * min_payoff_price);
//Print trade details into dst
void TradeRepr(char* dst, int size, Trade* trades, int index);

#if __cplusplus
inline int PostionValue(Option o, unsigned int current_price);
inline int PostionValue(Trade t, unsigned int current_price);

inline void CalculateValueInRange(Option* options, size_t no_of_options, int start_price, int end_price, int * price_output, int* max_payoff, int* max_payoff_price , int* min_payoff, int* min_payoff_price);
inline void CalculateValueInRange(Trade* trades, size_t no_of_trades, int start_price, int end_price, int * price_output, int* max_payoff, int* max_payoff_price , int* min_payoff, int* min_payoff_price);

inline void PositionRepr(char * dst, int size, Option* opt, int index);
inline void PositionRepr(char * dst, int size, Trade* trade, int index);
//Calculate Trade intrinsic value at a specific price
#endif //__cplusplus
#endif // _TRADE_CREATION_H

#ifdef TRADE_CREATION_IMPLEMENTATION
inline int OptionPostionValue(Option o, unsigned int current_price){
  if (o.type == CALL){
    if ( current_price < o.strike_price){
      return 0;
    }else{
      return (int) (current_price - o.strike_price);
    }
  }else if(o.type == PUT){
    if ( current_price > o.strike_price){
      return 0;
    }else{
      return (int) (o.strike_price - current_price);
    }
  }else{
    return -1; 
  }
}

void CalculateOptionValueInRange(Option* options, size_t no_of_options, int start_price, int end_price, int * price_output, int* max_payoff, int* max_payoff_price , int* min_payoff, int* min_payoff_price){
  *min_payoff = ( 1 << 31) - 1;
  *max_payoff = -(*min_payoff);
  for(int price = start_price ; price <= end_price; price++){
    int total_payoff = 0;
    for(int opt_idx = 0; opt_idx < no_of_options; opt_idx++){
      int r = OptionPostionValue(options[opt_idx], price);
      total_payoff += r;
    }
    if(total_payoff < *min_payoff) {
      *min_payoff = total_payoff;
      *min_payoff_price = price;
    }
    if(total_payoff > *max_payoff) {
      *max_payoff = total_payoff;
      *max_payoff_price = price;
    }

    price_output[price - start_price] = total_payoff;
  }
}
//price_output should be the same size as that of (1 + end_price - start_price)
void OptionRepr(char * dst, int size, Option* opt, int index){
  dst[size - 1] = 0; 
  switch(opt[index].type){
    case PUT:{
     snprintf(dst, size-1, "PUT of %.2f\n", (double)(opt[index].strike_price/100));
    }break;
    case CALL:{
     snprintf(dst, size-1, "CALL of %.2f\n", (double)(opt[index].strike_price/100));
    }break;
  };
}

Trade CreateTrade(TradeType event, ContractType type, size_t quantity, unsigned int money_exchange, OptionType opt_type = UNINIT, unsigned int strike_price = 0){
  Trade t;
  t.event               = event;
  t.contract_type       = type;
  t.quantity            = quantity;
  t.money_exchange      = money_exchange;
  if(type == OPTION){
    t.option.type         = opt_type;
    t.option.strike_price = strike_price;
  }
  return t;
}

int TradeValue(Trade trade, unsigned int current_price){
  int payoff = 0;
  switch (trade.contract_type){
    case OPTION:{
      payoff += OptionPostionValue(trade.option, current_price);
    }break;
    case STOCK:{
      payoff += current_price;
      //printf("%s:%d := %d\n", __FILE__, __LINE__, payoff);
    }break;
  };

  if (trade.event == BUY)  { payoff =  payoff; payoff -= trade.money_exchange;}
  if (trade.event == SELL) { payoff = -payoff; payoff += trade.money_exchange;}
  return payoff*trade.quantity;
}


void CalculateTradeValueInRange(Trade* trades, size_t no_of_trades, int start_price, int end_price, int* price_output, int* max_payoff, int* max_payoff_price, int *min_payoff, int * min_payoff_price){

  *min_payoff = ( 1 << 31) - 1;
  *max_payoff = -(*min_payoff);
  for(int price = start_price ; price <= end_price; ++price){
    int total_payoff = 0;
    for(int opt_idx = 0; opt_idx < no_of_trades; opt_idx++){
      int r = TradeValue(trades[opt_idx], price);

      //printf("%.2f: %.2f \n", (double)price/100, (double)r/100);
      total_payoff += r;
    }


    if(total_payoff < *min_payoff) {
      *min_payoff = total_payoff;
      *min_payoff_price = price;
    }
    if(total_payoff > *max_payoff) {
      *max_payoff = total_payoff;
      *max_payoff_price = price;
    }
    price_output[price - start_price] = total_payoff;
  }
}


void TradeRepr(char* dst, int size, Trade* trades, int index){
  dst[size-1] = 0; 
  static char *s_BUY    = "BUY";
  static char *s_SELL   = "SELL";
  static char *s_CALL   = "CALL";
  static char *s_PUT    = "PUT";

  char * event;
  char * contract;

  Trade trade = trades[index];
  if(trade.event == BUY)       event = s_BUY;
  else if(trade.event == SELL) event = s_SELL;

  if (trade.contract_type == OPTION){
    if(trade.option.type == CALL) contract = s_CALL;
    else if(trade.option.type == PUT) contract = s_PUT;
    snprintf(dst, size - 1, "%s %s(s) for %.3f at %.2f", 
        event, contract,(double) (trade.option.strike_price/100), (double)(trade.money_exchange/100));
  }else if (trade.contract_type == STOCK){
   snprintf(dst, size - 1, "%s STK for %.2f", event, (double)trade.money_exchange/100); 
  }
}

#if __cplusplus
inline int PostionValue(Option o, unsigned int current_price){ OptionPostionValue(o, current_price); }

inline int PostionValue(Trade t, unsigned int current_price){ TradeValue(t, current_price); }


inline void CalculateValueInRange(Option* options, size_t no_of_options, int start_price, int end_price, int * price_output, int* max_payoff, int* max_payoff_price , int* min_payoff, int* min_payoff_price){
 CalculateOptionValueInRange(options, no_of_options, start_price, end_price, price_output, max_payoff, max_payoff_price, min_payoff, min_payoff_price);
}

inline void CalculateValueInRange(Trade* trades, size_t no_of_trades, int start_price, int end_price, int * price_output, int* max_payoff, int* max_payoff_price , int* min_payoff, int* min_payoff_price){
 CalculateTradeValueInRange(trades, no_of_trades, start_price, end_price, price_output, max_payoff, max_payoff_price, min_payoff, min_payoff_price);
}


inline void PositionRepr(char * dst, int size, Option* opt, int index){
  OptionRepr(dst, size, opt, index);
}
inline void PositionRepr(char * dst, int size, Trade* trade, int index){
  TradeRepr(dst, size, trade, index);
}
#endif //__cplusplus
#endif // TRADE_CREATION_IMPLEMENTATION
