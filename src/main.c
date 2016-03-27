#include <pebble.h>

Window *my_window;
TextLayer *text_layer;
TextLayer *step_layer;
static BitmapLayer *bitmap_layer;
static GBitmap *megaman_bitmap;
static int counter=0;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(text_layer, s_buffer);
}

//static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
//  update_time();
//}

static void update_megaman(){
  gbitmap_destroy(megaman_bitmap);
  bitmap_layer_destroy(bitmap_layer);
  bitmap_layer = bitmap_layer_create(GRect(48,60,50,50));
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(bitmap_layer)); 
  if (counter==2)
    megaman_bitmap = gbitmap_create_with_resource(RESOURCE_ID_megamanRun1);          
  else if (counter==0)
    megaman_bitmap = gbitmap_create_with_resource(RESOURCE_ID_megamanRun2);
  else
    megaman_bitmap = gbitmap_create_with_resource(RESOURCE_ID_megamanMidRun);
  counter++;
  if (counter>=4)
    counter=0;
  bitmap_layer_set_bitmap(bitmap_layer, megaman_bitmap);
  //testing health
  HealthMetric metric = HealthMetricWalkedDistanceMeters;//HealthMetricStepCount;
  time_t start = time_start_of_today();
  time_t end = time(NULL);

  // Check the metric has data available for today
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, 
    start, end);
  if(mask & HealthServiceAccessibilityMaskAvailable) {
    // Data is available!
    int meters=health_service_sum_today(metric);
    int miles=meters/1600;
    int tenths=(meters-miles*1600)/160;
    static char data[10];
    snprintf(data, sizeof(data),"%d.%d Mi", miles,tenths);//(int)health_service_sum_today(metric)/1600);//???
    //snprintf(buf, sizeof(buf), "%ld", pedometerCount);
    text_layer_set_text(step_layer,data);
    APP_LOG(APP_LOG_LEVEL_INFO, "Miles today: %d.%d", 
            miles,tenths);
  } else {
    // No data recorded yet today
    text_layer_set_text(step_layer,"0.0 Mi");
//    APP_LOG(APP_LOG_LEVEL_ERROR, "Data unavailable!");
  }
  
}

static void megamanRun(struct tm *tick_time, TimeUnits units_chanaged) {
  update_megaman();
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create GBitmap
  megaman_bitmap = gbitmap_create_with_resource(RESOURCE_ID_megamanStand);
  //shrink bitmap?
  //gbitmap_set_bounds(megaman_bitmap, grect_inset(GRect(0,0,200,200),GEdgeInsets(0,0,0,0)));
  // Create BitmapLayer to display the GBitmap
  bitmap_layer = bitmap_layer_create(GRect(48,60,50,50));

  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(bitmap_layer, megaman_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
  
  // Create the TextLayer with specific bounds
  text_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(18, 12), bounds.size.w, 50));
  // Create step text layer
  step_layer = text_layer_create(GRect(0,110,bounds.size.w,50));
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_text(text_layer, "00:00");
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  
  // step text layer attributes
  text_layer_set_background_color(step_layer,GColorClear);
  text_layer_set_text_color(step_layer, GColorBlack);
  text_layer_set_text(step_layer, "0.0 Mi");
  text_layer_set_font(step_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(step_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  layer_add_child(window_layer, text_layer_get_layer(step_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(text_layer);
  text_layer_destroy(step_layer);
  // Destroy GBitmap
  gbitmap_destroy(megaman_bitmap);
  // Destroy BitmapLayer
  bitmap_layer_destroy(bitmap_layer);
}

void handle_init(void) {
  my_window = window_create();
  //window_set_background_color(my_window, GColorBlack);
  text_layer = text_layer_create(GRect(0,0,144,20));
  step_layer = text_layer_create(GRect(0,0,144,20));
  
    // Set handlers to manage the elements inside the Window
  window_set_window_handlers(my_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Show the Window on the watch, with animated=true
  window_stack_push(my_window, true);
  // Register with TickTimerService
//  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  tick_timer_service_subscribe(SECOND_UNIT, megamanRun);
  // Make sure the time is displayed from the start
  update_time(); 
}

void handle_deinit(void) {
  //text_layer_destroy(text_layer);
  //text_layer_destroy(step_layer);
  //gbitmap_destroy(megaman_bitmap);
  //bitmap_layer_destroy(bitmap_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
