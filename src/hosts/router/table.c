#include "hosts/router/table.h"


/**
 * Tabela de mapeamentos entre endereços MAC de hosts da rede e portas digitais. Através desta
 * tabela, é possível que um host da rede envie um comando para controlar um relé atrelado
 * a uma porta digital do gateway.
 *
 * A tabela fica armazenada na memória flash, mas poderá ser atualizada através de um comando
 * especial via ethernet ou conexão serial.
 */
const table_entry_t entries[200] =
	{ { 1, 11, 1, 0 },
	  { 1, 21, 1, 0 },
	  { 1, 31, 1, 0 },
	  { 5, 41, 1, 0 },
	  { 5, 51, 1, 0 } };


uint8_t table_findFirst(
	uint16_t mac )
{
	// itera na tabela em busca do endereço MAC
	uint8_t min = 0;
	uint8_t max = (uint8_t)sizeof(entries) - 1;
	uint8_t pos = (uint8_t)sizeof(entries) / 2;
	do {
		if (entries[pos].active == 1 &&
		    entries[pos].macAddress == mac)
			break;
		if (entries[pos].active == 1 &&
		    entries[pos].macAddress < mac)
			min = pos + 1;
		else
			max = pos - 1;
		pos = min + (max - min) / 2;
	} while (min != max);
	// verifica se encontrou alguma ocorrência
	if (entries[pos].active == 0 || entries[pos].macAddress != mac) return 0xFF;
	// verifica se existe alguma entrada anterior com o mesmo endereço MAC
	while (pos > 0 && entries[pos-1].macAddress == mac) pos--;
	return pos;
}


