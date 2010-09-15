/*
**    nullserv.c - inetd service for spitting out null content. Integrate it 
**                 with DNS to provide advert blocking on a LAN.
**    
**    build: make nullserv && strip nullserv
**    run:   Add something like the following to /etc/inetd.conf and restart inetd.
**
**    www  stream  tcp  nowait  nobody  /usr/sbin/tcpd  /usr/local/bin/nullserv
**
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define DEFAULT_MIMETYPE "image/gif"

static const unsigned char null_gif[] = {
  0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x01, 0x00, 0x01, 0x00, 0xf0, 0x00, 
  0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x21, 0xf9, 0x04, 0x01, 0x00, 
  0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 
  0x00, 0x02, 0x02, 0x44, 0x01, 0x00, 0x3b
};

static const unsigned char null_png[] = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
  0x01, 0x03, 0x00, 0x00, 0x00, 0x25, 0xdb, 0x56, 0xca, 0x00, 0x00, 0x00, 
  0x03, 0x73, 0x42, 0x49, 0x54, 0x08, 0x08, 0x08, 0xdb, 0xe1, 0x4f, 0xe0, 
  0x00, 0x00, 0x00, 0x06, 0x50, 0x4c, 0x54, 0x45, 0xff, 0xff, 0xff, 0x00, 
  0x00, 0x00, 0x55, 0xc2, 0xd3, 0x7e, 0x00, 0x00, 0x00, 0x02, 0x74, 0x52, 
  0x4e, 0x53, 0x00, 0xff, 0x5b, 0x91, 0x22, 0xb5, 0x00, 0x00, 0x00, 0x0a, 
  0x49, 0x44, 0x41, 0x54, 0x08, 0x99, 0x63, 0x60, 0x00, 0x00, 0x00, 0x02, 
  0x00, 0x01, 0xf4, 0x71, 0x64, 0xa6, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 
  0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

static const unsigned char null_jpg[] = {
  0xff, 0xd8, 0xff, 0xdb, 0x00, 0x43, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 
  0xc0, 0x00, 0x0b, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x11, 0x00, 
  0xff, 0xc4, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0xff, 0xc4, 
  0x00, 0x14, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xda, 0x00, 0x08, 
  0x01, 0x01, 0x00, 0x00, 0x3f, 0x00, 0x7f, 0x0f, 0xff, 0xd9
};

/* compiled from swfdec_test_initialize.as */
static const unsigned char null_swf[] = {
  0x88, 0x08, 0x02, 0x35, 0x00, 0x42, 0x75, 0x66, 0x66, 0x65, 0x72, 0x00, 0x4E, 0x61, 0x74, 0x69,
  0x76, 0x65, 0x00, 0x6C, 0x6F, 0x61, 0x64, 0x00, 0x42, 0x75, 0x66, 0x66, 0x65, 0x72, 0x5F, 0x6C,
  0x6F, 0x61, 0x64, 0x00, 0x70, 0x72, 0x6F, 0x74, 0x6F, 0x74, 0x79, 0x70, 0x65, 0x00, 0x64, 0x69,
  0x66, 0x66, 0x00, 0x42, 0x75, 0x66, 0x66, 0x65, 0x72, 0x5F, 0x64, 0x69, 0x66, 0x66, 0x00, 0x66,
  0x69, 0x6E, 0x64, 0x00, 0x42, 0x75, 0x66, 0x66, 0x65, 0x72, 0x5F, 0x66, 0x69, 0x6E, 0x64, 0x00,
  0x73, 0x75, 0x62, 0x00, 0x42, 0x75, 0x66, 0x66, 0x65, 0x72, 0x5F, 0x73, 0x75, 0x62, 0x00, 0x74,
  0x6F, 0x53, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x00, 0x42, 0x75, 0x66, 0x66, 0x65, 0x72, 0x5F, 0x74,
  0x6F, 0x53, 0x74, 0x72, 0x69, 0x6E, 0x67, 0x00, 0x49, 0x6D, 0x61, 0x67, 0x65, 0x00, 0x63, 0x6F,
  0x6D, 0x70, 0x61, 0x72, 0x65, 0x00, 0x49, 0x6D, 0x61, 0x67, 0x65, 0x5F, 0x63, 0x6F, 0x6D, 0x70,
  0x61, 0x72, 0x65, 0x00, 0x73, 0x61, 0x76, 0x65, 0x00, 0x49, 0x6D, 0x61, 0x67, 0x65, 0x5F, 0x73,
  0x61, 0x76, 0x65, 0x00, 0x53, 0x6F, 0x63, 0x6B, 0x65, 0x74, 0x00, 0x63, 0x6C, 0x6F, 0x73, 0x65,
  0x00, 0x53, 0x6F, 0x63, 0x6B, 0x65, 0x74, 0x5F, 0x63, 0x6C, 0x6F, 0x73, 0x65, 0x00, 0x65, 0x72,
  0x72, 0x6F, 0x72, 0x00, 0x53, 0x6F, 0x63, 0x6B, 0x65, 0x74, 0x5F, 0x65, 0x72, 0x72, 0x6F, 0x72,
  0x00, 0x73, 0x65, 0x6E, 0x64, 0x00, 0x53, 0x6F, 0x63, 0x6B, 0x65, 0x74, 0x5F, 0x73, 0x65, 0x6E,
  0x64, 0x00, 0x63, 0x6C, 0x6F, 0x73, 0x65, 0x64, 0x00, 0x53, 0x6F, 0x63, 0x6B, 0x65, 0x74, 0x5F,
  0x67, 0x65, 0x74, 0x5F, 0x63, 0x6C, 0x6F, 0x73, 0x65, 0x64, 0x00, 0x61, 0x64, 0x64, 0x50, 0x72,
  0x6F, 0x70, 0x65, 0x72, 0x74, 0x79, 0x00, 0x54, 0x65, 0x73, 0x74, 0x00, 0x61, 0x64, 0x76, 0x61,
  0x6E, 0x63, 0x65, 0x00, 0x54, 0x65, 0x73, 0x74, 0x5F, 0x61, 0x64, 0x76, 0x61, 0x6E, 0x63, 0x65,
  0x00, 0x6D, 0x6F, 0x75, 0x73, 0x65, 0x5F, 0x6D, 0x6F, 0x76, 0x65, 0x00, 0x54, 0x65, 0x73, 0x74,
  0x5F, 0x6D, 0x6F, 0x75, 0x73, 0x65, 0x5F, 0x6D, 0x6F, 0x76, 0x65, 0x00, 0x6D, 0x6F, 0x75, 0x73,
  0x65, 0x5F, 0x70, 0x72, 0x65, 0x73, 0x73, 0x00, 0x54, 0x65, 0x73, 0x74, 0x5F, 0x6D, 0x6F, 0x75,
  0x73, 0x65, 0x5F, 0x70, 0x72, 0x65, 0x73, 0x73, 0x00, 0x6D, 0x6F, 0x75, 0x73, 0x65, 0x5F, 0x72,
  0x65, 0x6C, 0x65, 0x61, 0x73, 0x65, 0x00, 0x54, 0x65, 0x73, 0x74, 0x5F, 0x6D, 0x6F, 0x75, 0x73,
  0x65, 0x5F, 0x72, 0x65, 0x6C, 0x65, 0x61, 0x73, 0x65, 0x00, 0x72, 0x65, 0x6E, 0x64, 0x65, 0x72,
  0x00, 0x54, 0x65, 0x73, 0x74, 0x5F, 0x72, 0x65, 0x6E, 0x64, 0x65, 0x72, 0x00, 0x72, 0x65, 0x73,
  0x65, 0x74, 0x00, 0x54, 0x65, 0x73, 0x74, 0x5F, 0x72, 0x65, 0x73, 0x65, 0x74, 0x00, 0x72, 0x61,
  0x74, 0x65, 0x00, 0x54, 0x65, 0x73, 0x74, 0x5F, 0x67, 0x65, 0x74, 0x5F, 0x72, 0x61, 0x74, 0x65,
  0x00, 0x71, 0x75, 0x69, 0x74, 0x00, 0x54, 0x65, 0x73, 0x74, 0x5F, 0x67, 0x65, 0x74, 0x5F, 0x71,
  0x75, 0x69, 0x74, 0x00, 0x74, 0x72, 0x61, 0x63, 0x65, 0x00, 0x54, 0x65, 0x73, 0x74, 0x5F, 0x67,
  0x65, 0x74, 0x5F, 0x74, 0x72, 0x61, 0x63, 0x65, 0x00, 0x6C, 0x61, 0x75, 0x6E, 0x63, 0x68, 0x65,
  0x64, 0x00, 0x54, 0x65, 0x73, 0x74, 0x5F, 0x67, 0x65, 0x74, 0x5F, 0x6C, 0x61, 0x75, 0x6E, 0x63,
  0x68, 0x65, 0x64, 0x00, 0x70, 0x72, 0x69, 0x6E, 0x74, 0x00, 0x73, 0x00, 0x49, 0x4E, 0x46, 0x4F,
  0x3A, 0x20, 0x00, 0x45, 0x52, 0x52, 0x4F, 0x52, 0x3A, 0x20, 0x00, 0x96, 0x04, 0x00, 0x08, 0x00,
  0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x00, 0x4E, 0x1D, 0x96, 0x02, 0x00, 0x08, 0x00, 0x1C,
  0x96, 0x04, 0x00, 0x08, 0x02, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x03, 0x4E, 0x4F, 0x96,
  0x02, 0x00, 0x08, 0x00, 0x1C, 0x96, 0x07, 0x00, 0x08, 0x04, 0x07, 0x00, 0x00, 0x00, 0x00, 0x43,
  0x4F, 0x96, 0x02, 0x00, 0x08, 0x00, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x04, 0x00,
  0x08, 0x05, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x06, 0x4E, 0x4F, 0x96, 0x02, 0x00, 0x08,
  0x00, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x04, 0x00, 0x08, 0x07, 0x08, 0x01, 0x1C,
  0x96, 0x02, 0x00, 0x08, 0x08, 0x4E, 0x4F, 0x96, 0x02, 0x00, 0x08, 0x00, 0x1C, 0x96, 0x02, 0x00,
  0x08, 0x04, 0x4E, 0x96, 0x04, 0x00, 0x08, 0x09, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x0A,
  0x4E, 0x4F, 0x96, 0x02, 0x00, 0x08, 0x00, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x04,
  0x00, 0x08, 0x0B, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x0C, 0x4E, 0x4F, 0x96, 0x04, 0x00,
  0x08, 0x0D, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x0D, 0x4E, 0x1D, 0x96, 0x02, 0x00, 0x08,
  0x0D, 0x1C, 0x96, 0x07, 0x00, 0x08, 0x04, 0x07, 0x00, 0x00, 0x00, 0x00, 0x43, 0x4F, 0x96, 0x02,
  0x00, 0x08, 0x0D, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x04, 0x00, 0x08, 0x0E, 0x08,
  0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x0F, 0x4E, 0x4F, 0x96, 0x02, 0x00, 0x08, 0x0D, 0x1C, 0x96,
  0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x04, 0x00, 0x08, 0x10, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00,
  0x08, 0x11, 0x4E, 0x4F, 0x96, 0x02, 0x00, 0x08, 0x12, 0x9B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x1D, 0x96, 0x02, 0x00, 0x08, 0x12, 0x1C, 0x96, 0x07, 0x00, 0x08, 0x04, 0x07, 0x00, 0x00,
  0x00, 0x00, 0x43, 0x4F, 0x96, 0x02, 0x00, 0x08, 0x12, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E,
  0x96, 0x04, 0x00, 0x08, 0x13, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x14, 0x4E, 0x4F, 0x96,
  0x02, 0x00, 0x08, 0x12, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x04, 0x00, 0x08, 0x15,
  0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x16, 0x4E, 0x4F, 0x96, 0x02, 0x00, 0x08, 0x12, 0x1C,
  0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x04, 0x00, 0x08, 0x17, 0x08, 0x01, 0x1C, 0x96, 0x02,
  0x00, 0x08, 0x18, 0x4E, 0x4F, 0x96, 0x03, 0x00, 0x02, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08,
  0x1A, 0x4E, 0x96, 0x09, 0x00, 0x08, 0x19, 0x07, 0x03, 0x00, 0x00, 0x00, 0x08, 0x12, 0x1C, 0x96,
  0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x02, 0x00, 0x08, 0x1B, 0x52, 0x17, 0x96, 0x04, 0x00, 0x08,
  0x1C, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x1C, 0x4E, 0x1D, 0x96, 0x02, 0x00, 0x08, 0x1C,
  0x1C, 0x96, 0x07, 0x00, 0x08, 0x04, 0x07, 0x00, 0x00, 0x00, 0x00, 0x43, 0x4F, 0x96, 0x02, 0x00,
  0x08, 0x1C, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x04, 0x00, 0x08, 0x1D, 0x08, 0x01,
  0x1C, 0x96, 0x02, 0x00, 0x08, 0x1E, 0x4E, 0x4F, 0x96, 0x02, 0x00, 0x08, 0x1C, 0x1C, 0x96, 0x02,
  0x00, 0x08, 0x04, 0x4E, 0x96, 0x04, 0x00, 0x08, 0x1F, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08,
  0x20, 0x4E, 0x4F, 0x96, 0x02, 0x00, 0x08, 0x1C, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96,
  0x04, 0x00, 0x08, 0x21, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x22, 0x4E, 0x4F, 0x96, 0x02,
  0x00, 0x08, 0x1C, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x04, 0x00, 0x08, 0x23, 0x08,
  0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x24, 0x4E, 0x4F, 0x96, 0x02, 0x00, 0x08, 0x1C, 0x1C, 0x96,
  0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x04, 0x00, 0x08, 0x25, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00,
  0x08, 0x26, 0x4E, 0x4F, 0x96, 0x02, 0x00, 0x08, 0x1C, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E,
  0x96, 0x04, 0x00, 0x08, 0x27, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x28, 0x4E, 0x4F, 0x96,
  0x03, 0x00, 0x02, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x2A, 0x4E, 0x96, 0x09, 0x00, 0x08,
  0x29, 0x07, 0x03, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96,
  0x02, 0x00, 0x08, 0x1B, 0x52, 0x17, 0x96, 0x03, 0x00, 0x02, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00,
  0x08, 0x2C, 0x4E, 0x96, 0x09, 0x00, 0x08, 0x2B, 0x07, 0x03, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x1C,
  0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x02, 0x00, 0x08, 0x1B, 0x52, 0x17, 0x96, 0x03, 0x00,
  0x02, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x2E, 0x4E, 0x96, 0x09, 0x00, 0x08, 0x2D, 0x07,
  0x03, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x04, 0x4E, 0x96, 0x02, 0x00,
  0x08, 0x1B, 0x52, 0x17, 0x96, 0x03, 0x00, 0x02, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x30,
  0x4E, 0x96, 0x09, 0x00, 0x08, 0x2F, 0x07, 0x03, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x1C, 0x96, 0x02,
  0x00, 0x08, 0x04, 0x4E, 0x96, 0x02, 0x00, 0x08, 0x1B, 0x52, 0x17, 0x96, 0x02, 0x00, 0x08, 0x31,
  0x9B, 0x07, 0x00, 0x00, 0x01, 0x00, 0x73, 0x00, 0x27, 0x00, 0x96, 0x02, 0x00, 0x08, 0x32, 0x1C,
  0x12, 0x9D, 0x02, 0x00, 0x1B, 0x00, 0x96, 0x04, 0x00, 0x08, 0x33, 0x08, 0x32, 0x1C, 0x47, 0x96,
  0x07, 0x00, 0x07, 0x01, 0x00, 0x00, 0x00, 0x08, 0x01, 0x1C, 0x96, 0x02, 0x00, 0x08, 0x31, 0x52,
  0x17, 0x1D, 0x96, 0x02, 0x00, 0x08, 0x15, 0x9B, 0x07, 0x00, 0x00, 0x01, 0x00, 0x73, 0x00, 0x27,
  0x00, 0x96, 0x02, 0x00, 0x08, 0x32, 0x1C, 0x12, 0x9D, 0x02, 0x00, 0x1B, 0x00, 0x96, 0x04, 0x00,
  0x08, 0x34, 0x08, 0x32, 0x1C, 0x47, 0x96, 0x07, 0x00, 0x07, 0x01, 0x00, 0x00, 0x00, 0x08, 0x01,
  0x1C, 0x96, 0x02, 0x00, 0x08, 0x31, 0x52, 0x17, 0x1D, 0x00
};

static const unsigned char null_text[] = "";

void send_headers(char* mime, int length) {
    printf("%s %d %s\r\n", "HTTP/1.1", 200, "OK");
    if (mime) {
        printf("Content-Type: %s\r\n", mime);
    }        

    if (length >= 0) {
        printf("Accept-ranges: bytes\r\n");        
        printf("Content-Length: %d\r\n", length);
    }        
    printf("Connection: close\r\n");
    printf("\r\n");
}

char* get_mime_type(char* ext) {
    // This is really crude, but also really simple ;-)
    if (!ext) return DEFAULT_MIMETYPE;
    if (strcasecmp(ext, ".gif") == 0)                                   return "image/gif";        
    if (strcasecmp(ext, ".png") == 0)                                   return "image/png";            
    if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0)  return "image/jpeg";
    if (strcasecmp(ext, ".swf") == 0)                                   return "application/x-shockwave-flash";            
    if (strcasecmp(ext, ".js") == 0 || strcasecmp(ext, ".ajax") == 0)   return "text/javascript";       
    if (strcasecmp(ext, ".htm") == 0 || strcasecmp(ext, ".html") == 0)  return "text/html";
    if (strcasecmp(ext, ".css") == 0)                                   return "text/css";        
    if (strcasecmp(ext, ".php") == 0)                                   return "text/plain";        
    if (strcasecmp(ext, ".asp") == 0 || strcasecmp(ext, ".aspx") == 0)  return "text/plain";    
    if (strcasecmp(ext, ".cgi") == 0 || strcasecmp(ext, ".pl") == 0)    return "text/plain";
    if (strcasecmp(ext, ".txt") == 0)                                   return "text/plain";            
    return DEFAULT_MIMETYPE;  
}

int main(int argc, char * argv[]) {
    char buf[4096];
    char* method;
    char* path;
    char* protocol;
    char* querystring;
    char* subpath;    
    char* ext; 
    char* mime_type;         

    if (!fgets(buf, sizeof(buf), stdin)) return -1;

    // Get the individual bits of the request
    method = strtok(buf, " ");
    path = strtok(NULL, " ");
    protocol = strtok(NULL, "\r");
 
    // Parse querystring if there is one.
    querystring = strstr(path, "?");
    if (querystring)
        querystring += sizeof(char);
    
    subpath = strtok(path, "?");
    ext = strrchr(subpath, '.');    
    mime_type = get_mime_type(ext);               
    
    /*
    printf("Method   : %s\r\n", method);    
    printf("Path     : %s\r\n", path);    
    printf("Protocol : %s\r\n", protocol);            
    printf("Query    : %s\r\n", querystring);                
    printf("Subpath  : %s\r\n", subpath);                    
    printf("Ext      : %s\r\n", ext);               
    printf("MimeType : %s\r\n", mime_type);                   
    exit(0);
    */          
                        
    if ( (mime_type == "text/plain") || (mime_type == "text/html") || (mime_type == "text/css") || (mime_type == "text/javascript") ) {
        send_headers(mime_type, (sizeof null_text -1));
        fwrite(null_text, 1, (sizeof null_text - 1), stdout);                
    }
    else if (mime_type == "image/gif" ) {
        send_headers(mime_type, (sizeof null_gif -1));
        fwrite(null_gif, 1, (sizeof null_gif - 1), stdout);                    
    }
    else if (mime_type == "image/png" ) {
        send_headers(mime_type, (sizeof null_png -1));
        fwrite(null_png, 1, (sizeof null_png - 1), stdout);                    
    }    
    else if (mime_type == "image/jpeg" ) {
        send_headers(mime_type, (sizeof null_jpg -1));
        fwrite(null_jpg, 1, (sizeof null_jpg - 1), stdout);                    
    }    
    else if (mime_type == "application/x-shockwave-flash" ) {
        send_headers(mime_type, (sizeof null_swf -1));
        fwrite(null_swf, 1, (sizeof null_swf - 1), stdout);                    
    }    
    else {
        return -1;
    }  
    return 0;
}
