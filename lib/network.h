#include <stdint.h>

enum network_state
{
	NOT_CONNECTED,
	CONNECTED_CLIENT,
	CONNECTED_SERVER,
};

int init_network();
uint64_t init_listen_socket(const char *port);
int init_connect_socket (const char* addr, const char* port);
int close_socket();
enum network_state get_network_state();

int send_data(const char *buf, int length, uint8_t blocking);
int receive_data(char *buf, int length, uint8_t blocking);