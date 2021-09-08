int DrawSlider(RenderRegion slider_region, float slider_pos, int padding_x, int padding_y){
  //Check if mouse is over the slider
  {
    int start_x =  padding_x;
    int end_x   =  slider_region.width - padding_x;

    int y = slider_region.height / 2;
    int y_start = y - 10;
    int y_end = y + 10;

    for ( y = y_start ; y < y_end; y++)
      DrawLine(slider_region, start_x, y, end_x, y, 0xffeeff);
  }
  
  {
    int slider_position = slider_pos * slider_region.width;
    int start_x = slider_position - 10;
    int end_x   = slider_position + 10;

    for (int x = start_x ; x < end_x; x++)
      DrawLine(slider_region, x, 0, x, slider_region.height, 0xffeeff);
  }
  return 0;
}
