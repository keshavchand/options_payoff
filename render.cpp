
typedef unsigned int uint;
struct RenderRegion{
  int    width;
  int    height;
  uint*  pixels;
  stbtt_fontinfo Font;
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

//static stbtt_fontinfo Font;
#define DEFAULT_CHAR_SIZE 0xfe000
unsigned char* Font_contents = 0;

stbtt_fontinfo STB_font_init(char *font_filename){
  stbtt_fontinfo Font;
    OFSTRUCT file_open_buff;
    BY_HANDLE_FILE_INFORMATION file_info;
    HANDLE TTF_File = CreateFileA(font_filename,GENERIC_READ,  FILE_SHARE_READ,NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
    GetFileInformationByHandle(TTF_File, &file_info);

    assert(file_info.nFileSizeHigh == 0);
    unsigned long file_size = file_info.nFileSizeLow; 
    if (file_size > DEFAULT_CHAR_SIZE) printf("FONT FILE SIZE: %d\n", file_size);
    size_t alloc_space_size = file_size * sizeof (unsigned char);
    alloc_space_size = (alloc_space_size > DEFAULT_CHAR_SIZE) ? alloc_space_size : DEFAULT_CHAR_SIZE;
    Font_contents = (unsigned char *)VirtualAlloc(0, alloc_space_size , MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    //Read entire file
    unsigned long bytes_read = 0;
    bool result = ReadFile(TTF_File, Font_contents, file_size,(DWORD *) &bytes_read, NULL);
    unsigned char a ;
    //for (int i = 0 ; i < file_size ; i++ ) a = Font_contents[i];
    assert(result);
    assert(bytes_read == file_size);
    int offset = stbtt_GetFontOffsetForIndex((const unsigned char*) &Font_contents,0);
    offset = (offset > 0) ? offset : 0;
   stbtt_InitFont(&Font, Font_contents, offset);
   return Font;
}



// WE SUPPORT ONLY CAPITAL LETTERS AND NUMBERS!!!!!!!!
//return width
int STB_Font_render_left(RenderRegion region, int line_height, int x_offset, int y_offset, char* text, unsigned int foreground_color, unsigned int background_color){
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
 
  int char_distance_x = 0;
  int char_distance_y = 0;
  float scale = stbtt_ScaleForPixelHeight(&Font, line_height);
  for(int i = 0 ; text[i] != 0; i++){
    int width, height;
    int x_off;
    int y_off;
    unsigned char *bitmap = stbtt_GetCodepointBitmap(&Font, 0, scale, text[i], &width, &height, &x_off ,&y_off);
    int ascent, decent, lineGap;
    stbtt_GetFontVMetrics(&Font ,&ascent, &decent, &lineGap);
    char_distance_y = (char_distance_y > (ascent - decent + lineGap)) ? char_distance_y : ascent - decent + lineGap;
    //height = ascent - decent + lineGap;
    for(int Y_pos = 0; Y_pos < height; Y_pos++){
      for(int X_pos = 0; X_pos < width; X_pos++){
        //int idx_y = (HEIGHT - 10 - Y_pos)* WIDTH;
        int idx_y = ((height + y_offset)- Y_pos);
        int idx_x = (X_pos + x_offset + char_distance_x);
        int idx_bmp = Y_pos * width + X_pos;
        //lerp the color between forward and background color
        int color = RGBLerp::lerp(background_color, foreground_color , (double) bitmap[idx_bmp] / 255);
        DrawPixel(region, idx_x, idx_y, color);
      }
    }

    int advance_width, leftSideBearing;
    stbtt_GetCodepointHMetrics(&Font, text[i], &advance_width, &leftSideBearing);
    char_distance_x += (int)((advance_width + leftSideBearing)* scale);
    char_distance_x += (int)(stbtt_GetCodepointKernAdvance(&Font, text[i], text[i+1]) * scale);
    stbtt_FreeBitmap(bitmap, 0);
  }

  return char_distance_x - x_offset;
}


void STB_Font_render_right(RenderRegion region, int line_height, int x_offset, int y_offset, char* text, unsigned int foreground_color, unsigned int background_color){
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
  int lent = strlen(text);
  for(int i = lent - 1; i >= 0 ; i--){
    int width, height;
    int x_off;
    int y_off;
    unsigned char *bitmap = stbtt_GetCodepointBitmap(&Font, 0, scale, text[i], &width, &height, &x_off , &y_off);

    for(int Y_pos = 0; Y_pos < height; Y_pos++){
      for(int X_pos = 0; X_pos < width; X_pos++){
        //int idx_y = (HEIGHT - 10 - Y_pos)* WIDTH;
        int idx_y = (height + y_offset - Y_pos);
        int idx_x = (X_pos  + x_offset - (width + x_off) - char_distance);
        int idx_bmp = Y_pos * width + X_pos;
        //lerp the color between forward and background color
        int color = RGBLerp::lerp(background_color, foreground_color , (double) bitmap[idx_bmp] / 255);
        if( idx_x > region.width || idx_y > region.height) printf("%d %d\n", idx_x, idx_y);
        DrawPixel(region, idx_x, idx_y, color);
      }
    }

    int advance_width, leftSideBearing;
    stbtt_GetCodepointHMetrics(&Font, text[i], &advance_width, &leftSideBearing);
    //char_distance += (int)(advance_width * scale);
    char_distance += (int)((leftSideBearing + advance_width) * scale);
    char_distance += (int)(stbtt_GetCodepointKernAdvance(&Font, text[i - 1], text[i]) * scale);
    stbtt_FreeBitmap(bitmap, 0);
  }
}

void MergeRenderRegion(RenderRegion dst, RenderRegion src, int from_x, int from_y) {
  int dst_y = from_y;
  int dst_x = from_x;
  for(int src_y = 0; src_y < src.height; src_y++){
    for(int src_x = 0; src_x < src.width; src_x++){
      uint pixel = src.pixels[src_y * src.width + src_x];
      DrawPixel(dst, dst_x, dst_y, pixel);
      dst_x ++;
    }
    dst_y ++;
    dst_x = from_x;
  }
}
