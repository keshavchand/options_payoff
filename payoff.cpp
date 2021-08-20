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
#define HEIGHT 1000
// Pixels start at (0,0) at top and go to (Height, width) at lowest
unsigned int PIXELS [WIDTH * HEIGHT];


void GenerateTestScreen(unsigned int * pixels, int offset){
  for( int i = 0; i < WIDTH; i++){
    for( int j = 0; j < HEIGHT; j++){
      pixels[i * HEIGHT + j] = (0x00 + offset)&0xff;
      pixels[i * HEIGHT + j] |= ((0x00 + offset)&0xff) << 8;
    }
  }

}

static void DrawStraightLineAlongY(unsigned int *pixels, int point_x , int point_y1, int point_y2,unsigned int color){
  if ( point_y1 > point_y2){
    int temp = point_y2;
    point_y2 = point_y1;
    point_y1 = temp;
  }

  for(int y = point_y1 ; y < point_y2; y++){
     pixels[y * WIDTH + point_x] = color;
  }
}

static void DrawStraightLineAlongX(unsigned int *pixels, int point_y , int point_x1, int point_x2,unsigned int color){
  if ( point_x1 > point_x2){
    int temp = point_x2;
    point_x2 = point_x1;
    point_x1 = temp;
  }

  for(int x = point_x1 ; x <  point_x2 ; x++){
     pixels[point_y * WIDTH + x] = color;
  }
}

void DrawLine(unsigned int *pixels, int point_x1, int point_y1, int point_x2, int point_y2, unsigned int foreground_color){
  if ( point_x1 == point_x2){
    DrawStraightLineAlongX(pixels, point_x1, point_y1, point_y2, foreground_color);
    return;
  }
  if ( point_y1 == point_y2){
    DrawStraightLineAlongY(pixels, point_y1, point_x1, point_x2, foreground_color);
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
      pixels[(y) * WIDTH + x] = foreground_color ;
    }

  }
  
 return; 
}
 
static BITMAPINFO bmi;
int DrawOnScreen(HDC hdc, int screen_width, int screen_height){
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
  LRESULT result = 0;
  switch (message){
    case WM_CLOSE: {
      Running = 0;
    } break;
    case WM_PAINT: {
       PAINTSTRUCT ps;
       GenerateTestScreen((unsigned int *) &PIXELS, offset++);
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
         DrawLine((unsigned int*) PIXELS, 1, 1, 100, 100, 0xffeeff);
         DrawLine((unsigned int*) PIXELS, 1, 10, 10, 100, 0xffeeff);
         //DrawLine((unsigned int*) PIXELS, 100, 50, 200, 100, 0xffeeff);
         //DrawLine((unsigned int*) PIXELS, 100, 1, 100, 100, 0xffeeff);
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
