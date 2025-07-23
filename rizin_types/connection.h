struct connection_info{
	uint32_t socket_fd;
	uint8_t pad;
	uint8_t sin_family;
	uint16_t sin_port;
	uint32_t sin_addr;
	uint8_t sin_zero[8];
	uint32_t addr_len;
};
