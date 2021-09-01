#include <windows.h>
#include <stdio.h>
#define TRADE_CREATION_IMPLEMENTATION
#include "trade_creation.h"

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
} region {
  _WIDTH,
  _HEIGHT,
  PIXELS  
};


inline void DrawPixel(RenderRegion region, unsigned int x, unsigned int y, unsigned int color){
  if ( y < 0 || y > region.height || x > region.width || x < 0) return;
  region.pixels[y * region.width + x] = color;
}

inline void FillScreen(RenderRegion region, unsigned int color){
  //printf("%d x %d\n", region.width, region.height);
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
  uint* pixels = PIXELS;

  printf("%d x %d\n", width , height);
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

#define STB_TRUETYPE_IMPLEMENTATION
#include "./stb_truetype.h"

static stbtt_fontinfo Font;
unsigned char Font_contents[0xfe000];
void STB_Font_render(RenderRegion region, int line_height, int x_offset, int y_offset, char* text, char *font_filename, unsigned int foreground_color, unsigned int background_color){
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
  if(!Font.userdata){
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
  }
 
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


int Running = 1;
LRESULT WindowProc(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam){
  LRESULT result = 0;
  switch (message){
    case WM_CLOSE: {
      Running = 0;
    } break;
    case WM_PAINT: {
#if 0
       return 1;
#else
       PAINTSTRUCT ps;
       HDC hdc = BeginPaint(window_handle, &ps);
       {
         RECT rect;
         GetClientRect(window_handle, &rect);
         DrawOnScreen(hdc, region ,rect.right - rect.left, rect.bottom - rect.top);
       }
       EndPaint(window_handle, &ps);
#endif
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

  struct GetAmt{
    constexpr static int getAmt(float a){
      return (int)100*a;
    }
  };


#include "position.h"
#define STARTPRICE GetAmt::getAmt(80.00)
#define ENDPRICE GetAmt::getAmt(120.00)

  int output_prices[1 + ENDPRICE - STARTPRICE];
  int max_payoff;
  int max_payoff_price;
  int min_payoff;
  int min_payoff_price;

  CalculateTradeValueInRange(trades, trade_buffer_amt, STARTPRICE, ENDPRICE, output_prices, &max_payoff, &max_payoff_price, &min_payoff, &min_payoff_price);

  printf("%d %d", max_payoff, min_payoff);
  char * window_class_name = "Option Payoff Chart";
  WNDCLASSA window_class = {0};
  window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  window_class.lpfnWndProc = WindowProc;
  window_class.hInstance = GetModuleHandle(0);
  window_class.hCursor = LoadCursor(GetModuleHandle(NULL), IDC_ARROW); 
  window_class.lpszClassName = window_class_name;

  HDC hdc;

  if(RegisterClass(&window_class)){
    window = CreateWindowExA(0,
           window_class_name, 
           "Option Payoff Chart", 
           WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
           CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT, 
           0, 0, GetModuleHandle(0), 0);

    if (window) {
      hdc = GetDC(window);
      while (Running){
        MSG message;
        BOOL Ret = PeekMessage(&message, 0,0,0,1);
        if ( Ret > 0){
          TranslateMessage(&message);
          DispatchMessage(&message);
          continue;
        }
        int color = 0x282828;
        FillScreen(region, color);
        RenderPayoff(region, max_payoff , ENDPRICE, min_payoff , STARTPRICE, output_prices, 100 , 100); //Main Part
        printf("%s\n", "Font rendering start");
        STB_Font_render(region,200,0,0, "TRADE PAYOFF", "C:/Windows/Fonts/arial.ttf", 0xffeeff, color);
#define REPR_SIZE 255
        static char trade_repr[REPR_SIZE];
        //printf("%d\n", option_amt);
        for (int i = 0 ; i < trade_buffer_amt ; i++){
          int line_size = 100;
          TradeRepr((char *)trade_repr, REPR_SIZE, trades, i);
          STB_Font_render(region,line_size,10,region.height - line_size*(i + 1), trade_repr, "C:/Windows/Fonts/arial.ttf", 0xffeeff, color);
        }
        printf("%s\n", "Font rendering end");
        RECT rect;
        GetClientRect(window, &rect);
        DrawOnScreen(hdc, region, rect.right - rect.left, rect.bottom - rect.top);
        ShowWindow(window, SW_SHOW);
      }
    }
  }
}
