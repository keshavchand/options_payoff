#include <windows.h>
#include <stdio.h>
#define TRADE_CREATION_IMPLEMENTATION
#include "trade_creation.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "./stb_truetype.h"


typedef unsigned int uint;

#define _WIDTH 3840
#define _HEIGHT 2160
//#define WIDTH 100
//#define HEIGHT 100
// Pixels start at (0,0) at top and go to (Height, width) at lowest
uint PIXELS [_WIDTH * _HEIGHT];

struct RenderRegion{
  int    width;
  int    height;
  uint*  pixels;
  stbtt_fontinfo Font;
} region {
  _WIDTH,
  _HEIGHT,
  PIXELS,
  NULL
};


inline void DrawPixel(RenderRegion region, unsigned int x, unsigned int y, unsigned int color){
  if ( y < 0 || y > region.height || x > region.width || x < 0) return;
  region.pixels[y * region.width + x] = color;
}

inline void FillScreen(RenderRegion region, unsigned int color){
  for( int y = 0; y < region.height; y++){
    for( int x = 0; x < region.width; x++){
      DrawPixel(region, x, y, color);
    }
  }
}

void DrawLine(RenderRegion region, int point_x1, int point_y1, int point_x2, int point_y2, unsigned int foreground_color){
  if ( point_y1 == point_y2){
    //DrawStraightLineAlongX
    if ( point_x1 > point_x2){
      int temp = point_x2;
      point_x2 = point_x1;
      point_x1 = temp;
    }

    int point_y = point_y1;
    for(int x = point_x1 ; x <= point_x2 ; x++){
      DrawPixel(region, x, point_y, foreground_color);
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
      DrawPixel(region, point_x, y, foreground_color);
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
      DrawPixel(region, x, y, foreground_color);
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
      DrawPixel(region, x, y, foreground_color);
    }

  }
  
 return; 
}

void DrawLineWide(RenderRegion region, int width , int point_x1, int point_y1, int point_x2, int point_y2, unsigned int color){
  int begin_x1 = point_x1 - (int)( width / 2);
  int begin_x2 = point_x2 - (int)( width / 2);

  for(int i = 0 ; i < width ; i++){
    DrawLine(region, begin_x1 + i, point_y1, begin_x2 + i, point_y2, color);
  }
  for(int i = 0 ; i < width ; i++){
    DrawLine(region, begin_x1, point_y1 + i, begin_x2, point_y2 + i, color);
  }
 
}

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
#if 0
      width, height,
#else
      _WIDTH, _HEIGHT,
#endif
      pixels, &bmi,
      DIB_RGB_COLORS, SRCCOPY);
}

void RenderPayoff(RenderRegion region, int max_payoff,int max_payoff_price, int min_payoff,int min_payoff_price, int *price_output,  int padding_X = 0, int padding_Y = 0){

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
  int loss_color = 0xf16161;
  int profit_color = 0x32cd32;
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
//static stbtt_fontinfo Font;
unsigned char Font_contents[0xfe000];

stbtt_fontinfo STB_font_init(char *font_filename){
  stbtt_fontinfo Font;
    OFSTRUCT file_open_buff;
    BY_HANDLE_FILE_INFORMATION file_info;
    HANDLE TTF_File = CreateFileA(font_filename,GENERIC_READ,  FILE_SHARE_READ,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    GetFileInformationByHandle(TTF_File, &file_info);
    assert(file_info.nFileSizeHigh == 0);
    unsigned long file_size = file_info.nFileSizeLow; 
    assert(file_size < 0xfe000);

    //Read entire file
    unsigned long bytes_read = 0;
    bool result = ReadFile(TTF_File, Font_contents, file_size,(DWORD *) &bytes_read, NULL);
    assert(result);

   stbtt_InitFont(&Font, Font_contents, stbtt_GetFontOffsetForIndex((const unsigned char*) &Font_contents,0));
   return Font;
}

void STB_Font_render_left(RenderRegion region, int line_height, int x_offset, int y_offset, char* text, unsigned int foreground_color, unsigned int background_color){
  struct RGBLerp{
    static const inline int lerp(unsigned int a, unsigned int b, double t){
      unsigned int red_from   = a & 0x00ff0000;
      unsigned int red_to     = b & 0x00ff0000;

      unsigned int green_from = a & 0x0000ff00;
      unsigned int green_to   = b & 0x0000ff00;

      unsigned int blue_from  = a & 0x000000ff;
      unsigned int blue_to    = b & 0x000000ff;

      unsigned int red   = red_from   + (red_to   - red_from)   * t;
      unsigned int green = green_from + (green_to - green_from) * t;
      unsigned int blue  = blue_from  + (blue_to  - blue_from)  * t;

      return red|green|blue;
    }
  };

  stbtt_fontinfo Font = region.Font;
 
  int char_distance = 0;
  float scale = stbtt_ScaleForPixelHeight(&Font, line_height);
  for(int i = 0 ; text[i] != 0; i++){
    int width, height;
    int x_off;
    unsigned char *bitmap = stbtt_GetCodepointBitmap(&Font, 0, scale, text[i], &width, &height, &x_off ,0);

    for(int Y_pos = 0; Y_pos < height; Y_pos++){
      for(int X_pos = 0; X_pos < width; X_pos++){
        //int idx_y = (HEIGHT - 10 - Y_pos)* WIDTH;
        int idx_y = ((height + y_offset)- Y_pos);
        int idx_x = (X_pos + x_offset + char_distance);
        int idx_bmp = Y_pos * width + X_pos;
        //lerp the color between forward and background color
        int color = RGBLerp::lerp(background_color, foreground_color , (double) bitmap[idx_bmp] / 255);
        DrawPixel(region, idx_x, idx_y, color);
      }
    }

    int advance_width, leftSideBearing;
    stbtt_GetCodepointHMetrics(&Font, text[i], &advance_width, &leftSideBearing);
    char_distance += (int)(advance_width * scale);
    char_distance += (int)(stbtt_GetCodepointKernAdvance(&Font, text[i], text[i+1]) * scale);
    stbtt_FreeBitmap(bitmap, 0);
  }
}

constexpr static int getAmt(float a){
  return (int)100*a;
}

//#define STARTPRICE getAmt(80.00)
//#define ENDPRICE getAmt(120.00)

static  int  startprice          =  0;
static  int  endprice            =  0;
static  int  *output_prices      =  0;
static  int  output_prices_size  =  0;
static  int  iteration           =  0;

void RenderSetup(int size){
  if (size > output_prices_size) {
    size = (4*1024 > size) ? 4*1024 : size;
    VirtualFree(output_prices, 0, MEM_RELEASE);
    output_prices = (int*)VirtualAlloc(0, size * sizeof(int), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    output_prices_size = size;
  }
}


void Render(RenderRegion region, int startprice, int endprice, int curr_iter){
#include "position.h"
  RenderSetup(endprice - startprice);
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
  int color = 0x282828;
  FillScreen(region, color);
  RenderPayoff(region, max_payoff, endprice, min_payoff, startprice, output_prices, 100 , 100); //Main Part
  //printf("%s\n", "Font rendering start");
#define PRICE_OUTPUT_SIZE 35
  char range[PRICE_OUTPUT_SIZE];
  range[PRICE_OUTPUT_SIZE - 1] = 0;
  snprintf(range, PRICE_OUTPUT_SIZE - 1, "(%d:%d) max@%d min@%d", startprice, endprice, max_payoff_price, min_payoff_price);
  STB_Font_render_left(region,100,0,0, range, 0xffeeff, color);
#define REPR_SIZE 255
  static char trade_repr[REPR_SIZE];
  //printf("%d\n", option_amt);
  for (int i = 0 ; i < trade_buffer_amt ; i++){
    int line_size = 100;
    TradeRepr((char *)trade_repr, REPR_SIZE, trades, i);
    STB_Font_render_left(region, line_size, 10, region.height - line_size*(i + 1), trade_repr, 0xffeeff, color);
  }
}

int Running = 1;
LRESULT WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam){
  LRESULT result = 0;
  static last_mouse_x;
  static last_mouse_x;
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
         DrawOnScreen(hdc, region ,rect.right - rect.left, rect.bottom - rect.top);
       }
       EndPaint(window_handle, &ps);
    } break;
    case WM_LBUTTONDOWN:{
      last_mouse_x = (lParam & 0x0000ffff) >> 0;
      last_mouse_y = (lParam & 0xffff0000) >> 16;
    } break; 
    case WM_MOUSEMOVE: {
      if(wParam & MK_LBUTTON){
        int new_mouse_x = (lParam & 0x0000ffff) >> 0;
        int new_mouse_y = (lParam & 0xffff0000) >> 16;

        int diff_x = last_mouse_x;
        printf("%d\n", diff_x);
        last_mouse_x = new_mouse_x;
        last_mouse_y = new_mouse_y;

      }
    } break;
    case WM_MOUSEWHEEL:{

      int zdiff = GET_WHEEL_DELTA_WPARAM(wParam);
      zdiff /= 120;
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
        printf("%.2f ; %.2f\n", (double)startprice / 100, (double)endprice / 100);
      } else {
        //Diverge from price
        startprice -= (price - startprice) * factor;
        endprice += (endprice - price) * factor;
        printf("%.2f ; %.2f\n", (double)startprice / 100, (double)endprice / 100);
      } 
      Render(region, startprice, endprice, iteration++);
      PostMessageA(window_handle, WM_PAINT, 0,0);

      printf("zdiff : %i @ ( x: %lld , y: %lld)\n", 
          GET_WHEEL_DELTA_WPARAM(wParam),
          (lParam & 0x0000ffff) >> 0,
          (lParam & 0xffff0000) >> 16);
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

int main(){
  char * window_class_name = "Option Payoff Chart";
  WNDCLASSA window_class = {0};
  window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  window_class.lpfnWndProc = WindowProc;
  window_class.hInstance = GetModuleHandle(0);
  window_class.hCursor = LoadCursor(GetModuleHandle(NULL), IDC_ARROW); 
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
      startprice = getAmt(80.00);
      endprice = getAmt(120.00);
      Render(region, startprice, endprice, iteration++);

      hdc = GetDC(window);
      while (Running){
        MSG message;
        BOOL Ret = PeekMessage(&message, 0,0,0,1);
        if ( Ret > 0){
          TranslateMessage(&message);
          DispatchMessage(&message);
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
}
