import sensor, image, time, math, tf
from utime import sleep_ms
import pyb

black_threshold = (30, 95)  # 黑色阈值

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time=2000)
sensor.set_windowing((240, 240))  # 设为正方形
sensor.set_contrast(8)  # 对比度
sensor.set_auto_gain(False)
sensor.set_auto_whitebal(False)  # 一定要关白平衡
clock = time.clock()

SANY = 20
SANYH = 200
SANX = 20
SANXW = 200
ROI = (SANX, SANY, SANXW, SANYH)

class ARROWJUDGE:


    def judge_arrow():
        img = sensor.snapshot().lens_corr(strength=1.8, zoom=1.0)
        countleft = 0
        countright = 0
        arrow = img.find_blobs([black_threshold], roi=ROI, x_stride=3, y_stride=3, area_threshold=8000, merge=False)
        for blob in arrow:
            rcx = blob.cx()
            rcy = (blob.cy())
            roix = blob[0]
            roiy = (blob[1])
            roiw = blob[2]
            roih = (blob[3])
        if (arrow):
            for blob in arrow:
                SANY = roiy  # 更新窗口参数
                SANYH = (roiy + roih)
                SANX = roix
                SANXW = (roix + roiw)+40
                img.draw_rectangle(blob.rect())  # 画矩形
                img.draw_cross(blob.cx(), blob.cy())  # 画十字
            for i in range(SANY, SANYH):
                for j in range(SANX, int((SANX + SANXW) / 2 - 1)):
                    gray = img.get_pixel(j, i)
                    if gray < 95:  # 黑色，计数
                        countleft = countleft + 1
                for k in range(int((SANX + SANXW) / 2), SANXW):
                    gray = img.get_pixel(k, i)

                    if gray < 95:
                        countright = countright + 1
            if SANY < SANX:
                result = "uparrow"
                # print("uparrow")
                return 1
                pass
            elif countleft - countright > 100:
                result = "leftarrow"
                # print("leftarrow")  # img.draw_string(roix-5,roiy-5,"leftarrow",scale=3,x_spacing=-7)
                return 2
            elif countleft - countright < -100:
                result = "rightarrow"
                # print("rightarrow")
                return 3
            # print(roix, roiy, roiw, roih, countleft, countright)
