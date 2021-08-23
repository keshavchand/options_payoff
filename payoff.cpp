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

//#define WIDTH 3840
//#define HEIGHT 2160
#define WIDTH 1000
#define HEIGHT 800
// Pixels start at (0,0) at top and go to (Height, width) at lowest
unsigned int PIXELS [WIDTH * HEIGHT];


void FillScreen(unsigned int * pixels, int color){
  for( int i = 0; i < WIDTH; i++){
    for( int j = 0; j < HEIGHT; j++){
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
  //SetStretchBltMode(hdc, COLORONCOLOR);
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

void RenderPayoff(unsigned int* pixels, int max_payoff,int max_payoff_price, int min_payoff,int min_payoff_price, int *price_output, int start_price, int end_price, int padding_X = 0, int padding_Y = 0){

  //For Width adjustment : X axis
  int shifted_max_price     = max_payoff_price - min_payoff_price;
  int price_range_len       = shifted_max_price; 
  //max_price * streth      = Width
  float stretch_ratio_price = float(WIDTH - 2 * padding_X) / float(shifted_max_price);

  //For Height adjustment : Y axis
  int shifted_max_payoff    = max_payoff - min_payoff;
  shifted_max_payoff        += 2 * padding_Y;
  int payoff_range_len      = shifted_max_payoff; 
  //max_payoff * streth      = HEIGHT
  float stretch_ratio_payoff = float(HEIGHT - 2 * padding_Y) / float(shifted_max_payoff);

  int no_of_prices = end_price - start_price;

  //Iterate[0..no_of_prices - 1] and draw a line from[i to i + 1];
  for (int i = 0 ; i < no_of_prices - 1; i++){
    int x_pos_start = (int) (i    ) * stretch_ratio_price + padding_X;
    int x_pos_end   = (int) (i + 1) * stretch_ratio_price + padding_X;
    int y_pos_start = (int) (price_output[i]     - min_payoff) * stretch_ratio_payoff + padding_Y;
    int y_pos_end   = (int) (price_output[i + 1] - min_payoff) * stretch_ratio_payoff + padding_Y;
    DrawLine(pixels, x_pos_start, y_pos_start, x_pos_end, y_pos_end, 0xffeeff);

  }

}

#define STB_TRUETYPE_IMPLEMENTATION
#include "./stb_truetype.h"

static stbtt_fontinfo Font;
unsigned char Font_contents[0xfe000];
void STB_Font_render(unsigned int* pixels, char* text, char *font_filename, int foreground_color, int background_color){
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
 
  int last_height = 0;
  for(int i = 0 ; ; i++){
    int width, height;
    if(text[i] == 0) break;
    int x_off;
    unsigned char *bitmap = stbtt_GetCodepointBitmap(&Font, 0, stbtt_ScaleForPixelHeight(&Font, 80), text[i], &width, &height, &x_off ,0);

    //for(int Y_pos = height - 1; Y_pos >= 0; Y_pos--){
    for(int Y_pos = 0; Y_pos < height; Y_pos++){
      for(int X_pos = 0; X_pos < width; X_pos++){
        int idx_y = (height - Y_pos)* WIDTH;
        int idx_x = (X_pos + last_height);
        if(idx_x > WIDTH) break;
        int idx = idx_y + idx_x;
        if(idx < 0 || idx > HEIGHT*WIDTH) break;
        pixels[idx] = background_color;
        if(bitmap[Y_pos * width + X_pos]) pixels[idx] = foreground_color;
//          (unsigned int)(bitmap[Y_pos * width + X_pos] << 8 * 3 |
//                        bitmap[Y_pos * width + X_pos]);
           
      }
    }

    last_height += height;
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
       PAINTSTRUCT ps;
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

int main(){

  Option opt[10];
  //for(int i = 100 ; i < 100 + 10; i++){
  //  Option *o = &opt[i - 100];
  //  //o->type = CALL ; 
  //  o->type = PUT ; 
  //  o->strike_price = i * 100 + i + 10;
  //  o->expiry_date  = i % 30 + 1;
  //  o->expiry_month = i % 12 + 1;
  //  o->expiry_year  = 2021;
  //}


  struct GetAmt{
    static int getAmt(float a){
      return (int)100*a;
    }
  };

  //Isn't worring about expiry dates
  {
    opt[0].type = CALL;
    opt[0].strike_price = GetAmt::getAmt(150.00);
  }
  {
    opt[1].type = PUT;
    opt[1].strike_price = GetAmt::getAmt(100.00);
  }
  {
    opt[2].type = PUT;
    opt[2].strike_price = GetAmt::getAmt(200.00);
  }
  {
    opt[3].type = CALL;
    opt[3].strike_price = GetAmt::getAmt(500.00);
  }
  {
    opt[4].type = CALL;
    opt[4].strike_price = GetAmt::getAmt(100.00);
  }
// 10000 = 100.00
#define STARTPRICE 10000 
#define ENDPRICE 20000 

  int output_prices[1 + ENDPRICE - STARTPRICE];
  int max_payoff;
  int max_payoff_price;
  int min_payoff;
  int min_payoff_price;
  CalculateOptionPayoff(opt, 5, STARTPRICE, ENDPRICE, output_prices, &max_payoff,&max_payoff_price, &min_payoff, &min_payoff_price);

  //for(int i = 0 ; i < 20; i++){
  //  printf("%u %u %u %u := %u\n", 
  //      opt[i].strike_price, opt[i].expiry_date,
  //      opt[i].expiry_month,opt[i].expiry_year,
  //      OptionIntrinsicValue(opt[i], 500));
  //}


  char * window_class_name = "Option Payoff Chart";
  WNDCLASSA window_class = {0};
  window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  window_class.lpfnWndProc = WindowProc;
  window_class.hInstance = GetModuleHandle(0);
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
        int color = 0x808080;
        FillScreen((unsigned int *) &PIXELS, color);
        RenderPayoff(PIXELS, max_payoff , ENDPRICE, min_payoff , STARTPRICE, output_prices, STARTPRICE, ENDPRICE, 10 , 10);
        STB_Font_render(PIXELS, "HELLO", "C:/Windows/Fonts/arial.ttf", 0xffeeff, 0x808080);
        RECT rect;
        GetClientRect(window, &rect);
        DrawOnScreen(hdc, rect.right - rect.left, rect.bottom - rect.top);
        ShowWindow(window, SW_SHOW);
      }
    }
  }
}
