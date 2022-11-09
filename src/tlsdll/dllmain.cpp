#define TLS_PAYLOAD_SIZE 0x2000
thread_local char tls_data[TLS_PAYLOAD_SIZE];

__declspec(dllexport) void* get_tls_data()
{
	return &tls_data[0];
}
