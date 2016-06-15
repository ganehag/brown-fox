#ifndef httpdata
#define httpdata

#define HTTP_ERR	0
#define HTTP_OK		2

void httpsend(char *host, char *payoad);

uint8_t http_response(char *bigbuf);

#endif