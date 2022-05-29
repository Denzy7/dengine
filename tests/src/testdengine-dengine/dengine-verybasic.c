#include <dengine/dengine.h>
int main(int argc, char *argv[])
{
    dengine_init();
    while(dengine_update()){
        denginegui_text(50, 50, "Hello There! This is a very basic window...", NULL);
    }
    dengine_terminate();
}
