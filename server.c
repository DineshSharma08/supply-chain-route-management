
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <winsock2.h>

#define PORT 8080
#define REQUEST_SIZE 16384
#define DATA_SIZE 12000

void send_all(SOCKET client, const char *data, int length) {
    int sent = 0;

    while (sent < length) {
        int n = send(client, data + sent, length - sent, 0);
        if (n <= 0) return;
        sent += n;
    }
}

void send_response(SOCKET client, const char *status,
                   const char *type, const char *body) {
    char header[512];
    int length = (int)strlen(body);

    snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n\r\n",
        status, type, length);

    send_all(client, header, (int)strlen(header));
    send_all(client, body, length);
}

void decode_url(char *text) {
    char *read = text;
    char *write = text;

    while (*read) {
        if (*read == '+') {
            *write++ = ' ';
            read++;
        } else if (*read == '%' && read[1] && read[2]) {
            char hex[3] = {read[1], read[2], '\0'};
            *write++ = (char)strtol(hex, NULL, 16);
            read += 3;
        } else {
            *write++ = *read++;
        }
    }

    *write = '\0';
}

int get_param(const char *body, const char *key,
              char *output, size_t output_size) {
    char copy[4096];
    output[0] = '\0';

    if (strlen(body) >= sizeof(copy)) return 0;

    strcpy(copy, body);

    char *part = strtok(copy, "&");

    while (part != NULL) {
        char *equals = strchr(part, '=');

        if (equals != NULL) {
            *equals = '\0';

            char param_key[256];
            char param_value[2048];

            snprintf(param_key, sizeof(param_key), "%s", part);
            snprintf(param_value, sizeof(param_value), "%s", equals + 1);

            decode_url(param_key);
            decode_url(param_value);

            if (strcmp(param_key, key) == 0) {
                snprintf(output, output_size, "%s", param_value);
                return 1;
            }
        }

        part = strtok(NULL, "&");
    }

    return 0;
}

void append_text(char *output, size_t capacity,
                 size_t *position, const char *format, ...) {
    if (*position >= capacity) return;

    va_list args;
    va_start(args, format);

    int written = vsnprintf(
        output + *position,
        capacity - *position,
        format,
        args
    );

    va_end(args);

    if (written > 0) {
        if ((size_t)written >= capacity - *position)
            *position = capacity;
        else
            *position += (size_t)written;
    }
}

void append_json_string(char *output, size_t capacity,
                        size_t *position, const char *text) {
    append_text(output, capacity, position, "\"");

    while (*text && *position < capacity) {
        unsigned char c = (unsigned char)*text++;

        if (c == '"' || c == '\\') {
            append_text(output, capacity, position, "\\%c", c);
        } else if (c == '\n') {
            append_text(output, capacity, position, "\\n");
        } else if (c == '\r') {
            append_text(output, capacity, position, "\\r");
        } else if (c == '\t') {
            append_text(output, capacity, position, "\\t");
        } else if (c >= 32) {
            append_text(output, capacity, position, "%c", c);
        }
    }

    append_text(output, capacity, position, "\"");
}

void remove_newline(char *line) {
    line[strcspn(line, "\r\n")] = '\0';
}

void save_location(const char *name) {
    FILE *file = fopen("locations.txt", "a");

    if (file == NULL) return;

    fprintf(file, "%s\n", name);
    fclose(file);
}

void save_route(const char *from, const char *to, double distance) {
    FILE *file = fopen("routes.txt", "a");

    if (file == NULL) return;

    fprintf(file, "%s|%s|%.2f\n", from, to, distance);
    fclose(file);
}

void save_delivery(const char *name, const char *destination) {
    FILE *file = fopen("deliveries.txt", "a");

    if (file == NULL) return;

    fprintf(file, "%s|%s\n", name, destination);
    fclose(file);
}

void send_all_data(SOCKET client) {
    char json[DATA_SIZE];
    size_t pos = 0;

    append_text(json, sizeof(json), &pos, "{\"locations\":[");

    FILE *file = fopen("locations.txt", "r");
    char line[1024];
    int first = 1;

    if (file != NULL) {
        while (fgets(line, sizeof(line), file)) {
            remove_newline(line);
            if (line[0] == '\0') continue;

            if (!first)
                append_text(json, sizeof(json), &pos, ",");

            append_json_string(json, sizeof(json), &pos, line);
            first = 0;
        }

        fclose(file);
    }

    append_text(json, sizeof(json), &pos, "],\"routes\":[");

    file = fopen("routes.txt", "r");
    first = 1;

    if (file != NULL) {
        while (fgets(line, sizeof(line), file)) {
            remove_newline(line);

            char *a = strchr(line, '|');
            if (a == NULL) continue;
            *a = '\0';

            char *b = strchr(a + 1, '|');
            if (b == NULL) continue;
            *b = '\0';

            char *end;
            double distance = strtod(b + 1, &end);

            if (end == b + 1 || distance <= 0) continue;

            if (!first)
                append_text(json, sizeof(json), &pos, ",");

            append_text(json, sizeof(json), &pos, "{\"from\":");
            append_json_string(json, sizeof(json), &pos, line);

            append_text(json, sizeof(json), &pos, ",\"to\":");
            append_json_string(json, sizeof(json), &pos, a + 1);

            append_text(json, sizeof(json), &pos,
                        ",\"distance\":%.2f}", distance);

            first = 0;
        }

        fclose(file);
    }

    append_text(json, sizeof(json), &pos, "],\"deliveries\":[");

    file = fopen("deliveries.txt", "r");
    first = 1;

    if (file != NULL) {
        while (fgets(line, sizeof(line), file)) {
            remove_newline(line);

            char *separator = strchr(line, '|');
            if (separator == NULL) continue;

            *separator = '\0';

            if (!first)
                append_text(json, sizeof(json), &pos, ",");

            append_text(json, sizeof(json), &pos, "{\"name\":");
            append_json_string(json, sizeof(json), &pos, line);

            append_text(json, sizeof(json), &pos, ",\"destination\":");
            append_json_string(json, sizeof(json), &pos, separator + 1);

            append_text(json, sizeof(json), &pos, "}");

            first = 0;
        }

        fclose(file);
    }

    append_text(json, sizeof(json), &pos, "],\"completedDeliveries\":[");

    file = fopen("completed_deliveries.txt", "r");
    first = 1;

    if (file != NULL) {
        while (fgets(line, sizeof(line), file)) {
            remove_newline(line);

            char *separator = strchr(line, '|');
            if (separator == NULL) continue;

            *separator = '\0';

            if (!first)
                append_text(json, sizeof(json), &pos, ",");

            append_text(json, sizeof(json), &pos, "{\"name\":");
            append_json_string(json, sizeof(json), &pos, line);

            append_text(json, sizeof(json), &pos, ",\"destination\":");
            append_json_string(json, sizeof(json), &pos, separator + 1);

            append_text(json, sizeof(json), &pos, "}");

            first = 0;
        }

        fclose(file);
    }

    append_text(json, sizeof(json), &pos, "]}");

    send_response(client, "200 OK", "application/json", json);
}

void process_next_delivery(SOCKET client) {
    FILE *file = fopen("deliveries.txt", "r");
    char first_line[2048];

    if (file == NULL ||
        fgets(first_line, sizeof(first_line), file) == NULL) {
        if (file != NULL) fclose(file);

        send_response(client, "200 OK", "application/json",
            "{\"processed\":false,\"message\":\"No pending deliveries\"}");
        return;
    }

    remove_newline(first_line);

    char *separator = strchr(first_line, '|');

    if (separator == NULL) {
        fclose(file);
        send_response(client, "500 Internal Server Error",
            "application/json",
            "{\"message\":\"Invalid delivery data\"}");
        return;
    }

    *separator = '\0';

    char delivery_name[1024];
    char destination[1024];

    snprintf(delivery_name, sizeof(delivery_name), "%s", first_line);
    snprintf(destination, sizeof(destination), "%s", separator + 1);

    FILE *temp = fopen("deliveries.tmp", "w");

    if (temp == NULL) {
        fclose(file);
        send_response(client, "500 Internal Server Error",
            "application/json",
            "{\"message\":\"Could not update deliveries\"}");
        return;
    }

    char line[2048];

    while (fgets(line, sizeof(line), file)) {
        fputs(line, temp);
    }

    fclose(file);
    fclose(temp);

    FILE *completed = fopen("completed_deliveries.txt", "a");

    if (completed == NULL) {
        remove("deliveries.tmp");
        send_response(client, "500 Internal Server Error",
            "application/json",
            "{\"message\":\"Could not save completed delivery\"}");
        return;
    }

    fprintf(completed, "%s|%s\n", delivery_name, destination);
    fclose(completed);

    if (remove("deliveries.txt") != 0 &&
        fopen("deliveries.txt", "r") != NULL) {
        remove("deliveries.tmp");
        send_response(client, "500 Internal Server Error",
            "application/json",
            "{\"message\":\"Could not update pending deliveries\"}");
        return;
    }

    if (rename("deliveries.tmp", "deliveries.txt") != 0) {
        send_response(client, "500 Internal Server Error",
            "application/json",
            "{\"message\":\"Could not update pending deliveries\"}");
        return;
    }

    char json[4096];
    size_t pos = 0;

    append_text(json, sizeof(json), &pos,
                "{\"processed\":true,\"name\":");

    append_json_string(json, sizeof(json), &pos, delivery_name);

    append_text(json, sizeof(json), &pos, ",\"destination\":");

    append_json_string(json, sizeof(json), &pos, destination);

    append_text(json, sizeof(json), &pos, "}");

    send_response(client, "200 OK", "application/json", json);
}

void handle_request(SOCKET client, char *request) {
    char method[16] = "";
    char path[512] = "";

    sscanf(request, "%15s %511s", method, path);

    char *query = strchr(path, '?');
    if (query != NULL) *query = '\0';

    char *header_end = strstr(request, "\r\n\r\n");
    char *body = header_end ? header_end + 4 : "";

    if (strcmp(method, "GET") == 0 &&
        strcmp(path, "/api/data") == 0) {
        send_all_data(client);
        return;
    }

    if (strcmp(method, "POST") == 0 &&
        strcmp(path, "/api/locations") == 0) {
        char name[1024];

        if (!get_param(body, "name", name, sizeof(name)) ||
            name[0] == '\0') {
            send_response(client, "400 Bad Request",
                "application/json",
                "{\"message\":\"Location name is required\"}");
            return;
        }

        save_location(name);
        send_response(client, "200 OK", "application/json",
                      "{\"message\":\"Location saved\"}");
        return;
    }

    if (strcmp(method, "POST") == 0 &&
        strcmp(path, "/api/routes") == 0) {
        char from[1024], to[1024], distance_text[100];

        if (!get_param(body, "from", from, sizeof(from)) ||
            !get_param(body, "to", to, sizeof(to)) ||
            !get_param(body, "distance", distance_text,
                       sizeof(distance_text))) {
            send_response(client, "400 Bad Request",
                "application/json",
                "{\"message\":\"Route details are required\"}");
            return;
        }

        char *end;
        double distance = strtod(distance_text, &end);

        if (from[0] == '\0' || to[0] == '\0' ||
            strcmp(from, to) == 0 ||
            end == distance_text || *end != '\0' ||
            distance <= 0) {
            send_response(client, "400 Bad Request",
                "application/json",
                "{\"message\":\"Invalid route details\"}");
            return;
        }

        save_route(from, to, distance);

        send_response(client, "200 OK", "application/json",
                      "{\"message\":\"Route saved\"}");
        return;
    }

    if (strcmp(method, "POST") == 0 &&
        strcmp(path, "/api/deliveries") == 0) {
        char name[1024], destination[1024];

        if (!get_param(body, "name", name, sizeof(name)) ||
            !get_param(body, "destination", destination,
                       sizeof(destination)) ||
            name[0] == '\0' || destination[0] == '\0') {
            send_response(client, "400 Bad Request",
                "application/json",
                "{\"message\":\"Delivery details are required\"}");
            return;
        }

        save_delivery(name, destination);

        send_response(client, "200 OK", "application/json",
                      "{\"message\":\"Delivery saved\"}");
        return;
    }

    if (strcmp(method, "POST") == 0 &&
        strcmp(path, "/api/deliveries/process") == 0) {
        process_next_delivery(client);
        return;
    }

    if (strcmp(method, "GET") != 0) {
        send_response(client, "404 Not Found",
                      "text/plain", "Not Found");
        return;
    }

    const char *filename = NULL;
    const char *content_type = "text/plain";

    if (strcmp(path, "/") == 0 ||
        strcmp(path, "/index.html") == 0) {
        filename = "index.html";
        content_type = "text/html";
    } else if (strcmp(path, "/style.css") == 0) {
        filename = "style.css";
        content_type = "text/css";
    } else if (strcmp(path, "/app.js") == 0) {
        filename = "app.js";
        content_type = "application/javascript";
    } else {
        send_response(client, "404 Not Found",
                      "text/plain", "404 - Not Found");
        return;
    }

    FILE *static_file = fopen(filename, "rb");

    if (static_file == NULL) {
        send_response(client, "404 Not Found",
                      "text/plain", "File not found");
        return;
    }

    fseek(static_file, 0, SEEK_END);
    long file_size = ftell(static_file);
    rewind(static_file);

    if (file_size < 0 || file_size > 1000000) {
        fclose(static_file);
        send_response(client, "500 Internal Server Error",
                      "text/plain", "Invalid file size");
        return;
    }

    char *content = malloc((size_t)file_size + 1);

    if (content == NULL) {
        fclose(static_file);
        send_response(client, "500 Internal Server Error",
                      "text/plain", "Out of memory");
        return;
    }

    size_t read_size = fread(content, 1, (size_t)file_size, static_file);
    fclose(static_file);
    content[read_size] = '\0';

    char header[512];

    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %u\r\n"
        "Connection: close\r\n\r\n",
        content_type, (unsigned int)read_size);

    send_all(client, header, (int)strlen(header));
    send_all(client, content, (int)read_size);

    free(content);
}

int main(void) {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock initialization failed.\n");
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == INVALID_SOCKET) {
        printf("Could not create socket.\n");
        WSACleanup();
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_address,
             sizeof(server_address)) == SOCKET_ERROR) {
        printf("Could not bind port %d. Is another server running?\n", PORT);
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, 10) == SOCKET_ERROR) {
        printf("Could not listen for requests.\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Supply Chain server running at http://127.0.0.1:%d/\n", PORT);

    while (1) {
        client_socket = accept(server_socket, NULL, NULL);

        if (client_socket == INVALID_SOCKET) continue;

        char request[REQUEST_SIZE];
        int received = 0;
        int expected_length = -1;
        int header_length = -1;

        request[0] = '\0';

        while (received < REQUEST_SIZE - 1) {
            int n = recv(client_socket, request + received,
                         REQUEST_SIZE - 1 - received, 0);

            if (n <= 0) break;

            received += n;
            request[received] = '\0';

            char *end_headers = strstr(request, "\r\n\r\n");

            if (end_headers != NULL) {
                header_length = (int)(end_headers + 4 - request);

                char *length_header = strstr(request, "Content-Length:");

                if (length_header != NULL) {
                    expected_length = atoi(length_header + 15);
                } else {
                    expected_length = 0;
                }

                if (received >= header_length + expected_length) break;
            }
        }

        if (received > 0) {
            handle_request(client_socket, request);
        }

        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();

    return 0;
}
