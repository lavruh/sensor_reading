#ifndef GRAPH_H
#define GRAPH_H

#include <Arduino.h>

class Graph {
private:
    int x, y, width;
    float minVal, maxVal;
    float maxReading, minReading;

public:
    Graph(int x, int y, int width, float minVal, float maxVal);
    int toScale(float val);
    void showValue(float val);
    void reset();
    void showHistValue(float v);
};

#endif // GRAPH_H