/**
 * main.cpp - continuous example
 */
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "PinNames.h"
#include "PinNamesTypes.h"
#include "mbed_thread.h"
#include <mbed.h>
#include <VL53L1X.h>

#define I2C_SCL1 A6
#define I2C_SDA1 D12
#define send_pin A3
#define recive_pin A4
#define sg_pin D9
DigitalOut comm_out(send_pin);
DigitalIn comm_in(recive_pin,PullDown);

// Serial pc(A7, A2);//(SERIAL_TX, SERIAL_RX
I2C i2c_ds3231(D0, D1);//(I2C_SDA, I2C_SCL)
// DigitalIn enable(D3) ;
PwmOut sg90 (sg_pin);
const int DS3231_ADDR = 0xd0;


// DigitalIn ServoIn (D12, PullDown);
void sg_reset (void){ //舵机复位，夹紧球
    sg90.period_ms(20);
    sg90.pulsewidth_us(500);
}
void sg_release (void){ //松开
    sg90.period_ms(20);
    sg90.pulsewidth_us(1500);
}

void trans_team_info();

I2C i2c(I2C_SDA1,I2C_SCL1);
Timer timer;

VL53L1X sensor(&i2c, &timer);

//Serial pc(USBTX, USBRX, 115200);
DigitalInOut xshout(D13,PIN_OUTPUT,OpenDrainNoPull,0);
int main() {
  xshout = 1;
  //sg_reset();
  
  thread_sleep_for(500);
  sensor.setTimeout(500);
  printf("Program started!\r\n");
  if (!sensor.init())
  {
    printf("Failed to detect and initialize sensor!");
    while (1){
        thread_sleep_for(1000);
    }
  }
  // Use long distance mode and allow up to 50000 us (50 ms) for a measurement.
  // You can change these settings to adjust the performance of the sensor, but
  // the minimum timing budget is 20 ms for short distance mode and 33 ms for
  // medium and long distance modes. See the VL53L1X datasheet for more
  // information on range and timing limits.
  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(50000);

  // Start continuous readings at a rate of one measurement every 50 ms (the
  // inter-measurement period). This period should be at least as long as the
  // timing budget.
  // sensor.read() return mm 
  sensor.startContinuous(50);
/*
  while (1)
  {
    printf("%u", sensor.read());
    if (sensor.timeoutOccurred())
    {
      printf(" TIMEOUT\r\n");
    }

    printf("\r\n");
  }
*/

  // trans_team_info();
  int test_dist = sensor.read();
  printf("test_dist. = %d\n",test_dist);
//sg_release();
//printf("sg_release\n");
    int front_dist = 0;
    // while(1) {
    //     front_dist = sensor.read();
    // printf("%u\n", sensor.read());
    // }
    
    comm_out = 0;
    while(comm_in == 0);
    //thread_sleep_for(500);
    printf("start flag2 detection\n");
    int cnt2=0;
    while(comm_in == 1 && cnt2 < 4) { // flag2 走向栏杆的左拐阈值
        front_dist = sensor.read();
        printf("flag2_distance = %d cnt2 = %d\n", front_dist,cnt2);
        if(front_dist <= 800) {
            cnt2 ++;
            // comm_out = 1;
            // break;
        }
    }
    comm_out = 1;
    thread_sleep_for(300);
    comm_out = 0;
    while(comm_in == 0);
    int cnt4=0;
    while(comm_in == 1 && cnt4 < 10) { // flag4 拐第二个弯的阈值
        front_dist = sensor.read();
        printf("flag4_distance = %d\n", front_dist);
        if(front_dist <= 700) {
            cnt4 ++;
            printf("cnt4 = %d\n",cnt4);
        }
    }
    comm_out = 1;
    thread_sleep_for(500);
    comm_out = 0;
    while(comm_in == 0);

    int cnt6=0;
    while(comm_in == 1 && cnt6 < 8) { // flag6 在卵石路拐最后一个弯的阈值
        front_dist = sensor.read();
        printf("flag6_distance = %d\n", front_dist);
        if(front_dist <= 800) {
            cnt6 ++;
            printf("cnt6 = %d\n",cnt6);
        }
    }
    comm_out = 1;
    thread_sleep_for(500);
    comm_out = 0;


    while(comm_in == 0);
    int cnt8=0;
    while(comm_in == 1 && cnt8 < 6) { // flag7 撞到篮子
        front_dist = sensor.read();
        printf("flag6_distance = %d\n", front_dist);
        printf("%d\n", front_dist);
        if(front_dist <= 700) {
            cnt8 ++;
            printf("cnt6 = %d\n",cnt8);
        }
    }
    comm_out = 1;
    thread_sleep_for(200);
    comm_out = 0;
    while(comm_in == 0);
    while(comm_in == 1) { //机械臂操作
        // do some then break
        sg_release();
        
    }
    thread_sleep_for(200);
    comm_out = 0; //告诉他你已经操作完了，可以走了

    while(comm_in == 0);
    while(comm_in == 1) { //HC12操作
        // do some HC-12 over
        trans_team_info();
    }
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
