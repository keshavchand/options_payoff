#include <windows.h>
#include <stdio.h>
#include <assert.h>

enum OptionType{
  UNINIT,
  PUT, CALL,
  __OPTION_TYPES,

#define OPTION_MAX __OPTION_TYPES - UNINIT
};

typedef struct {
  OptionType type;
  //(thinking reqd) Oil prices went to -ve in 2020 so maybe signed??
  unsigned int strike_price; // = Rupees * 100 + paisa
  unsigned int expiry_date;  // Range between 1-30
  unsigned int expiry_month; // Range between 1-12
  unsigned int expiry_year;  // 
}Option ;

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

//#define WIDTH 3840
//#define HEIGHT 2160
#define WIDTH 800
#define HEIGHT 600
// Pixels start at (0,0) at top and go to (Height, width) at lowest
unsigned int PIXELS [WIDTH * HEIGHT];


void FillScreen(unsigned int * pixels, int color){
  for( int i = 0; i < WIDTH; i++){
    for( int j = 0; j < HEIGHT; j++){
//      pixels[i * HEIGHT + j] = (0x00 + offset)&0xff;
//      pixels[i * HEIGHT + j] |= ((0x00 + offset)&0xff) << 8;
    pixels[i * HEIGHT + j] = color;
    }
  }

}

void DrawLine(unsigned int *pixels, int point_x1, int point_y1, int point_x2, int point_y2, unsigned int foreground_color){
  if ( point_y1 == point_y2){
    //DrawStraightLineAlongX
    if ( point_x1 > point_x2){
      int temp = point_x2;
      point_x2 = point_x1;
      point_x1 = temp;
    }

    int point_y = point_y1;
    for(int x = point_x1 ; x <= point_x2 ; x++){
      int idx = point_y * WIDTH + x;
      if( idx < 0 || idx > WIDTH * HEIGHT) continue;
      pixels[idx] = foreground_color;
    }
    return;
  }
  if ( point_x1 == point_x2){
    //DrawStraightLineAlongY
    if ( point_y1 > point_y2){
      int temp = point_y2;
      point_y2 = point_y1;
      point_y1 = temp;
    }

    int point_x = point_x1;
    for(int y = point_y1 ; y <= point_y2; y++){
      int idx = y * WIDTH + point_x;
      if( idx < 0 || idx > WIDTH * HEIGHT) continue;
      pixels[idx] = foreground_color;
    }
    return;
  }

  int dx = point_x1 - point_x2;
  if (dx < 0) dx *= -1;

  int dy = point_y1 - point_y2;
  if (dy < 0) dy *= -1;

  //If we have more dx then we draw along x axis using y = mx + c
  //else we draw along y axis using x = y/m - d where d = c/m
  if( dx > dy){
    if (point_x1 > point_x2){
      int temp = point_x2;
      point_x2 = point_x1;
      point_x1 = temp;

      temp = point_y2;
      point_y2 = point_y1;
      point_y1 = temp;
    }

    //y = Mx + c
    //M = slope
    //c = y intersect
    float first_pixel_center_x = (float)point_x1 + 0.5f;
    float first_pixel_center_y = (float)point_y1 + 0.5f;

    float second_pixel_center_x = (float)point_x2 + 0.5f;
    float second_pixel_center_y = (float)point_y2 + 0.5f;

    float slope = 
      (second_pixel_center_y - first_pixel_center_y)/(second_pixel_center_x - first_pixel_center_x);
    float y_intersect = first_pixel_center_y - (slope * first_pixel_center_x);

    for(int x = point_x1; x < point_x2; x++){
      float x_pos = (float)x + 0.5f;
      float y_pos = (slope * x_pos) + y_intersect;
      int y = (int) (y_pos);
      int idx = (y) * WIDTH + x;
      if( idx < 0 || idx > WIDTH * HEIGHT) continue;
      pixels[(y) * WIDTH + x] = foreground_color ;
    }
  }else{
    if (point_y1 > point_y2){
      int temp = point_y2;
      point_y2 = point_y1;
      point_y1 = temp;

      temp = point_x2;
      point_x2 = point_x1;
      point_x1 = temp;
    }

    //x = y*(1/m) - c*(1/m)
    //M = slope
    //c = y intersect
    float first_pixel_center_x = (float)point_x1 + 0.5f;
    float first_pixel_center_y = (float)point_y1 + 0.5f;

    float second_pixel_center_x = (float)point_x2 + 0.5f;
    float second_pixel_center_y = (float)point_y2 + 0.5f;

    float slope_inverse = 
      (second_pixel_center_x - first_pixel_center_x)/(second_pixel_center_y - first_pixel_center_y);
    // c*(1/m)
    //THINK: maybe rename is better
    float y_intersect = first_pixel_center_y * slope_inverse - (first_pixel_center_x);

    for(int y = point_y1; y < point_y2; y++){
      float y_pos = (float)y + 0.5;
      float x_pos = (slope_inverse * y_pos) - y_intersect;
      int x = (int) (x_pos);
      int idx = (y) * WIDTH + x;
      if( idx < 0 || idx > WIDTH * HEIGHT) continue;
      pixels[(y) * WIDTH + x] = foreground_color ;
    }

  }
  
 return; 
}

void DrawLineWide(unsigned int* pixels, int width , int point_x1, int point_y1, int point_x2, int point_y2, unsigned int color){
  int begin_x1 = point_x1 - (int)( width / 2);
  int begin_x2 = point_x2 - (int)( width / 2);

  for(int i = 0 ; i < width ; i++){
    DrawLine(pixels, begin_x1 + i, point_y1, begin_x2 + i, point_y2, color);
  }
 
}

static BITMAPINFO bmi;
HWND window;
int DrawOnScreen(HDC hdc, int screen_width, int screen_height){
  if(bmi.bmiHeader.biSize == 0){
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WIDTH;
    bmi.bmiHeader.biHeight = HEIGHT; // -ve for top down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = HEIGHT * WIDTH;
  }
  SetStretchBltMode(hdc, COLORONCOLOR);
  return StretchDIBits(hdc, 
      0,0,
      screen_width, screen_height,
      0,0,
      WIDTH, HEIGHT,
      PIXELS, &bmi,
      DIB_RGB_COLORS, SRCCOPY);
}


//price_output should be the same size as that of (1 + end_price - start_price)
void CalculateOptionPayoff(Option* options, size_t no_of_options, int start_price, int end_price, int * price_output, int* max_payoff, int* max_payoff_price , int* min_payoff, int* min_payoff_price){
  *min_payoff = ( 1 << 31) - 1;
  *max_payoff = -(*min_payoff);
  for(int price = start_price ; price <= end_price; price++){
    int total_payoff = 0;
    for(int opt_idx = 0; opt_idx < no_of_options; opt_idx++){
      int r = OptionIntrinsicValue(options[opt_idx], price);
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

int i = 0x1b;
void RenderPayoff(unsigned int* pixels, int max_payoff,int max_payoff_price, int min_payoff,int min_payoff_price, int *price_output, int start_price, int end_price){

  //For Width adjustment : X axis
  int shifted_max_price     = max_payoff_price - min_payoff_price;
  int price_range_len       = shifted_max_price; 
  //max_price * streth      = Width
  float stretch_ratio_price = float(WIDTH) / float(shifted_max_price);

  //For Height adjustment : Y axis
  int shifted_max_payoff    = max_payoff - min_payoff;
  int payoff_range_len      = shifted_max_payoff; 
  //max_payoff * streth      = HEIGHT
  float stretch_ratio_payoff = float(HEIGHT) / float(shifted_max_payoff);

  int no_of_prices = end_price - start_price;
  //Iterate[0..no_of_prices - 1] and draw a line from[i to i + 1];
  
  //Debug Stuff
  char s[256];
  s[255] = 0;

  int last_point_x = (int) (0    ) * stretch_ratio_price;
  int last_point_y = (int) (price_output[0] - min_payoff) * stretch_ratio_payoff;
  //Debug Stuff end

  for (int i = 0 ; i < no_of_prices - 1; i++){
    int x_pos_start = (int) (i    ) * stretch_ratio_price;
    int x_pos_end   = (int) (i + 1) * stretch_ratio_price;
    int y_pos_start = (int) (price_output[i] - min_payoff) * stretch_ratio_payoff;
    int y_pos_end   = (int) (price_output[i + 1] - min_payoff) * stretch_ratio_payoff;

    assert(x_pos_start < WIDTH);
    assert(x_pos_end < WIDTH);
    assert(y_pos_start < HEIGHT);
    assert(y_pos_end < HEIGHT);
    //Debug stuff
#if 0
    if (x_pos_start < last_point_x || y_pos_start < last_point_y){
      snprintf(s, 255, "%d %d to %d %d\n", x_pos_start, y_pos_start, x_pos_end, y_pos_end);
      OutputDebugStringA(s);
    }        
    last_point_x = x_pos_start;
    last_point_y = y_pos_start;
#endif
    //Debug stuff end

    //if ( i > 800) exit(0);
    //printf("%d:%d -> %d:%d\n", x_pos_start, y_pos_start, x_pos_end, y_pos_end);
    DrawLine(pixels, x_pos_start, y_pos_start, x_pos_end, y_pos_end, 0xffeeff);

  }

}

int Running = 1;
int color = 0x808080;
LRESULT WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam){
  LRESULT result = 0;
  switch (message){
    case WM_CLOSE: {
      Running = 0;
    } break;
    case WM_PAINT: {
       PAINTSTRUCT ps;
#if 0
       FillScreen((unsigned int *) &PIXELS, color);
#else

#endif
       HDC hdc = BeginPaint(window_handle, &ps);
       {
         RECT rect;
         GetClientRect(window_handle, &rect);
         DrawOnScreen(hdc, rect.right - rect.left, rect.bottom - rect.top);
       }
       EndPaint(window_handle, &ps);
    } break;
    case WM_KEYDOWN:{
      if ( wParam == 'Q'){
        Running = 0;
      }
    } break;
    default: {
      result = DefWindowProc(window_handle, message, wParam, lParam);
    } break;
  }

  return result;
}

int debug_point_x2 = 0;
int debug_point_y2 = 0;
int main(){

  Option opt[10];
  for(int i = 100 ; i < 100 + 10; i++){
    Option *o = &opt[i - 100];
    o->type = CALL;
    o->strike_price = i * 100 + i + 10;
    o->expiry_date  = i % 30 + 1;
    o->expiry_month = i % 12 + 1;
    o->expiry_year  = 2021;
  }
// 10000 = 100.00
#define STARTPRICE 10000 
// 10000 = 100.00
#define ENDPRICE 30000 

  int output_prices[1 + ENDPRICE - STARTPRICE];
  int max_payoff;
  int max_payoff_price;
  int min_payoff;
  int min_payoff_price;
  CalculateOptionPayoff(opt, 10, STARTPRICE, ENDPRICE, output_prices, &max_payoff,&max_payoff_price, &min_payoff, &min_payoff_price);

  //for(int i = 0 ; i < 20; i++){
  //  printf("%u %u %u %u := %u\n", 
  //      opt[i].strike_price, opt[i].expiry_date,
  //      opt[i].expiry_month,opt[i].expiry_year,
  //      OptionIntrinsicValue(opt[i], 500));
  //}


  char * window_class_name = "Option Payoff Chart";
  WNDCLASSA window_class = {0};
  window_class.style = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = WindowProc;
  window_class.hInstance = GetModuleHandle(0);
  window_class.lpszClassName = window_class_name;

  if(RegisterClass(&window_class)){
    window = CreateWindowExA(0,
           window_class_name, 
           "Option Payoff Chart", 
           WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
           CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT, 
           0, 0, GetModuleHandle(0), 0);

    int a = 0x1a;
    if (window) {
      while (Running){
        MSG message;
        BOOL Ret = PeekMessage(&message, 0,0,0,1);
        if ( Ret > 0){
          TranslateMessage(&message);
          DispatchMessage(&message);
          continue;
        }
        FillScreen((unsigned int *) &PIXELS, color);
#if 0
        DrawLine((unsigned int* )&PIXELS, a++, 0, a, 1,0xffeeff);
#else
        RenderPayoff(PIXELS, max_payoff , ENDPRICE, min_payoff , STARTPRICE, output_prices, STARTPRICE, ENDPRICE);
#endif

         //Test codstatice just remove it {
         //DrawLineWide((unsigned int*) PIXELS, 5, WIDTH/2 , HEIGHT/2, debug_point_x2, debug_point_y2, 0x000000);
         //debug_point_x2++;
         //if (debug_point_x2 > WIDTH){
         // debug_point_x2 = 0;
         // debug_point_y2 = (debug_point_y2 + 1) % HEIGHT;
         // printf("%d\n", debug_point_y2);
         //}

         HDC hdc = GetDC(window);
         {
           RECT rect;
           GetClientRect(window, &rect);
           DrawOnScreen(hdc, rect.right - rect.left, rect.bottom - rect.top);
         }
         ReleaseDC(window, hdc);
        ShowWindow(window, SW_SHOW);
      }
    }
  }
}
