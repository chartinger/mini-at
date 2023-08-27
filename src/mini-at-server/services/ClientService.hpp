#ifndef CLIENT_SERVCE_H
#define CLIENT_SERVCE_H

class ClientService {
  public:
    virtual void setup() = 0;
    virtual void loop() = 0;
    virtual void send(void *clientData, const char *data) = 0;
};

#endif
