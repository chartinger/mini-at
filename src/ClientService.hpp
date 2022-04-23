#ifndef CLIENT_SERVCE_H
#define CLIENT_SERVCE_H

class ClientService {
  public:
    virtual void setup();
    virtual void loop();
    virtual void send(void *clientData, const char *data);
};

#endif
