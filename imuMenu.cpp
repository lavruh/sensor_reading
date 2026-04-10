#include <M5Unified.h>
#include "menu.h"
#include "field.h"


struct rect_t
{
  int32_t x;
  int32_t y;
  int32_t w;
  int32_t h;
};

extern uint16_t lineH;
extern TitleField titleField;
extern TitleField body1;
extern TitleField body2;
extern String dataToSend;
extern void typeText(String);
void drawGraph(const rect_t& r, const m5::imu_data_t& data);



// IMUMenuStrategy implementation
void IMUMenuStrategy::loopTask() {
  titleField.show("Gyro");

 
  auto imu_update = M5.Imu.update();
  int dataOffset = M5.Display.width()/2;
  static rect_t rect_graph_area = {M5.Display.width()/2, M5.Display.height() /2 , M5.Display.width()/2, 200 };
  auto data = M5.Imu.getImuData();
  drawGraph(rect_graph_area, data);

  char buffer[20];

  float list = atan2(-data.accel.x, sqrt(data.accel.y * data.accel.y + data.accel.z * data.accel.z)) * 180.0 / PI;
  float trim = atan2(data.accel.y, data.accel.z) * 180.0 / PI;


  body1.showValue(list, "L:");
  body2.showValue(trim, "T:");

  snprintf(buffer, sizeof(buffer), "L:%.1f T:%.1f", list, trim);
  dataToSend = buffer;
  M5.delay(100);

}

void IMUMenuStrategy::onButtonA() {
    typeText(dataToSend);
}


static constexpr const uint32_t color_tbl[18] = 
{
0xFF0000u, 0xCCCC00u, 0xCC00FFu,
0xFFCC00u, 0x00FF00u, 0x0088FFu,
0xFF00CCu, 0x00FFCCu, 0x0000FFu,
0xFF0000u, 0xCCCC00u, 0xCC00FFu,
0xFFCC00u, 0x00FF00u, 0x0088FFu,
0xFF00CCu, 0x00FFCCu, 0x0000FFu,
};
static constexpr const float coefficient_tbl[3] = { 0.5f, (1.0f / 256.0f), (1.0f / 1024.0f) };
static int prev_xpos[18];

void drawBar(int32_t ox, int32_t oy, int32_t nx, int32_t px, int32_t h, uint32_t color)
{
  uint32_t bgcolor = (color >> 3) & 0x1F1F1Fu;
  if (px && ((nx < 0) != (px < 0)))
  {
    M5.Display.fillRect(ox, oy, px, h, bgcolor);
    px = 0;
  }
  if (px != nx)
  {
    if ((nx > px) != (nx < 0))
    {
      bgcolor = color;
    }
    M5.Display.setColor(bgcolor);
    M5.Display.fillRect(nx + ox, oy, px - nx, h);
  }
}

void drawGraph(const rect_t& r, const m5::imu_data_t& data)
{
  float aw = (128 * r.w) >> 1;
  float gw = (128 * r.w) / 256.0f;
  float mw = (128 * r.w) / 1024.0f;
  int ox = (r.x + r.w)>>1;
  int oy = r.y;
  int h = (r.h / 18);
  int bar_count = 9;

  M5.Display.startWrite();
  for (int index = 0; index < bar_count; ++index)
  {
    float xval;
    if (index < 9)
    {
      auto coe = coefficient_tbl[index / 3] * r.w;
      xval = data.value[index] * coe;
    }
    else
    {
      xval = M5.Imu.getOffsetData(index - 9) * (1.0f / (1 << 19));
    }

    // for Linear scale graph.
    float tmp = xval;

    // The smaller the value, the larger the amount of change in the graph.
//  float tmp = sqrtf(fabsf(xval * 128)) * (signbit(xval) ? -1 : 1);

    int nx = tmp;
    int px = prev_xpos[index];
    if (nx != px)
    prev_xpos[index] = nx;
    drawBar(ox, oy + h * index, nx, px, h - 1, color_tbl[index]);
  }
  M5.Display.endWrite();
}