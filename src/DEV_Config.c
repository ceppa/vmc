/*****************************************************************************
* | File      	:   DEV_Config.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*----------------
* |	This version:   V2.0
* | Date        :   2019-07-08
* | Info        :   Basic version
*
******************************************************************************/
#include "DEV_Config.h"
#include <unistd.h>
#include <fcntl.h>

uint32_t fd,UART_fd;
char *UART_Device;

int TXDEN_1;
int TXDEN_2;
int IRQ_PIN;
/******************************************************************************
function:	Equipment Testing
parameter:
Info:   Only supports Jetson Nano and Raspberry Pi
******************************************************************************/
static int DEV_Equipment_Testing(void)
{
	int i;
	int fd;
	char value_str[20];
	fd = open("/etc/issue", O_RDONLY);
//    printf("Current environment: ");
	while(1) {
		if (fd < 0) {
			return -1;
		}
		for(i=0;; i++) {
			if (read(fd, &value_str[i], 1) < 0) {
				return -1;
			}
			if(value_str[i] ==32) {
				printf("\r\n");
				break;
			}
//			printf("%c",value_str[i]);
		}
		break;
	}

	if(i<5) {
		printf("Unrecognizable\r\n");
        return -1;
	} else {
		char RPI_System[10]   = {"Raspbian"};
		for(i=0; i<6; i++) {
			if(RPI_System[i] != value_str[i])
                    return 'J';
		}
        return 'R';
	}
	return -1;
}


/******************************************************************************
function:	GPIO Function initialization and transfer
parameter:
Info:
******************************************************************************/
void DEV_GPIO_Mode(uint16_t Pin, uint16_t Mode)
{
    /*
        0:  INPT
        1:  OUTP
    */
    SYSFS_GPIO_Export(Pin);
    if(Mode == 0 || Mode == SYSFS_GPIO_IN){
        SYSFS_GPIO_Direction(Pin, SYSFS_GPIO_IN);
    }else{
        SYSFS_GPIO_Direction(Pin, SYSFS_GPIO_OUT);
    }
}

void DEV_Digital_Write(uint16_t Pin, uint8_t Value)
{
    SYSFS_GPIO_Write(Pin, Value);
}

uint8_t DEV_Digital_Read(uint16_t Pin)
{
    uint8_t Read_value = 0;
    Read_value = SYSFS_GPIO_Read(Pin);
    return Read_value;
}


/**
 * delay x ms
**/
void DEV_Delay_ms(UDOUBLE xms)
{
    UDOUBLE i;
    for(i=0; i < xms; i++){
        usleep(1000);
    }
}


void GPIO_Config(void)
{
    int Equipment = DEV_Equipment_Testing();
    if(Equipment=='R'){
        /************************
        Raspberry Pi GPIO
        ***********************/
        TXDEN_1 = 27;
        TXDEN_2 = 22;
        IRQ_PIN = 25;
    }else if(Equipment=='J'){
        /************************
        Jetson Nano GPIO
        ***********************/
        TXDEN_1 = GPIO27;
        TXDEN_2 = GPIO22;
        IRQ_PIN = GPIO25;
    }else{
        printf("Device read failed or unrecognized!!!\r\n");
        while(1);
    }

    DEV_GPIO_Mode(TXDEN_1, 1);
    DEV_GPIO_Mode(TXDEN_2, 1);
    DEV_GPIO_Mode(IRQ_PIN, 0);
}

/******************************************************************************
function:	SPI Function initialization and transfer
parameter:
Info:
******************************************************************************/
void DEV_UART_Init(char *Device)
{
    UART_Device = Device;
    DEV_HARDWARE_UART_begin(UART_Device);

    // UART_Set_Baudrate(115200);
}

void UART_Write_Byte(uint8_t data)
{
  DEV_HARDWARE_UART_writeByte(data);
}

int UART_Read_Byte(void)
{
    return DEV_HARDWARE_UART_readByte();
}

int UART_Read_Bytes(uint8_t *data,uint32_t len)
{
    return DEV_HARDWARE_UART_readBytes(data,len);
}

void UART_Set_Baudrate(uint32_t Baudrate)
{
  DEV_HARDWARE_UART_setBaudrate(Baudrate);
}

int UART_Write_nByte(uint8_t *pData, uint32_t Lan)
{
  DEV_HARDWARE_UART_write(pData, Lan);
  return 0;
}
/******************************************************************************
function:	Module Initialize, the library and initialize the pins, SPI protocol
parameter:
Info:
******************************************************************************/
uint8_t DEV_ModuleInit(void)
{
  printf("USE_DEV_LIB \r\n");
  GPIO_Config();



    return 0;
}

/******************************************************************************
function:	Module exits, closes SPI and BCM2835 library
parameter:
Info:
******************************************************************************/
void DEV_ModuleExit(void)
{
}

