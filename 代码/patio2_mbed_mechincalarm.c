#include "DigitalIn.h"
#include "DigitalOut.h"
#include "PinNames.h"
#include "PinNamesTypes.h"
#include "mbed_thread.h"
#include <cstdio>
#include <mbed.h>
#include "VL53L1X.h"

#define I2C_SCL A6
#define I2C_SDA D12
#define send_pin A3
#define recive_pin A4
#define sg_pin D9

DigitalOut comm_out(send_pin, PullDown);
DigitalIn comm_in(recive_pin,PullDown);

I2C i2c(I2C_SDA,I2C_SCL);
Timer timer;
VL53L1X sensor(&i2c, &timer);
I2C i2c_ds3231(D0, D1);//(I2C_SDA, I2C_SCL)
PwmOut sg90 (sg_pin);
const int DS3231_ADDR = 0xd0;

DigitalInOut xshout(D13,PIN_OUTPUT,OpenDrainNoPull,0);
void sg_reset (void){ //舵机复位，夹紧球
    sg90.period_ms(20);
    sg90.pulsewidth_us(500);
}

void sg_release (void){ //松开
    sg90.period_ms(20);
    sg90.pulsewidth_us(1500);
}

void trans_team_info() {
    int ret;
    int sec, min, hour, day, date, month, year;
    char data_write[2];
    char data_read[8];

    data_write[0] = 0;
    i2c_ds3231.write(DS3231_ADDR, data_write, 1, 0);
    ret = i2c_ds3231.read(DS3231_ADDR, data_read, 7, 0);
    sec = (data_read[0] >> 4) * 10 + (data_read[0] & 0x0f);
    min = (data_read[1] >> 4) * 10 + (data_read[1] & 0x0f);
    hour = (data_read[2] >> 5 & 0x01) * 20 + (data_read[2] >> 4 & 0x01) * 10 + (data_read[2] & 0x0f);
        
    day = data_read[3] & 0x07;
        
    date = (data_read[4] >> 4) * 10 + (data_read[4] & 0x0f);
    month = (data_read[5] >> 4 & 0x01) * 10 + (data_read[5] & 0x0f);
    year = 2000 + (data_read[6] >> 4) * 10 + (data_read[6] & 0x0f);

    // pc.printf("Team Number:44\r\nTeam Member Names:\r\nEnze Li\r\nZijun Yuan\r\nYinuo Gao\r\nShuyi Jin\r\nZiyi Xiao\r\nYuchen Zhou\r\nZixun Zhaao\r\nJiaqi Zhang\r\nXiuyi Xu\r\nRuitong Liu\r\n Time:\r\n");
    // pc.printf(" %d-%02d-%02d %d %02d:%02d:%02d\n\r",  year, month, date, day, hour, min, sec);

    printf("Team Number:44\r\nTeam Member Names:\r\nEnze Li\r\nZijun Yuan\r\nYinuo Gao\r\nShuyi Jin\r\nZiyi Xiao\r\nYuchen Zhou\r\nZixun Zhaao\r\nJiaqi Zhang\r\nXiuyi Xu\r\nRuitong Liu\r\n Time:\r\n");
    printf(" %d-%02d-%02d %d %02d:%02d:%02d\n\r",  year, month, date, day, hour, min, sec);

}

int main(){
    comm_out = 0;
    xshout = 1;
    thread_sleep_for(1000);
  
    sensor.setTimeout(500);
    printf("Program started!\r\n");
    if (!sensor.init())
    {
        printf("Failed to detect and initialize sensor!");
        while (1){
             thread_sleep_for(1000);
        }
    }
    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(50000);
    sensor.startContinuous(50);
    
    float front_dist = 0;
    int cnt = 0;
    int flag = 1;
    printf("Patio 2 Start\r\n");
    sg_reset();
    while(comm_in==false){
        front_dist = sensor.read();
        printf("front distance:%u\r\n", front_dist);
        if(front_dist < 300){
            cnt++;
        }
        else{
            cnt = 0;
        }
        if(cnt == 3){
            comm_out = 1;
            thread_sleep_for(1000);
            printf("Turn\r\n");
            comm_out = 0;
            cnt = 0;
        }
    }
    
    sg_release();
    thread_sleep_for(5000);
    while(true){
        if(comm_in==true){
            break;
        }

    }

    trans_team_info();

    }

