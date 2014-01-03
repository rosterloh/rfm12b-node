#ifndef _rfm12b_h_
#define _rfm12b_h_

#include <node.h>
#include <v8.h>
#include <node_buffer.h>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <unistd.h>
//#include <fcntl.h>
#include <errno.h>
//#include <termios.h>

enum BandID {
    RFM12B_BAND_433_MHZ = 1,
    RFM12B_BAND_868_MHZ = 2,
    RFM12B_BAND_915_MHZ = 3
};

enum BitRate {
    RFM12B_BIT_RATE_115200_BPS = 0x02,
    RFM12B_BIT_RATE_57600_BPS = 0x05,
    RFM12B_BIT_RATE_49200_BPS = 0x06,
    RFM12B_BIT_RATE_38400_BPS = 0x08,
    RFM12B_BIT_RATE_19200_BPS = 0x11,
    RFM12B_BIT_RATE_9600_BPS = 0x23,
    RFM12B_BIT_RATE_4800_BPS = 0x47,
    RFM12B_BIT_RATE_2400_BPS = 0x91,
    RFM12B_BIT_RATE_1200_BPS = 0x9E
};

#define ERROR_STRING_SIZE 1024

v8::Handle<v8::Value> Open(const v8::Arguments& args);
void EIO_Open(uv_work_t* req);
void EIO_AfterOpen(uv_work_t* req);
void AfterOpenSuccess(int fd, v8::Handle<v8::Value> dataCallback, v8::Handle<v8::Value> errorCallback);

v8::Handle<v8::Value> Write(const v8::Arguments& args);
void EIO_Write(uv_work_t* req);
void EIO_AfterWrite(uv_work_t* req);

v8::Handle<v8::Value> Close(const v8::Arguments& args);
void EIO_Close(uv_work_t* req);
void EIO_AfterClose(uv_work_t* req);

BandID ToBandEnum(int band);
BitRate ToBitRateEnum(int rate);
    
struct OpenBaton {
public:
  char path[1024];
  v8::Persistent<v8::Value> callback;
  v8::Persistent<v8::Value> dataCallback;
  v8::Persistent<v8::Value> errorCallback;
  int result;
  int groupID;
  BandID bandID;
  BitRate bitRate;
  bool sendACK;
  int nodeID;
  char errorString[ERROR_STRING_SIZE];
};

struct WriteBaton {
public:
  int fd;
  char* bufferData;
  size_t bufferLength;
  size_t offset;
  v8::Persistent<v8::Object> buffer;
  v8::Persistent<v8::Value> callback;
  int result;
  char errorString[ERROR_STRING_SIZE];
};

struct QueuedWrite {
public:
  uv_work_t req;
  ngx_queue_t queue;
  WriteBaton* baton;
};

struct CloseBaton {
public:
  int fd;
  v8::Persistent<v8::Value> callback;
  char errorString[ERROR_STRING_SIZE];
};

#endif