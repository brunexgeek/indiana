#ifndef SIMPLE_HTTP_HTTP_H
#define SIMPLE_HTTP_HTTP_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "global.h"


typedef enum 
{
	HTTP10 = 0x10,
	HTTP11 = 0x11
} http_version_t;

typedef struct
{
	Uint8 data[MAX_BLOCK_SIZE];
	Uint32 length;
	void *next;
} http_buffer_t;

typedef struct
{
	char *name;
	char *value;
	void *next;
} http_header_entry_t;

typedef struct
{
	http_header_entry_t *entries;
	Uint8 count;
} http_header_t;

typedef struct
{
	http_buffer_t *entries;
	void *next;
} http_content_t;

typedef struct
{
	http_version_t version;
	http_header_t header;
	http_content_t content;
	
} http_request_t;

typedef struct
{
	http_version_t version;
	Uint8 status;
	char *statusName;
	http_header_t header;
	http_content_t content;
} http_response_t;

typedef struct
{
	char *host;
	Uint8 port;
	char *userAgent;
	Uint8 flags;
	//socket_t socket;
} http_connection_t;


BEGIN_EXTERN_C

SO_PRIVATE Uint8 http_initialize();

SO_PRIVATE Uint8 http_terminate();

SO_PRIVATE Uint8 http_createConnection(
	http_connection_t *connection,
	const char *host,
	Uint8 port,
	const char *userAgent,
	Uint8 flags );

SO_PRIVATE Uint8 http_destroyConnection(
	http_connection_t *connection );

SO_PRIVATE Uint8 http_createRequest(
	http_request_t *request,
	http_version_t version );

SO_PRIVATE Uint8 http_destroyRequest(
	http_request_t *request );

SO_PRIVATE Uint8 http_setRequestHeader(
	http_request_t *request,
	const char *fieldName,
	const char *fieldValue );

SO_PRIVATE Uint8 http_removeRequestHeader(
	http_request_t *request,
	const char *fieldName );

SO_PRIVATE Uint8 http_sendRequest(
	http_connection_t *connection,
	const char *resource,
	const http_request_t *request,
	Uint8 flags );

END_EXTERN_C

#endif  // SIMPLE_HTTP_HTTP_H
