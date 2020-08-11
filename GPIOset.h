#ifndef __GPIOset
#define __GPIOset

#include <stdlib.h>
#include <stdio.h> 
#include <arm-linux-gnueabihf/sys/unistd.h>
#include <arm-linux-gnueabihf/sys/fcntl.h>      
#include <arm-linux-gnueabihf/sys/mman.h>

#define GPIO_BASE 0x7E215000    // pi4, GPFSEL0, GPIO Function Select 0, set Pin mode
#define GPFSEL0   0x00      // GPIO Fucntin Select
#define GPSET0    0x1C      // GPIO Pin Output Set 0
#define GPCLR0    0x28      // GPIO Pin Output Clear 0
#define GPLEV0    0x34      // GPIO Pin Level

#define GPIO_IN 0
#define GPIO_OUT 1

// Access GPIO Register
unsigned int* get_base_addr(); // Get GPIO register Pointer
void gpio_sel(unsigned int* addr, int port, int mode); // Set Pin Mode
void gpio_set(unsigned int* addr, int port, int mode); // Port set
void gpio_lev(unsigned int* addr, int port, int mode); // Port Level
void gpio_clear(unsigned int* addr, int port); // Port clear

// User Define Function
void LED_on(int port);
void LED_off(int port);
int Blink(int port, int time);
void Blink_off(int port, int cpid);

unsigned int* get_base_addr() {
    int fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("Can not open /dev/mem\n");
        exit(-1);
    }

#define PAGE_SIZE (4096)
    void* mmaped = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE);
    if (mmaped < 0) {
        printf("mmap failed\n");
        exit(-1);
    }
    close(fd);

    return (unsigned int*)mmaped;
}

void gpio_sel(unsigned int* addr, int port, int mode) {
    unsigned int* a = addr + (port / 10);
    unsigned int mask = ~(0x7 << ((port % 10) * 3));
    *a &= mask;
    *a |= (mode & 0x7) << ((port % 10) * 3);
}

void gpio_set(unsigned int* addr, int port, int mode) {
    if (port < 0 || port > 31) {
        printf("Port is out of range : %d\n");
        exit(-1);
    }
    if (mode == 1) {
        *(addr + 7) = 0x1 << port;
    }
    else {
        *(addr + 7) = 0x0 << port;
    }
}

void gpio_lev(unsigned int* addr, int port, int mode) {
    if (port < 0 || port > 31) {
        printf("Port is out of range : %d\n");
        exit(-1);
    }
    if (mode == 1) {
        *(addr + 13) = 0x1 << port;
    }
    else {
        *(addr + 13) = 0x0 << port;
    }
}

void gpio_clear(unsigned int* addr, int port) {
    if (port < 0 || port > 31) {
        printf("Port is out of range : %d\n");
        exit(-1);
    }
    *(addr + 10) = 0x1 << port;
}

void LED_on(int port) {
    volatile unsigned int* addr = get_base_addr();
    gpio_sel(addr, port, GPIO_OUT);
    gpio_set(addr, port, GPIO_OUT);
    gpio_lev(addr, port, GPIO_OUT);
}

void LED_off(int port) {
    volatile unsigned int* addr = get_base_addr();
    gpio_clear(addr, port);
}

int Blink(int port, int time) {
    volatile unsigned int* addr = get_base_addr();
    gpio_sel(addr, port, GPIO_OUT);
    pid_t pid;
    pid = fork();

    if (pid == 0) {
        while (1) {
            gpio_set(addr, port, GPIO_OUT);
            gpio_lev(addr, port, GPIO_OUT);
            usleep(10000 * time);
            gpio_clear(addr, port);
            usleep(10000 * time);
        }
    }
    else if (pid == -1) {
        printf("fork Failed\n");
        exit(-1);
    }
    else {
        return (int)pid;
    }
}

void Blink_off(int port, int cpid) {
    volatile unsigned int* addr = get_base_addr();
    gpio_clear(addr, port);
    kill((pid_t)cpid, 1);
}


#endif

