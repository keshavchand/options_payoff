#include <windows.h>
#include <stdio.h>

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

#define WIDTH 800
#define HEIGHT 600
unsigned int PIXELS [WIDTH * HEIGHT];
static BITMAPINFO bmi;

void GenerateTestScreen(unsigned int * pixels, int offset){
  for( int i = 0; i < WIDTH; i++){
    for( int j = 0; j < HEIGHT; j++){
      pixels[i * HEIGHT + j] = 0xff0000;
      pixels[i * HEIGHT + j] += (offset & 0xff) << 8;
    }
  }
}
 
int DrawPixelsOnScreen(HDC hdc, int screen_width, int screen_height){
  if(bmi.bmiHeader.biSize == 0){
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WIDTH;
    bmi.bmiHeader.biHeight = -HEIGHT; // -ve for top down
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

int Running = 1;
int offset = 0;
LRESULT WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam){
  GenerateTestScreen((unsigned int *) &PIXELS, offset++);
  HDC hdc = GetDC(window_handle);
  {
    RECT rect;
    GetClientRect(window_handle, &rect);
    int i = DrawPixelsOnScreen(hdc, rect.right - rect.left, rect.bottom - rect.top);
    //printf("asdf %d \n", i == GDI_ERROR);
  }
  ReleaseDC(window_handle, hdc);
  LRESULT result = 0;
  switch (message){
    case WM_CLOSE: {
      Running = 0;
    } break;
    case WM_PAINT: {
       PAINTSTRUCT ps;
       GenerateTestScreen((unsigned int *) &PIXELS, 0);
       HDC hdc = BeginPaint(window_handle, &ps);
       RECT rect;
       GetClientRect(window_handle, &rect);
       DrawPixelsOnScreen(hdc, rect.right - rect.left, rect.bottom - rect.top);
       EndPaint(window_handle, &ps);
    } break;
    default: {
      result = DefWindowProc(window_handle, message, wParam, lParam);
    } break;
  }

  return result;
}

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


  char * window_class_name = "Option Payoff Chart";
  WNDCLASSA window_class = {0};
  window_class.style = CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = WindowProc;
  window_class.hInstance = GetModuleHandle(0);
  window_class.lpszClassName = window_class_name;

  if(RegisterClass(&window_class)){
    HWND window = CreateWindowExA(0,
           window_class_name, 
           "Option Payoff Chart", 
           WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
           CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT, 
           0, 0, GetModuleHandle(0), 0);

    if (window) {
      while (Running){
        MSG message;
        BOOL Ret = PeekMessage(&message, 0,0,0,1);
        if ( Ret > 0){
          TranslateMessage(&message);
          DispatchMessage(&message);
          continue;
        }
         GenerateTestScreen((unsigned int *) &PIXELS, offset++);
         HDC hdc = GetDC(window);
         {
           RECT rect;
           GetClientRect(window, &rect);
           int i = DrawPixelsOnScreen(hdc, rect.right - rect.left, rect.bottom - rect.top);
           //printf("asdf %d \n", i == GDI_ERROR);
         }
         ReleaseDC(window, hdc);
        ShowWindow(window, SW_SHOW);
      }
    }
  }
};
