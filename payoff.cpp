#include <windows.h>
#include <stdio.h>
#include <string.h>
#define TRADE_CREATION_IMPLEMENTATION
#include "trade_creation.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "./stb_truetype.h"

#include "render.cpp"

#define _WIDTH 3840
#define _HEIGHT 2160
//#define WIDTH 100
//#define HEIGHT 100
// Pixels start at (0,0) at top and go to (Height, width) at lowest
uint PIXELS[_WIDTH * _HEIGHT];
RenderRegion region = {
 _WIDTH , _HEIGHT,
 PIXELS, NULL
};

//static BITMAPINFO bmi;
BITMAPINFO CreateBitmap(RenderRegion region){
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = region.width;
    bmi.bmiHeader.biHeight = region.height; // -ve for top down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = region.width * region.height;

    return bmi;
}

HWND window;
int DrawOnScreen(HDC hdc, RenderRegion region , int screen_width, int screen_height){
  //SetStretchBltMode(hdc, COLORONCOLOR);
  BITMAPINFO bmi = CreateBitmap(region);
  int width = bmi.bmiHeader.biWidth;
  int height = bmi.bmiHeader.biHeight;
  uint* pixels = region.pixels;

  return StretchDIBits(hdc, 
      0,0,
      screen_width, screen_height,
      0,0,
#if 1
      width, height,
#else
      _WIDTH, _HEIGHT,
#endif
      pixels, &bmi,
      DIB_RGB_COLORS, SRCCOPY);
}

static int loss_color = 0xf16161;
static int profit_color = 0x32cd32;
void RenderPayoff(RenderRegion region, int max_payoff,int max_payoff_price, int min_payoff,int min_payoff_price, int *price_output,  int padding_X = 100, int padding_Y = 100){

  //For Width adjustment : X axis
  int shifted_max_price     = max_payoff_price - min_payoff_price;
  int price_range_len       = shifted_max_price; 
  //max_price * streth      = Width
  float stretch_ratio_price = float(region.width - 2 * padding_X) / float(shifted_max_price);

  //For Height adjustment : Y axis
  int shifted_max_payoff    = max_payoff - min_payoff;
  shifted_max_payoff        += 2 * padding_Y;
  int payoff_range_len      = shifted_max_payoff; 
  //max_payoff * streth      = HEIGHT
  float stretch_ratio_payoff = float(region.height - 2 * padding_Y) / float(shifted_max_payoff);

  int no_of_prices = max_payoff_price - min_payoff_price;

  //Iterate[0..no_of_prices - 1] and draw a line from[i to i + 1];
  for (int i = 0 ; i < no_of_prices - 1; i++){
    int x_pos_start = (int) (i    ) * stretch_ratio_price + padding_X;
    int x_pos_end   = (int) (i + 1) * stretch_ratio_price + padding_X;
    int y_pos_start = (int) (price_output[i]     - min_payoff) * stretch_ratio_payoff + padding_Y;
    int y_pos_end   = (int) (price_output[i + 1] - min_payoff) * stretch_ratio_payoff + padding_Y;
    if( price_output[i + 1] > 0){
      DrawLineWide(region, 10, x_pos_start, y_pos_start, x_pos_end, y_pos_end, profit_color );
    } else{
      DrawLineWide(region, 10, x_pos_start, y_pos_start, x_pos_end, y_pos_end, loss_color );

    }
  }

}

//Load Trades hot reload
typedef void (* InitSampleTrades)(Trade *trades, int* trade_buffer_amt, int trade_buffer_size);
static struct TradePositionFileHotLoading{
  HMODULE          library;
  FILETIME         last_file_time;
  InitSampleTrades init_sample_trades;
} trade_file_hotloading;

static bool isNew(FILETIME maybe_new, FILETIME old){
  if ( maybe_new.dwHighDateTime > old.dwHighDateTime) return true;
  else if (maybe_new.dwHighDateTime == old.dwHighDateTime) {
    if(maybe_new.dwLowDateTime > old.dwLowDateTime){
      return true;
    }
  }
  return false;
}

bool LoadTrades(char* filename){
  //Get last write time
  bool new_trades = false;
  FILETIME last_time = trade_file_hotloading.last_file_time;
  HANDLE file = CreateFileA( filename, GENERIC_READ , FILE_SHARE_WRITE, NULL, 
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  FILETIME new_file_time;
  GetFileTime(file, NULL, NULL, &new_file_time);
  CloseHandle(file);

  //If changed then load again
  if(isNew(new_file_time, last_time)) {
    new_trades = true;
    printf("NEW TRADES\n");
    if(trade_file_hotloading.library) FreeLibrary(trade_file_hotloading.library);

    //Cant overide file on windows
    DeleteFile("new_required.dll");
    CopyFile(filename, "new_required.dll", 1);

    trade_file_hotloading.last_file_time = new_file_time;

    trade_file_hotloading.library = LoadLibraryA("new_required.dll");
    assert(trade_file_hotloading.library != NULL);
    trade_file_hotloading.init_sample_trades = 
     (InitSampleTrades) GetProcAddress(trade_file_hotloading.library , "InitSampleTrades"); 
  }

  return new_trades;
}
//End load trades

constexpr static int getAmt(float a){
  return (int)100*a;
}
constexpr static double getAmt(int a){
  return (double)a/100;
}

//#define STARTPRICE getAmt(80.00)
//#define ENDPRICE getAmt(120.00)

static  int  startprice          =  0;
static  int  endprice            =  0;
static  int  *output_prices      =  0;
static  int  output_prices_size  =  0;
static  int  iteration           =  0;
static  int  line_size           =  0;
static  int  screen_mouse_x      =  0;
static  int  screen_mouse_y      =  0;
static  int  screen_width        =  0;
static  int  screen_height       =  0;
static  int  padding_x           =  100;
static  int  padding_y           =  100;


long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void RenderSetup(int size){
  if (!line_size) line_size = 100;
  if (size > output_prices_size) {
    size = (4*1024 > size) ? 4*1024 : size;
    VirtualFree(output_prices, 0, MEM_RELEASE);
    output_prices = (int*)VirtualAlloc(0, size * sizeof(int), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    output_prices_size = size;
  }
}


bool Render(RenderRegion region, int startprice, int endprice, int curr_iter){
  RenderSetup(endprice - startprice);
  bool are_new_trades = LoadTrades("position.dll");
  if (are_new_trades) iteration++;
  trade_file_hotloading.init_sample_trades(trades, &trade_buffer_amt, Trade_buffer_size);

  //NOTE idk about allocating data rn
  //maybe will do it later
  //currently a global variable
  //int* output_prices = (int *)_alloca(1 + endprice - startprice);
  static int  max_payoff;
  static int  max_payoff_price;
  static int  min_payoff;
  static int  min_payoff_price;

  stbtt_fontinfo Font = region.Font;

  if ( curr_iter != iteration){
  CalculateTradeValueInRange(trades, trade_buffer_amt, startprice, endprice, output_prices, &max_payoff, &max_payoff_price, &min_payoff, &min_payoff_price);
  //iteration = curr_iter;
  }

  //Rendering part
  int bg_color = 0x282828;
  FillScreen(region, bg_color);


  //Print payoff graph
  RenderPayoff(region, max_payoff, endprice, min_payoff, startprice, output_prices, padding_x , padding_y); //Main Part

  //Print MIN MAX
#define PRICE_OUTPUT_SIZE 40
  char range[PRICE_OUTPUT_SIZE];
  range[PRICE_OUTPUT_SIZE - 1] = 0;
  snprintf(range, PRICE_OUTPUT_SIZE - 1, "(%.2f:%.2f) MAX@%.2f MIN@%.2f", getAmt(startprice), getAmt(endprice), getAmt(max_payoff_price), getAmt(min_payoff_price));
  STB_Font_render_left(region,line_size * 0.9 ,0,0, range, 0xffeeff, bg_color);
  
  //GetPrice under cursor
  //map current pixel count to screen
  //find position of cursor on pixel wrt screen
  //printf("%d of %d\n", screen_mouse_x, region.width - padding_x)
  
  int price_under_mouse = 0;
  char price_under_mouse_str[25];
  price_under_mouse_str[24] = 0;
  //Note: incase the screen isnt created
  if (screen_width){
    int mouse_pixel_x = map(screen_mouse_x, 0, screen_width, 0, region.width);
    if (!(mouse_pixel_x < padding_x || mouse_pixel_x > (region.width - padding_x)))
    price_under_mouse = map(mouse_pixel_x, 0, region.width, startprice, endprice);
    snprintf(price_under_mouse_str, 24, "%.02f", getAmt(price_under_mouse));
  }

  printf("%d \n", price_under_mouse);


  //Print Trades
  //if loss then loss color 
  //if profit then profit color
#define REPR_SIZE 255
  static char trade_repr[REPR_SIZE];
  for (int i = 0 ; i < trade_buffer_amt ; i++){
    TradeRepr((char *)trade_repr, REPR_SIZE, trades, i);
#if 1
    int trade_payoff = TradeValue(trades[i], price_under_mouse);
    if ( trade_payoff > 0)
      STB_Font_render_left(region, line_size, 10, region.height - line_size*(i + 1), trade_repr, profit_color, bg_color);
    else
      STB_Font_render_left(region, line_size, 10, region.height - line_size*(i + 1), trade_repr, loss_color, bg_color);
#else
    STB_Font_render_left(region, line_size, 10, region.height - line_size*(i + 1), "HELLO WORLD", 0xffeeff, bg_color);
#endif
  }
  
  if (price_under_mouse)
    STB_Font_render_right(region, line_size, region.width, region.height - line_size, price_under_mouse_str, 0xffeeff, bg_color);
  return are_new_trades;

}

int Running     = 1;
int Deactivated = 0;
LRESULT WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam){
  LRESULT result = 0;
  static int last_mouse_x;
  static int last_mouse_y;

  //printf("MSG: %x\n", message);
  switch (message){
    case WM_CLOSE: {
      Running = 0;
    } break;
    case WM_PAINT: {
       PAINTSTRUCT ps;
       HDC hdc = BeginPaint(window_handle, &ps);
       {
         RECT rect;
         GetClientRect(window_handle, &rect);
         screen_width  = rect.right - rect.left;
         screen_height = rect.bottom - rect.top;
         DrawOnScreen(hdc, region ,rect.right - rect.left, rect.bottom - rect.top);
       }
       EndPaint(window_handle, &ps);
    } break;
    case WM_LBUTTONDOWN:{
      if(!(~wParam & (MK_LBUTTON | MK_CONTROL))){
      last_mouse_x = (lParam & 0x0000ffff) >> 0;
      last_mouse_y = (lParam & 0xffff0000) >> 16;
      }
    } break; 
    case WM_MOUSEMOVE: {
      // iff mouse left button is down
      // and control key is pressed
      if(!(~wParam & (MK_LBUTTON | MK_CONTROL))){
        int new_mouse_x = (lParam & 0x0000ffff) >> 0;
        int new_mouse_y = (lParam & 0xffff0000) >> 16;

        RECT rect;
        GetWindowRect(window_handle, &rect);
        int window_width = rect.right - rect.left;
        int mouse_x = (lParam & 0x0000ffff) >> 0;
        float last_mouse_x_adj = (float)last_mouse_x;

        mouse_x -= rect.left;
        float mouse_x_adj = (float)mouse_x / (float)window_width;
        last_mouse_x_adj -= (float)rect.left;
        last_mouse_x_adj /= window_width;

        int price_under_last_mouse = startprice + last_mouse_x_adj*(endprice - startprice);
        int price_under_new_mouse = startprice + mouse_x_adj*(endprice - startprice);

        int price_diff = price_under_new_mouse - price_under_last_mouse;
        //printf("%d\n", price_diff);

        startprice -= price_diff;
        endprice -= price_diff;
        startprice = (startprice < 0)? 0 : startprice;
  
        last_mouse_x = new_mouse_x;
        last_mouse_y = new_mouse_y;

#if 0
        Render(region, startprice, endprice, iteration++);
        {
         HDC hdc = GetDC(window_handle);
         RECT rect;
         GetClientRect(window_handle, &rect);
         DrawOnScreen(hdc, region ,rect.right - rect.left, rect.bottom - rect.top);
         ReleaseDC(window_handle, hdc);
        }
#endif
        //printf("%f\n", diff_sec);
      }

        screen_mouse_x = (lParam & 0x0000ffff) >> 0;
        screen_mouse_y = (lParam & 0xffff0000) >> 16;
        
        Render(region, startprice, endprice, iteration++);
        {
         HDC hdc = GetDC(window_handle);
         RECT rect;
         GetClientRect(window_handle, &rect);
         DrawOnScreen(hdc, region ,rect.right - rect.left, rect.bottom - rect.top);
         ReleaseDC(window_handle, hdc);
        }

    } break;
    case WM_MOUSEWHEEL:{
      int zdiff = GET_WHEEL_DELTA_WPARAM(wParam);
      zdiff /= 120;
      if(wParam & MK_CONTROL){
        line_size += zdiff;   
        //printf("%d\n", line_size);
      }else{
        float factor = 0.20; 

        RECT rect;
        GetWindowRect(window_handle, &rect);
        int window_width = rect.right - rect.left;
        int mouse_x = (lParam & 0x0000ffff) >> 0;
        mouse_x -= rect.left;

        int price = startprice + (endprice - startprice)*((float)mouse_x / (float)window_width);
        //Converge to price
        if (zdiff > 0){
          startprice += (price - startprice) * factor;
          endprice -= (endprice - price) * factor;
          startprice = (startprice < 0)? 0 : startprice;
          //printf("%.2f ; %.2f\n", (double)startprice / 100, (double)endprice / 100);
        } else {
          //Diverge from price
          startprice -= (price - startprice) * factor;
          endprice += (endprice - price) * factor;
          startprice = (startprice < 0)? 0 : startprice;
          //printf("%.2f ; %.2f\n", (double)startprice / 100, (double)endprice / 100);
        } 
        Render(region, startprice, endprice, iteration++);
      }
        {
          HDC hdc = GetDC(window_handle);
          RECT rect;
          GetClientRect(window_handle, &rect);
          DrawOnScreen(hdc, region ,rect.right - rect.left, rect.bottom - rect.top);
          ReleaseDC(window_handle, hdc);
        }

    } break;
    case WM_MOUSEACTIVATE:
    case WM_ACTIVATE:{
      if (wParam){ Deactivated = 0; }
      else if(!wParam){ Deactivated = 1; }
#if 0
      {
        HDC hdc = GetDC(window_handle);
        RECT rect;
        GetClientRect(window_handle, &rect);
        DrawOnScreen(hdc, region ,rect.right - rect.left, rect.bottom - rect.top);
        ReleaseDC(window_handle, hdc);
      }
#endif
    } break; 
    case WM_GETICON:{
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

DWORD WINAPI ThreadProc( LPVOID lpParameter){
 HWND workingWindow = (HWND) lpParameter;
 for(;;){
    Sleep(1000);
#if 0
    BOOL isHung = IsHungAppWindow(workingWindow);
    if (isHung) printf("WINDOW HUNG\n");
    else printf("Working\n");

    printf("Deactivated: %d\n", Deactivated);
#endif
 }
}

int main(){
  char * window_class_name = "Option Payoff Chart";
  WNDCLASSA window_class = {0};
  window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  window_class.lpfnWndProc = WindowProc;
  window_class.hCursor  = LoadCursor(0, IDC_ARROW);
  window_class.hInstance = GetModuleHandle(0);
  window_class.lpszClassName = window_class_name;
  HDC hdc;

  //Font face
  region.Font = STB_font_init("C:/Windows/Fonts/arial.ttf");

  if(RegisterClass(&window_class)){
    window = CreateWindowExA(0,
           window_class_name, 
           "Option Payoff Chart", 
           WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
           CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT, 
           0, 0, GetModuleHandle(0), 0);

    if (window) {
      CreateThread(NULL, 0, ThreadProc, (LPVOID*) window, 0, NULL);
      startprice = getAmt(80.00f);
      endprice = getAmt(120.00f);
      Render(region, startprice, endprice, iteration++);

      hdc = GetDC(window);
      int pm_count = 0;
      SetTimer(window, NULL, 100, 0);
      while (Running){
        MSG message;
        //System cant go idle when using peek message
        BOOL Ret;
#if 0
        // We use GetMessage if Deactived
        if (Deactivated) Ret = GetMessage(&message, 0, 0, 0);
        else             Ret = PeekMessage(&message, 0,0,0,1);
#else
        //Else we rely on SetTimer
        Ret = PeekMessage(&message, 0,0,0,1);
        pm_count ++;
#endif
        if ( Ret > 0){
          HWND window_handle = window;
          UINT msg = message.message;
          WPARAM wParam = message.wParam; 
          LPARAM lParam = message.lParam; 
          WindowProc(window_handle, msg, wParam, lParam);
#if 0
          TranslateMessage(&message);
          DispatchMessage(&message);
#endif
          continue;
        }
        Render(region, startprice, endprice, iteration);
        RECT rect;
        GetClientRect(window, &rect);
        DrawOnScreen(hdc, region, rect.right - rect.left, rect.bottom - rect.top);
        ShowWindow(window, SW_SHOW);
      }
    }
  }
  return 0;
}
