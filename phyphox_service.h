#ifndef PHYPHOX_SERVICE_H
#define PHYPHOX_SERVICE_H

class BLEServer;

bool startPhyphoxService();
bool stopPhyphoxService();
bool isPhyphoxServiceConnected();
BLEServer* getPhyphoxBleServer();
void loopPhyphoxService();

#endif
