import sensor, image, time, math
from utime import sleep_ms
import car
from pid import PID
from hcsr04 import HCSR04

sensor_right = HCSR04(trigger_pin='P9', echo_pin='P10')
sensor_forward = HCSR04(trigger_pin='P11', echo_pin='P12')

rho_pid = PID(p=0.4, i=0)
theta_pid = PID(p=0.001, i=0)
distance_pid = PID(p=0.4, i=0)

kernel_size = 1
kernel = [-1, -1, -1,\
          -1, +8, -1,\
          -1, -1, -1]

ROI = [40, 0, 240, 240]

BINARY_VISIBLE = True
i = 0

sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # Set pixel format to GRAYSCALE
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.skip_frames(time = 2000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.

def ultrasonic_control(distance, limt):

    distance_err = limt - sensor_right.distance_cm()
    output = distance_pid.get_pid(distance_err, 1)
    car.run(50 + output, 50 - output)



while(True):
    clock.tick()                    # Update the FPS clock.

    right_distance = sensor_right.distance_cm()

    if(right_distance > 20 or i == 1):

        img = sensor.snapshot()         # Take a picture and return the image.

        histogram = img.get_histogram()
        Thresholds = histogram.get_threshold()
        l = Thresholds.l_value()
        a = Thresholds.a_value()
        b = Thresholds.b_value()
        v = Thresholds.value()
        print("Thresholds", v)

        img.morph(kernel_size, kernel)
        img.mean(1)
        img.binary([(v, 255)])
        img.erode(2, threshold = 2)
        img.dilate(2, threshod = 2)
        img.dilate(2)

        line = img.get_regression([(255,255) if BINARY_VISIBLE else THRESHOLD], roi = ROI)

        if (line): #找到线

            img.draw_line(line.line(), color = 127)
            center_pos =  (line.x1() + line.x2()) / 2 #找到直线中心点
            rho_err = center_pos - img.width()/2

            if line.theta()>90:

                theta_err = line.theta()-180

            else:

                theta_err = line.theta()

            print("rho_err theta_err", rho_err, theta_err)

            rho_output = rho_pid.get_pid(rho_err,1)
            theta_output = theta_pid.get_pid(theta_err,1)
            output = rho_output+theta_output
            print("outputs", output)

            car.run(50+output, 50-output)

        else:

            car.run(50, -50)

        if(right_distance < 20 and i == 1): #通过gate

            sleep_ms(3000)
            car.run(0, 0)
            break

    else:

        while(True): #发现障碍物，并通过

            if(right_distance < 20):

                right_distance = sensor_right.distance_cm()
                ultrasonic_control(right_distance, 10) #转到超声波控制

            else: #通过障碍物

                sleep_ms(500)
                break
        #特判
        car.run(0, 0)
        car.run(50, -50)
        sleep_ms(500)
        car.run(0, 0)
        sleep_ms(500)
        car.run(50, 50)
        sleep_ms(500)

        while(True): #上桥下桥

            right_distance = sensor_right.distance_cm()
            forward_distance = sensor_forward.distance_cm()

            if(right_distance < 20): #前半部分超声波控制

                ultrasonic_control(right_distance, 10)

            else: #无障碍物，保持直行

                car.run(50, 50)

                if(forward_distance < 30): #前方发现障碍物，结束直行

                    car.run(-50, 50) #旋转找线
                    i = 1#记录操作
                    break

    print("FPS", clock.fps())
