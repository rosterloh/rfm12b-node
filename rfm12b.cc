#include <node.h>

#include "rfm12b.h"
#include <sys/ioctl.h>
#include "rfm12b_ioctl.h"

using namespace v8;

uv_mutex_t write_queue_mutex;
ngx_queue_t write_queue;

Handle<Value> Open(const Arguments& args) {
    HandleScope scope;
    
    uv_mutex_init(&write_queue_mutex);
    ngx_queue_init(&write_queue);
    
    //Check inputs
    if(!args[0]->IsString()) {
        return scope.Close(ThrowException(Exception::TypeError(String::New("Device file path must be a string"))));
    }
    String::Utf8Value path(args[0]->ToString());
    
    if(!args[1]->IsObject()) {
        return scope.Close(ThrowException(Exception::TypeError(String::New("Second argument must be an object"))));
    }
    Local<Object> settings = args[1]->ToObject();

    // callback
    if(!args[2]->IsFunction()) {
        return scope.Close(ThrowException(Exception::TypeError(String::New("Third argument must be a function"))));
    }
    Local<Value> callback = args[2];
    
    OpenBaton* baton = new OpenBaton();
    memset(baton, 0, sizeof(OpenBaton));
    strcpy(baton->path, *path);
    baton->groupID = settings->Get(String::New("groupID"))->ToInt32()->Int32Value();
    baton->bandID = ToBandEnum(settings->Get(String::New("bandID"))->ToNumber()->NumberValue());
    baton->bitRate = ToBitRateEnum(settings->Get(String::New("bitRate"))->ToNumber()->NumberValue());
    baton->sendACK = settings->Get(String::New("sendACK"))->ToBoolean()->BooleanValue();
    baton->nodeID = settings->Get(String::New("nodeID"))->ToInt32()->Int32Value();
    
    baton->callback = Persistent<Value>::New(callback);
    baton->dataCallback = Persistent<Value>::New(settings->Get(String::New("dataCallback")));
    baton->errorCallback = Persistent<Value>::New(settings->Get(String::New("errorCallback")));

    uv_work_t* req = new uv_work_t();
    req->data = baton;
    uv_queue_work(uv_default_loop(), req, EIO_Open, (uv_after_work_cb)EIO_AfterOpen);
        
    return scope.Close(Undefined());
}

void EIO_Open(uv_work_t* req) {
    OpenBaton* data = static_cast<OpenBaton*>(req->data);
    
    int flags = (O_RDWR | O_NONBLOCK);
    int fd = open(data->path, flags);
    
    if (fd == -1) {
        snprintf(data->errorString, sizeof(data->errorString), "Cannot open %s", data->path);
        return;
    }
    
    // Flush maybe?
    //ret = ioctl(rfm12_fd, RFM12B_IOCTL_GET_GROUP_ID, &group_id);
    //ret = ioctl(rfm12_fd, RFM12B_IOCTL_GET_BAND_ID, &band_id);
    //ret = ioctl(rfm12_fd, RFM12B_IOCTL_GET_BIT_RATE, &bit_rate);
    
    int send_ack = 1;
    if(!data->sendACK)
        send_ack = 0;
        
    if (ioctl(fd, RFM12B_IOCTL_SET_JEEMODE_AUTOACK, &send_ack) == -1) {
        snprintf(data->errorString, sizeof(data->errorString), "Error %s calling ioctl( ..., RFM12B_IOCTL_SET_JEEMODE_AUTOACK, %d )", strerror(errno), send_ack );
    }
    
    // activate jeenode-compatible mode by giving this module a jeenode id
    if (ioctl(fd, RFM12B_IOCTL_SET_JEE_ID, &data->nodeID)) {
        snprintf(data->errorString, sizeof(data->errorString), "Error %s calling ioctl( ..., RFM12B_IOCTL_SET_JEE_ID, %d )", strerror(errno), data->nodeID );
    }
    
    data->result = fd;
}

void EIO_AfterOpen(uv_work_t* req) {
  OpenBaton* data = static_cast<OpenBaton*>(req->data);

  Handle<Value> argv[2];
  if(data->errorString[0]) {
    argv[0] = Exception::Error(String::New(data->errorString));
    argv[1] = Undefined();
  } else {
    argv[0] = Undefined();
    argv[1] = Int32::New(data->result);
    AfterOpenSuccess(data->result, data->dataCallback, data->errorCallback);
  }
  Function::Cast(*data->callback)->Call(Context::GetCurrent()->Global(), 2, argv);

  data->callback.Dispose();
  delete data;
  delete req;
}

void AfterOpenSuccess(int fd, Handle<Value> dataCallback, Handle<Value> errorCallback) {

}

Handle<Value> Close(const Arguments& args) {
  HandleScope scope;

  // file descriptor
  if(!args[0]->IsInt32()) {
    return scope.Close(ThrowException(Exception::TypeError(String::New("First argument must be an int"))));
  }
  int fd = args[0]->ToInt32()->Int32Value();

  // callback
  if(!args[1]->IsFunction()) {
    return scope.Close(ThrowException(Exception::TypeError(String::New("Second argument must be a function"))));
  }
  Local<Value> callback = args[1];

  CloseBaton* baton = new CloseBaton();
  memset(baton, 0, sizeof(CloseBaton));
  baton->fd = fd;
  baton->callback = Persistent<Value>::New(callback);

  uv_work_t* req = new uv_work_t();
  req->data = baton;
  uv_queue_work(uv_default_loop(), req, EIO_Close, (uv_after_work_cb)EIO_AfterClose);

  return scope.Close(Undefined());
}

void EIO_Close(uv_work_t* req) {
  CloseBaton* data = static_cast<CloseBaton*>(req->data);

  ssize_t r;

  r = close(data->fd);

  if (r && r != EBADF)
    snprintf(data->errorString, sizeof(data->errorString), "Unable to close fd %d, errno: %d", data->fd, errno);
}

void EIO_AfterClose(uv_work_t* req) {
  CloseBaton* data = static_cast<CloseBaton*>(req->data);

  Handle<Value> argv[1];
  if(data->errorString[0]) {
    argv[0] = Exception::Error(String::New(data->errorString));
  } else {
    argv[0] = Undefined();
  }
  Function::Cast(*data->callback)->Call(Context::GetCurrent()->Global(), 1, argv);

  data->callback.Dispose();
  delete data;
  delete req;
}

Handle<Value> Write(const Arguments& args) {
    HandleScope scope;
        
    return scope.Close(Undefined());
}

BandID ToBandEnum(int band) {
  switch(band) {
      case 1: return RFM12B_BAND_433_MHZ;
      case 2: return RFM12B_BAND_868_MHZ;
      case 3: return RFM12B_BAND_915_MHZ;
      default: return RFM12B_BAND_868_MHZ;
  }
}

BitRate ToBitRateEnum(int rate) {
  switch(rate) {
      case 0x02: return RFM12B_BIT_RATE_115200_BPS;
      case 0x05: return RFM12B_BIT_RATE_57600_BPS;
      case 0x06: return RFM12B_BIT_RATE_49200_BPS;
      case 0x08: return RFM12B_BIT_RATE_38400_BPS;
      case 0x11: return RFM12B_BIT_RATE_19200_BPS;
      case 0x23: return RFM12B_BIT_RATE_9600_BPS;
      case 0x47: return RFM12B_BIT_RATE_4800_BPS;
      case 0x91: return RFM12B_BIT_RATE_2400_BPS;
      case 0x9E: return RFM12B_BIT_RATE_1200_BPS;
      default: return RFM12B_BIT_RATE_49200_BPS;
  }
}

void init(Handle<Object> exports) {
    HandleScope scope;
    NODE_SET_METHOD(exports, "open", Open);
    NODE_SET_METHOD(exports, "close", Close);
    NODE_SET_METHOD(exports, "write", Write);
}

NODE_MODULE(rfm12b, init)