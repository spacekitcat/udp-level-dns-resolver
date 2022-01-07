#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <resolv.h>

#include "hexdump.h"

#define DNS_QTYPE_A 1
#define DNS_QCLASS_IN 1
#define STR_BUFFER_SIZE 255

struct sockaddr_in getDnsSocketAddress()
{
	return _res.nsaddr_list[0];
}

struct dns_header
{
	// https://datatracker.ietf.org/doc/html/rfc1035#section-4.1.1
	uint16_t tx_id;

	// <two byte DNS response/request flags>
	unsigned char recursion_desired : 1;
	unsigned char truncated : 1;
	unsigned char authoritive_answer : 1;
	unsigned char opcode : 4;
	unsigned char is_response : 1;
	unsigned char response_code : 4;
	unsigned char checking_disabled : 1;
	unsigned char answer_authenticatedd : 1;
	unsigned char z : 1;
	unsigned char recursion_availablw : 1;
	// </two byte DNS response/request flags>

	uint16_t question_count;
	uint16_t answer_rr_count;
	uint16_t authority_rr_count;
	uint16_t extra_rr_count;
} __attribute__((packed));

struct dns_question_label_section
{
	uint8_t size;
} __attribute__((packed));

struct dns_payload
{
	struct dns_header header;
} __attribute__((packed));

void *appendQueryLabel(void *dns_payload_ptr_it, char *target_zone, int target_zone_length)
{
	int target_zone_label_size = sizeof(struct dns_question_label_section) + target_zone_length;
	void *target_zone_label = malloc(target_zone_label_size);
	((struct dns_question_label_section *)target_zone_label)->size = target_zone_length;
	strncpy((char *)((struct dns_question_label_section *)target_zone_label + 1), target_zone, target_zone_length);
	memcpy(dns_payload_ptr_it, target_zone_label, target_zone_label_size);
	return dns_payload_ptr_it + target_zone_label_size;
}

int count_tokens(char *str_buffer, char delim)
{
	int token_count = 0;
	char c;
	for (int i = 0; i < strlen(str_buffer); ++i)
	{
		if (str_buffer[i] == delim)
		{
			if (i < strlen(str_buffer) - 1 && str_buffer[i + 1] != delim)
			{
				++token_count;
			}
		}
	}

	return token_count;
}

char **parse_tokens(char *str_buffer, const char *delim, const int token_count)
{
	char **tokens = malloc(sizeof(char *) * token_count);
	int token_idx = 0;

	char *token = strtok(str_buffer, delim);

	while (token != NULL)
	{
		tokens[token_idx] = malloc(strlen(token) + 1);
		strcpy(tokens[token_idx], token);

		++token_idx;
		token = strtok(NULL, delim);
	}

	return tokens;
}

int count_string_array_sizeof(char **str_matrix, const int string_count)
{
	int zoneLabelLengths = 0;
	for (int i = 0; i < string_count; ++i)
	{
		if (str_matrix[i] != NULL)
		{
			zoneLabelLengths += strlen(str_matrix[i]);
		}
	}

	return zoneLabelLengths;
}

int main(int argc, char *argv[])
{
	int zone_label_count;
	char **zone_label_list;

	if (argc == 2)
	{
		// +1 for the terminating label (or root zone if you prefer to think of it that way)
		zone_label_count = count_tokens(argv[1], '.') + 1;
		zone_label_list = parse_tokens(argv[1], ".", zone_label_count);
	}
	else
	{
		fprintf(stderr, "Error: The domain name was not specified as the first argument as expected\n");
		return 1;
	}

	res_init();

	int udp_socket;
	if ((udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		return 1;
	}

	struct dns_header header =
			{
					.tx_id = htons(0x99e2),
					.question_count = htons(1),
					.answer_rr_count = htons(0),
					.authority_rr_count = htons(0),
					.extra_rr_count = htons(0),
					.is_response = 0x0,
					.recursion_desired = 0x1,
					.truncated = 0x0,
					.z = 0x0,
			};

	int zoneLabelLengths = count_string_array_sizeof(zone_label_list, zone_label_count);
	int query_label_section_size = sizeof(uint16_t) * 2;
	int terminating_char = sizeof(uint8_t);
	int total_dns_payload_size = sizeof(struct dns_payload) + zone_label_count + zoneLabelLengths + query_label_section_size + terminating_char;

	void *dns_query = malloc(total_dns_payload_size);
	char *dns_payload_ptr_it = dns_query;
	memcpy(dns_payload_ptr_it, &header, sizeof(header));
	dns_payload_ptr_it += sizeof(header);

	// Transfer zone query labels to dns payload
	for (int i = 0; i < zone_label_count; ++i)
	{
		if (zone_label_list[i] != NULL)
		{
			dns_payload_ptr_it = appendQueryLabel(dns_payload_ptr_it, zone_label_list[i], strlen(zone_label_list[i]));
		}
	}
	dns_payload_ptr_it = appendQueryLabel(dns_payload_ptr_it, "", 0);

	// Set 16-bit QTYPE field
	uint16_t dns_qtype = htons(DNS_QTYPE_A);
	memcpy(dns_payload_ptr_it, &dns_qtype, 2);
	dns_payload_ptr_it += 2;
	// Set 16-bit QCLASS field
	uint16_t dns_qclass = htons(DNS_QCLASS_IN);
	memcpy(dns_payload_ptr_it, &dns_qclass, 2);
	dns_payload_ptr_it += 2;

	/** Request transmission **/
	printf("\n\nDNS query payload: \n");
	hexDump(dns_query, total_dns_payload_size);

	struct sockaddr_in dns_server = getDnsSocketAddress();
	socklen_t dns_server_len = sizeof(struct sockaddr_in);
	if (sendto(udp_socket, dns_query, total_dns_payload_size, 0, (struct sockaddr *)&dns_server, dns_server_len) < 0)
	{
		fprintf(stderr, "Error: An unexpected error was encountered while attempting to send the UDP packet");
		return 2;
	}

	/** Response **/
	const int response_buffer_size = 512;
	struct dns_payload *_payload = malloc(response_buffer_size);
	ssize_t response_length = recvfrom(udp_socket, _payload, response_buffer_size, MSG_WAITALL, (struct sockaddr *)&dns_server, &dns_server_len);

	printf("\nRecieved %zu bytes.\n", response_length);
	printf("\nDNS response payload: \n");
	hexDump(_payload, response_length);

	/** Shutdown **/
	for (int i = 0; i < zone_label_count; ++i)
	{
		free(zone_label_list[i]);
	}
	free(zone_label_list);
	free(dns_query);
	free(_payload);

	return 0;
}
