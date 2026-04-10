#include "Graph.h"
#include <M5Unified.h>

Graph::Graph(int x, int y, int width, float minVal, float maxVal) : x(x), y(y), minVal(minVal), maxVal(maxVal), width(width), maxReading(0), minReading(0) {
}

int Graph::toScale(float val) {
    if (val <= minVal) return 0;
    if (val >= maxVal) return 100;
    
    // Linear interpolation from [minVal, maxVal] to [0, 100]
    return (int)((val - minVal) / (maxVal - minVal) * 100.0f);
}

void Graph::showValue(float v) {
    int val = toScale(v);
    if (val < 0) val = 0;
    if (val > 100) val = 100;
    
    // Draw progress bar background (outline)
    M5.Display.drawRect(x, y, width + 2, 12, WHITE);
    
    // Draw filled portion
    int fillWidth = (val * width) / 100; // Scale to width pixels
    M5.Display.fillRect(x + 1, y + 1, fillWidth, 10, GREEN);
    
    // Clear the unfilled portion
    M5.Display.fillRect(x + 1 + fillWidth, y + 1, width - fillWidth, 10, BLACK);
}

void Graph::reset() {
    maxReading = 0;
    minReading = 0;
}

void Graph::showHistValue(float v) {
    // Update max and min readings
    if (v > maxReading) maxReading = v;
    if (v < minReading || minReading == 0) minReading = v;
    
    // Show the progress bar like showValue
    int val = toScale(v);
    if (val < 0) val = 0;
    if (val > 100) val = 100;
    
    // Draw progress bar background (outline)
    M5.Display.drawRect(x, y, width + 2, 12, WHITE);
    
    // Draw filled portion
    int fillWidth = (val * width) / 100; // Scale to width pixels
    M5.Display.fillRect(x + 1, y + 1, fillWidth, 10, GREEN);
    
    // Clear the unfilled portion
    M5.Display.fillRect(x + 1 + fillWidth, y + 1, width - fillWidth, 10, BLACK);
    
    // Draw orange vertical bars for max and min readings
    int maxPos = (toScale(maxReading) * width) / 100;
    int minPos = (toScale(minReading) * width) / 100;

    int maxX = x + 1 + maxPos - 1;
    int minX = x + 1 + minPos - 1;

    if (maxX < x + 1) maxX = x + 1;
    if (maxX + 3 > x + 1 + width) maxX = x + 1 + width - 3;
    if (minX < x + 1) minX = x + 1;
    if (minX + 3 > x + 1 + width) minX = x + 1 + width - 3;

    M5.Display.fillRect(maxX, y + 1, 3, 10, ORANGE);
    M5.Display.fillRect(minX, y + 1, 3, 10, ORANGE);

    // Draw min/max readings below the graph
    String minLabel = "min:" + String(minReading, 1);
    String maxLabel = "max:" + String(maxReading, 1);
    int labelY = y + 20;

    M5.Display.fillRect(x + 1, labelY, width, 10, BLACK);
    M5.Display.drawString(minLabel, x + 1, labelY, &fonts::DejaVu9);

    int maxLabelWidth = maxLabel.length() * 6;
    int maxLabelX = x + 1 + width - maxLabelWidth;
    if (maxLabelX < x + 1) maxLabelX = x + 1;
    M5.Display.drawString(maxLabel, maxLabelX, labelY, &fonts::DejaVu9);
}