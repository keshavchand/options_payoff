#include <stdio.h>
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
  unsigned int expiry_date;  // Range between 1-30
  unsigned int expiry_month; // Range between 1-12
  unsigned int expiry_year;  // 
};

unsigned int OptionIntrinsicValue(Option o, unsigned int current_price){
  if (o.type == CALL){
    if ( current_price < o.strike_price){
      return 0;
    }else{
      return current_price - o.strike_price;
    }
  }else if(o.type == PUT){
    if ( current_price > o.strike_price){
      return 0;
    }else{
      return o.strike_price - current_price;
    }
  }else{
    return -1; 
  }
}
//TODO output to a ppm file ??
int main(){
  Option opt[255];
  for(int i = 0 ; i < 255; i++){
    Option *o = &opt[i];
    o->type = PUT;
    o->strike_price = i * 100 + i + 10;
    o->expiry_date  = i % 30 + 1;
    o->expiry_month = i % 12 + 1;
    o->expiry_year  = 2021;
  }

  for(int i = 0 ; i < 20; i++){
    printf("%u %u %u %u := %u\n", 
        opt[i].strike_price, opt[i].expiry_date,
        opt[i].expiry_month,opt[i].expiry_year,
        OptionIntrinsicValue(opt[i], 500));
  }


};
