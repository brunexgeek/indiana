#include "http.h"


static Uint8 http_equal(
	const char *string1,
	const char *string2 )
{
	char x, y;
	int i;
	
	if (string1 == 0 || string2 == 0) return 0;
	
	for (i = 0; i < strlen(string1); ++i)
		if ( tolower(string1[i]) != tolower(string2[i]) ) return 0;
	
	return 1;
}

static Uint8 http_setHeader(
	http_header_t *headers,
	const char *fieldName,
	const char *fieldValue )
{
	http_header_entry_t *field;
	Uint8 error;

	if (headers == 0 || fieldName == 0) return HTTPERR_INVALID_ARGUMENT;
	if (fieldValue == 0) fieldValue = "";
	
	field = malloc( sizeof(http_header_t) );
	if (field == 0) return HTTPERR_OUT_OF_MEMORY;
	
	field->name = malloc( strlen(fieldName) + 1 );
	if (field->name == 0) ESCAPE_WITH_ERROR(HTTPERR_OUT_OF_MEMORY);
	field->value = malloc( strlen(fieldValue) +1 );
	if (field->value == 0) ESCAPE_WITH_ERROR(HTTPERR_OUT_OF_MEMORY);
	
	strcpy(field->name, fieldName);
	strcpy(field->value, fieldValue);
	if (field->name[ strlen(fieldName) -1 ] == ':')
		field->name[ strlen(fieldName) -1 ] = 0;
	
	field->next = headers->entries;
	headers->entries = field;
	
	return HTTPERR_NONE;
ESCAPE:
	if (field != 0 && field->name != 0) free(field->name);
	if (field != 0 && field->value != 0) free(field->value);
	if (field != 0) free(field);
	
	return error;
}


static Uint8 http_removeHeader(
	http_header_t *headers,
	const char *fieldName )
{
	http_header_entry_t *prev = 0, *header = 0;
	
	if (headers == 0 || fieldName == 0) 
		return HTTPERR_INVALID_ARGUMENT;
	
	header = headers->entries;
	while (header != 0)
	{
		if (http_equal(header->name, fieldName) != 0)
		{
			// remove a entrada da lista
			if (prev != 0) 
				prev->next = header->next;
			else
				headers->entries = header->next;
			break;
		}
		prev = header;
		header = header->next;
	}
	// libera a memória alocada pela entrada
	if (header != 0)
	{
		if (header->name != 0) free(header->name);
		if (header->value != 0) free(header->value);
		free(header);
		headers->count--;
	}

	return HTTPERR_NONE;
}

char *http_strtok(
	char * text, 
	const char * delim, 
	char **ptr )
{
	char *v, *p;

	if (ptr == 0 || (text == 0 && *ptr == 0)) return 0;
	p = *ptr;
	if (text != 0) p = text;

	// procura pelo primeiro caracter válido
	//while (*p != 0 && strchr(delim, *p) != 0) p++;
	v = p;
	// procura pelo próximo separador
	while (*p != 0 && strchr(delim, *p) == 0) p++;
	// retorna a posição a partir da qual deve continuar na próxima
	// chamada
	if (*p != 0) 
		*ptr = p + 1;
	else
		*ptr = 0;
	// substitui o separador por um zero
	*p = 0;

	return v;
}

static Uint8 http_parseResponse(
	char *data,
	http_response_t *response )
{
	char *line, *ptr = 0;
	int status;
	char version[12];
	char fieldName[128];
	char fieldValue[1024]; 
	
	if (response == 0 || data == 0) return HTTPERR_INVALID_ARGUMENT;
	memset(response, 0, sizeof(http_response_t));
	
	line = http_strtok(data, "\n", &ptr);
	if (sscanf(line, "%s %d", version, &status) == 0)
		return HTTPERR_INVALID_DATA;
	response->status = status;
	
	printf("http.version = %s\n", version);
	printf("http.status = %d\n", status);
	
	do {
		line = http_strtok(0, "\n", &ptr);
		if (*line != 0)
		{
			sscanf(line, "%s %s", fieldName, fieldValue);
			http_setHeader(&response->header, fieldName, fieldValue);
			printf("%s: %s\n", fieldName, fieldValue);
		}
	} while (*line != 0);
	
	return 0;
}


Uint8 http_createConnection(
	http_connection_t *connection,
	const char *host,
	Uint8 port,
	const char *userAgent,
	Uint8 flags )
{
	Uint8 error;

	if (connection == 0 || host == 0) return HTTPERR_INVALID_ARGUMENT;
	if (port == 0) port = 80;
	memset(connection, 0, sizeof(http_connection_t));

	// copia o endereço do servidor
	connection->host = malloc(strlen(host) + 1);
	if (connection->host == 0) ESCAPE_WITH_ERROR(HTTPERR_OUT_OF_MEMORY);
	strcpy(connection->host, (char*)host);
	// copia a porta
	connection->port = port;
	// copia o agente de usuário
	connection->userAgent = malloc(strlen(userAgent) + 1);
	if (connection->userAgent == 0) ESCAPE_WITH_ERROR(HTTPERR_OUT_OF_MEMORY);
	strcpy(connection->userAgent, (char*)userAgent);
	// copia as flags
	connection->flags = flags;

	return HTTPERR_NONE;
ESCAPE:
	if (connection->host != 0) free(connection->host);
	if (connection->userAgent != 0) free(connection->userAgent);
	
	return error;
}

Uint8 http_destroyConnection(
	http_connection_t *connection )
{
	if (connection == 0) return HTTPERR_INVALID_ARGUMENT;
	if (connection->host != 0) free(connection->host);
	if (connection->userAgent != 0) free(connection->userAgent);
	memset(connection, 0, sizeof(http_connection_t));
	return HTTPERR_NONE;
}

Uint8 http_createRequest(
	http_request_t *request,
	http_version_t version )
{
	Uint8 error;

	if (request == 0) return HTTPERR_INVALID_ARGUMENT;
	memset(request, 0, sizeof(http_request_t));

	// copia a versão do HTTP
	request->version = version;

	return HTTPERR_NONE;
}

Uint8 http_destroyRequest(
	http_request_t *request )
{
	void *ptr = 0;
	
	if (request == 0) return HTTPERR_INVALID_ARGUMENT;
	
	// libera a memória alocada pelos campos de cabeçalho
	while (request->header.entries != 0)
	{
		ptr = request->header.entries->next;
		free(request->header.entries->name);
		free(request->header.entries->value);
		free(request->header.entries);
		request->header.entries = ptr;
	}
	// libera a memória alocada pelo conteúdo
	while (request->content.entries != 0)
	{
		ptr = request->content.entries->next;
		free(request->content.entries);
		request->content.entries = ptr;
	}
	memset(request, 0, sizeof(http_request_t));
	
	return HTTPERR_NONE;
}

Uint8 http_setRequestHeader(
	http_request_t *request,
	const char *fieldName,
	const char *fieldValue )
{
	http_header_entry_t *field;
	Uint8 error;

	if (request == 0 || fieldName == 0) 
		return HTTPERR_INVALID_ARGUMENT;
	
	return http_setHeader(&request->header, fieldName, fieldValue);
}

Uint8 http_removeRequestHeader(
	http_request_t *request,
	const char *fieldName )
{
	http_header_entry_t *prev = 0, *header = 0;
	if (request == 0 || fieldName == 0) 
		return HTTPERR_INVALID_ARGUMENT;
	
	return http_removeHeader(&request->header, fieldName);
}

Uint8 http_sendRequest(
	http_connection_t *connection,
	const char *resource,
	const http_request_t *request,
	Uint8 flags )
{
	http_header_entry_t *header;
	
	if (resource == 0 || connection == 0 || request == 0)
		return HTTPERR_INVALID_ARGUMENT;
	
	printf("\nGET %s HTTP/1.%c\n", resource, 		
		(request->version == HTTP10) ? '0' : '1');
	
	header = request->header.entries;
	while (header != 0)
	{
		printf("%s: %s\n", header->name, header->value);
		header = header->next;
	}
	printf("User-Agent: %s\n\n", connection->userAgent);
	
	return HTTPERR_NONE;
}


int main( int argc, char **argv )
{
	http_request_t request;
	http_connection_t connection;
	http_response_t response;
	
	/*http_createConnection(&connection, "license.cpqd.com.br", 80,
		"CPqD TextoFala", 0);
	
	http_createRequest(&request, HTTP10);
	http_setRequestHeader(&request, "X-CPqD-License-Code", "d0f5cb0r8g4gs6");
	http_setRequestHeader(&request, "Content-Type", "text/plain");
	http_removeRequestHeader(&request, "X-CPqD-License-Code");
	
	http_sendRequest(&connection, "/register", &request, 0);
	
	http_destroyRequest(&request);
	http_destroyConnection(&connection);*/
	
	char *ptr = 0, text[256] = "HTTP/1.0 200 OK\nDate: Fri, 31 Dec 1999 23:59:59 GMT\nContent-Type: text/html\nContent-Length: 1354\n\n", *r;
		
	http_parseResponse(text, &response);
}
