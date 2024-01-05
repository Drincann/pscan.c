#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// Command-line Interface
typedef struct {
    int *ports;
    int count;
} PortList;


void printf_and_flush(const char *st, ...);
void print_usage();
void parse_cmd_options(int argc, char **argv, char **host, long *timeout, char **port_input);
PortList parse_ports(const char *port_input);

// Utility Functions
bool is_number(const char *str);
bool is_valid_port(int port);

// Core Functionality
bool is_port_open(const char* ip, int port, long timeout_ms);
void scan_range(const char *ip, long timeout_ms, int start_port, int end_port);
void execute_scan(const char *ip, long timeout_ms, const PortList *port_list);
void validate_and_scan_ports(const char *ip, long timeout_ms, char *port_input);

int main(int argc, char **argv) {
    char *host = NULL;
    long timeout = 500; // Default timeout
    char *port_input = NULL;

    parse_cmd_options(argc, argv, &host, &timeout, &port_input);

    if (!host) {
        printf("Host is required.\n");
        print_usage();
        return EXIT_FAILURE;
    }

    validate_and_scan_ports(host, timeout, port_input);

    return 0;
}

void parse_cmd_options(int argc, char **argv, char **host, long *timeout, char **port_input) {
    int opt;
    static struct option long_options[] = {
        {"host", required_argument, 0, 'h'},
        {"timeout", optional_argument, 0, 't'},
        {"port", optional_argument, 0, 'p'},
        {"help", no_argument, 0, 0},
        {0, 0, 0, 0}
    };

    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "h:t:p:", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h':
                *host = optarg;
                break;
            case 't':
                *timeout = strtol(optarg, NULL, 10);
                break;
            case 'p':
                *port_input = optarg;
                break;
            case 0:
                if (strcmp(long_options[long_index].name, "help") == 0) {
                    print_usage();
                    exit(0);
                }
                break;
            default:
                print_usage();
                exit(EXIT_FAILURE);
        }
    }
}

void print_usage() {
    printf("Usage: portscanner -h <host> [-t <timeout in ms>] [-p <port range or list>]\n");
    printf("Options:\n");
    printf("  -h, --host     Specify the host IP to scan\n");
    printf("  -t, --timeout  Set timeout in milliseconds (default 500ms)\n");
    printf("  -p, --port     Specify a single port, a list of ports, or a range (e.g., 80, 8080-8081)\n");
    printf_and_flush("  --help         Display this help message\n");
}

void validate_and_scan_ports(const char *ip, long timeout_ms, char *port_input) {
    // 默认端口列表，如果没有指定
    const char *default_ports = "22,80,443";
    char *ports_to_scan = port_input ? strdup(port_input) : strdup(default_ports);

    PortList port_list = parse_ports(ports_to_scan);
    execute_scan(ip, timeout_ms, &port_list);

    free(ports_to_scan);
    free(port_list.ports);
}

PortList parse_ports(const char *port_input) {
    PortList result;
    result.ports = malloc(65536 * sizeof(int)); // 大小足以容纳所有端口
    result.count = 0;

    char *input_copy = strdup(port_input);
    char *token = strtok(input_copy, ",");
    while (token != NULL) {
        if (strchr(token, '-')) {
            // 解析范围
            char *start_str = strtok(token, "-");
            char *end_str = strtok(NULL, "-");
            int start = atoi(start_str);
            int end = atoi(end_str);
            if (is_valid_port(start) && is_valid_port(end) && start <= end) {
                for (int port = start; port <= end; port++) {
                    result.ports[result.count++] = port;
                }
            }
        } else {
            // 单个端口
            int port = atoi(token);
            if (is_valid_port(port)) {
                result.ports[result.count++] = port;
            }
        }
        token = strtok(NULL, ",");
    }

    free(input_copy);
    return result;
}

void execute_scan(const char *ip, long timeout_ms, const PortList *port_list) {
    for (int i = 0; i < port_list->count; i++) {
        int port = port_list->ports[i];
        if (is_port_open(ip, port, timeout_ms)) {
            printf_and_flush("Port %d is open\n", port);
        } else {
            printf_and_flush("Port %d is closed or not responding\n", port);
        }
    }
}


void scan_range(const char *ip, long timeout_ms, int start_port, int end_port) {
    for (int port = start_port; port <= end_port; port++) {
        if (is_port_open(ip, port, timeout_ms)) {
            printf_and_flush("Port %d is open\n", port);
        } else {
            printf_and_flush("Port %d is closed or not responding\n", port);
        }
    }
}

bool is_port_open(const char* ip, int port, long timeout_ms) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &(addr.sin_addr));

    connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1) {
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
        close(sockfd);
        return so_error == 0;
    }

    close(sockfd);
    return false;
}

void printf_and_flush(const char *str, ...) {
    va_list args;
    va_start(args, str);
    vprintf(str, args);
    va_end(args);
    fflush(stdout);
}

bool is_number(const char *str) {
    while (*str) {
        if (!isdigit(*str))
            return false;
        str++;
    }
    return true;
}

bool is_valid_port(int port) {
    return port > 0 && port <= 65535;
}
