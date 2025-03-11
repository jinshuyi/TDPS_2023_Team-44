import sensor, image, time, math, pyd
from pyb import LED
import car
import GYcar
from pid import PID
import readDistance
pout = pyb.Pin("P6", pyb.Pin.OUT_PP)
pin = pyb.Pin("P9", pyb.Pin.IN)


rho_pid = PID(p=0.6, i=0)
theta_pid = PID(p=0.001, i=0)
thresh = [(125, 255)]
basic_speed = 15
width = 320
height = 240


def count_white_pixels(b_array):
    his = list()
    for i in range(39):
        real_column = (i + 1) * 8 - 1
        his.append(0)
        index = real_column
        his[i] += b_array[index]

        for j in range(239):
            index = index + 320
            his[i] += b_array[index]
    return his
def find_middle_line(his):
    sorted_list, sorted_index = sort(his)
    for i in range(25):
        his[sorted_index[i]] = 0
    half_sum = int(sum(his) / 2)
    temp = 0
    for i in range(39):
        temp += his[i]
        if temp >= half_sum:
            if i < 20 and i != 0:
                return (i-1)
            else:
                return i
def sort(alist):
    length = len(alist)
    line_list = list(range(length))
    for i in range(length - 1):
        for j in range(length - i - 1):
            if alist[j] > alist[j + 1]:
                alist[j], alist[j + 1] = alist[j + 1], alist[j]
                line_list[j], line_list[j + 1] = line_list[j + 1], line_list[j]
    return alist, line_list



sensor.reset() # Initialize the camera sensor.
sensor.set_pixformat(sensor.GRAYSCALE) # or sensor.RGB565
sensor.set_framesize(sensor.QVGA) # or sensor.QVGA (or others)
sensor.skip_frames(time = 2000) # Let new settings take affect.
sensor.set_gainceiling(8)
#uart = UART(3, 19200, timeout_char=100000)
sensor.set_hmirror(True)
sensor.set_vflip(True)

flag = 1
stage = 1
while(flag == 1):
    dis = readDistance()
    print('Distance: %0.2f cm')
    if stage == 0: #test stage
        stdev_list = []
        index_list = []
        img = sensor.snapshot()
        img.find_edges(image.EDGE_CANNY, threshold=(80, 120))
        for i in range(20):
            region = img.statistics(roi=(i * 16, 210, 16, 30))
            std.append(region.stdev())
        max_value = max(stdev_list)
        min_value = min(stdev_list)
        THRESHOLD = 1 / 2 * (max_value - min_value)
        for i in range(20):
            if std[i] >= (max_value - THRESHOLD):
                index_list.append(i)
        output = math.floor(index_list[math.floor(len(index_list) / 2)])
        middle_line = (output + 1) * 16
        img.draw_line((middle_line, 0, middle_line, img.height() - 1), color=255, thickness=2)
        rho_err = abs(middle_line) - img.width() / 2
        rho_output = rho_pid.get_pid(rho_err, 1)
        car.run(15 + rho_output, 15 - rho_output)
    if stage == 1: # first stage
        img = sensor.snapshot()
        img_matrix = img.find_edges(image.EDGE_CANNY, threshold=(70,70))
        his = count_white_pixels(img_matrix)
        middle_line = (find_middle_line(his)+1) * 8
        img.draw_line((middle_line, 0, middle_line, img.height()-1), color = 255, thickness = 2)
        rho_err = abs(middle_line)-img.width()/2
        print("rho_error:", rho_err)
        rho_output = rho_pid.get_pid(rho_err,1)
        print("rho_output:", rho_output)
        car.run(15+rho_output, 15-rho_output)
        if (pin.value() == 1):
            stage += 1
    if stage == 2:
        print("stage 2")
        car.pulse()
        utime.sleep_ms(2000)
        Gycar.turn(-90,1)
        car.pulse()
        utime.sleep_ms(2000)
        Gycar.runb()
        Gycar.turn(90, 1)
        utime.sleep_ms(400)
        stage += 1
    if stage == 3:
        print("stage 3")
        img = sensor.snapshot()
        img_matrix = img.find_edges(image.EDGE_CANNY, threshold=(70, 70))
        his = get_sparse_his(img_matrix)
        middle_line = (get_sparse_middle_line(his) + 1) * 8
        img.draw_line((middle_line, 0, middle_line, img.height() - 1), color=255, thickness=2)
        rho_err = abs(middle_line) - img.width() / 2
        print("rho_error:", rho_err)
        rho_output = rho_pid.get_pid(rho_err, 1)
        print("rho_output:", rho_output)
        car.run(15 + rho_output, 15 - rho_output)
    if stage == 4:
        print("stage 4")
        car.pulse()