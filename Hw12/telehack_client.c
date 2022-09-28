#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <linux/if_link.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <errno.h>

#include <stdio.h>
#include <unistd.h>     // open/close
#include <string.h>     // memcopy
#include <stdint.h>

#define URL "telehack.com"
#define PORT "23"

#define CMD_TELEHACK "figlet /%s %s\r\n"
#define EOF_DATA "\r\n."


const char* const supported_fonts[] = { "3-d", "3x5", "5lineoblique", "acrobatic",
		"alligator", "alligator2", "alphabet", "vatar", "banner", "banner3",
		"banner3-D", "anner4", "barbwire", "basic", "bell", "ig", "bigchief",
		"binary", "block", "ubble", "bulbhead", "calgphy2", "caligraphy",
		"atwalk", "chunky", "coinstak", "colossal", "omputer", "contessa",
		"contrast", "cosmic", "osmike", "cricket", "cursive", "cyberlarge",
		"ybermedium", "cybersmall", "diamond", "digital", "oh", "doom",
		"dotmatrix", "drpepper", "ftichess", "eftifont", "eftipiti",
		"eftirobot", "ftitalic", "eftiwall", "eftiwater", "epic", "ender",
		"fourtops", "fuzzy", "goofy", "othic", "graffiti", "hollywood",
		"invita", "sometric1", "isometric2", "isometric3", "isometric4",
		"talic", "ivrit", "jazmine", "jerusalem", "atakana", "kban", "larry3d",
		"lcd", "ean", "letters", "linux", "lockergnome", "adrid", "marquee",
		"maxfour", "mike", "ini", "mirror", "mnemonic", "morse", "oscow",
		"nancyj", "nancyj-fancy", "nancyj-underlined", "ipples", "ntgreek",
		"o8", "ogre", "awp", "peaks", "pebbles", "pepper", "oison", "puffy",
		"pyramid", "rectangles", "elief", "relief2", "rev", "roman", "ot13",
		"rounded", "rowancap", "rozzo", "unic", "runyc", "sblood", "script",
		"erifcap", "shadow", "short", "slant", "lide", "slscript", "small",
		"smisome1", "mkeyboard", "smscript", "smshadow", "smslant", "mtengwar",
		"speed", "stampatello", "standard", "tarwars", "stellar", "stop",
		"straight", "anja", "tengwar", "term", "thick", "hin", "threepoint",
		"ticks", "ticksslant", "inker-toy", "tombstone", "trek", "tsalagi",
		"wopoint", "univers", "usaflag", "wavy", "eird"
};


void log_error(const char *message) {
	fprintf(stderr, "%s %d: %s\n", message, errno, strerror(errno));
}

void close_socket(int socket) {
	if (socket > 0) {
		close(socket);
	}
}

int open_client_socket(const char *address, const char *port) {
	struct addrinfo hints;
	struct addrinfo *res = NULL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_protocol = 0;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ALL;

	int code = getaddrinfo(address, port, &hints, &res);
	if (code != 0) {
		fprintf(stderr, "Can't resolve address: %s\n", gai_strerror(code));
		return -1;
	}
	const int socket_descriptor = socket(res->ai_family, res->ai_socktype,
			res->ai_protocol);
	if (socket_descriptor == -1) {
		log_error("Create socket failed");
		return -1;
	}
	code = connect(socket_descriptor, res->ai_addr, res->ai_addrlen);
	if (code == -1) {
		freeaddrinfo(res);
		close_socket(socket_descriptor);
		fprintf(stderr, "%d: %s\n", code, gai_strerror(code));
		log_error("Can't connect");
		return -1;
	}
	freeaddrinfo(res);
	return socket_descriptor;
}

int network_send(const int socket, const uint8_t* const message, const int size) {
	return send(socket, message, size, 0);
}

int network_receive(const int socket, uint8_t *buf, const int size) {
	return recv(socket, buf, size, 0);
}

int main(int argc, char *argv[]) {
	char *font = NULL;
	char *text = NULL;
	if (argc == 3) {
		font = argv[1];
		text = argv[2];
	} else {
		printf("Please provide font and text as parameters: ./telehack_client font text\n");
		return 0;
	}
	int found = 0;
	for (uint32_t i = 0; i < sizeof(supported_fonts) / sizeof(supported_fonts[0]); i++) {
		if (strcmp(font, supported_fonts[i]) == 0) {
			found = 1;
			break;
		}
	}
	if (!found) {
		printf("Unsupported font %s. Must be one of the list:\n", font);
		for (uint32_t i = 0; i < sizeof(supported_fonts) / sizeof(supported_fonts[0]); i++) {
			if (i % 4 == 0) {
				printf("%s\n", supported_fonts[i]);
			} else {
				printf("%s\t", supported_fonts[i]);
			}
		}
		return 0;
	}

	int socket = open_client_socket(URL, PORT);
	if (socket > 0) {
        while(1) {
            int command_size = strlen(CMD_TELEHACK) + strlen(font) + strlen(text) + 1 - 4;
            uint8_t* command = NULL;
            command = calloc(sizeof(*command), command_size);
            if (command == NULL) {
                printf("Failed to allocate memory for command\n");
                break;
            }
            if (sprintf((char*) command, CMD_TELEHACK, font, text) <= 0) {
                printf("Failed to setup command\n");
                break;
            }

            uint8_t buffer[256] = { '\0' };
            int received = 0;
            while(1) {
            	received = network_receive(socket, buffer, sizeof(buffer) / sizeof(buffer[0]) - 1);
				if (received < 0) {
					continue;
				}
				buffer[received] = '\0';
				printf("%s", buffer);
				if (strstr((char*) buffer, EOF_DATA) != NULL) {
					break;
				} else {
					memset(buffer, '\0', sizeof(buffer) / sizeof(buffer[0]));
				}
            }

            if (network_send(socket, command, command_size) == -1) {
				log_error("Failed to send command");
				break;
			}
            while(1) {
                received = network_receive(socket, buffer, sizeof(buffer) / sizeof(buffer[0]) - 1);
                if (received < 0) {
                	continue;
                }
				buffer[received] = '\0';
				printf("%s", buffer);
				if (strstr((char*) buffer, EOF_DATA) != NULL) {
					break;
				} else {
					memset(buffer, '\0', sizeof(buffer) / sizeof(buffer[0]));
				}
            }
            break;
        }
	}
	close_socket(socket);
	return 0;
}
