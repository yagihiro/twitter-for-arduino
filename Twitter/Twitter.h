/*
  Twitter.h - twitter interface for arduino

  Copyright (c) 2009 Hiroki Yagita.

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  'Software'), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef TWITTER_h
#define TWITTER_h
#include <stdlib.h>
#include <string.h>
#include <HardwareSerial.h>
#include "Base64.h"
#include "../Ethernet/Client.h"

const char *CRLF = "\r\n";
uint8_t TWITTER_IPADDR[] = { 128,121,146,100 };

class TwitterClass {
public:
  TwitterClass():
    cli_(TWITTER_IPADDR, 80),
    encoded_account_(0),
    initialized_(false),
    status_() {}
  ~TwitterClass() {
    destroy_encoded_account();
  }

  void begin(const char *username, const char *password) {
    const char *c = ":";
    int len = strlen(username) + strlen(c) + strlen(password) + 1;

    char *buf = (char *)malloc(len);
    memset(buf, 0, len);

    strcat(buf, username);
    strcat(buf, c);
    strcat(buf, password);

    destroy_encoded_account();
    encoded_account_ = (char *)malloc(len*2);
    memset(encoded_account_, 0, len*2);
    base64(encoded_account_, buf, len - 1);
    free(buf);

    initialized_ = true;
  }

  bool update(const char *msg) {
    if (!initialized_) return false;
    if (!cli_.connect()) return false;

    status_.setup(msg);
    print("POST /statuses/update.json HTTP/1.1\r\n");
    print("Host: twitter.com\r\n");
    print_authorization_line();
    print(status_.length_);
    print(CRLF);
    print(status_.status_);
    print(CRLF);
    while (!cli_.available())
      ;
    return true;
  }

  void print_response() {
    while (cli_.connected()) {
      if (cli_.available()) {
        char c = cli_.read();
        Serial.print(c);
      }
    }

    cli_.stop();
  }

private:

  void print(const char *s) {
    Serial.print(s);
    cli_.print(s);
  }

  void print_authorization_line() {
    print("Authorization: Basic ");
    print(encoded_account_);
    print(CRLF);
  }

  void destroy_encoded_account() {
    if (encoded_account_) {
      free(encoded_account_);
      encoded_account_ = 0;
    }
  }

  Client cli_;
  char *encoded_account_;
  bool initialized_;

  struct Status {
    char *length_;
    char *status_;

    Status(): length_(0), status_(0) {}
    ~Status() { teardown(); }

    void setup(const char *msg) {
      teardown();
      setup_status(msg);
      setup_length();
    }
    void teardown() {
      if (length_) {
        free(length_);
        length_ = 0;
      }
      if (status_) {
        free(status_);
        status_ = 0;
      }
    }
    void setup_status(const char *msg) {
      const char *tag = "status=";
      int len = strlen(tag) + strlen(msg) + strlen(CRLF);

      status_ = (char *)malloc(len + 1);
      strcpy(status_, tag);
      strcat(status_, msg);
      strcat(status_, CRLF);
    }

    int setup_length() {
      const char *tag = "Content-Length: ";
      char len_s[4];

      itoa(strlen(status_), len_s, 10);

      int len = strlen(tag) + strlen(len_s) + strlen(CRLF);

      length_ = (char *)malloc(len + 1);
      strcpy(length_, tag);
      strcat(length_, len_s);
      strcat(length_, CRLF);
    }
  } status_;
};

TwitterClass Twitter;

#endif
