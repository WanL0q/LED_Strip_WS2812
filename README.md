# LED_Strip_WS2812
Control WS2812 LED strip with STM32F103
![Screenshot 2024-03-22 084601](https://github.com/WanL0q/LED_Strip_WS2812/assets/134664967/3e64fd06-0365-4d2e-80c9-7bd812c33679)
Led matrix Command Frame
ID package
Cycle(ms)
Receive -timeout(ms)
Data length


0x01
100ms
500ms
0x08


Location
Function
Data type 
Description
byte[0]
Led mode
bit[0,1]
0: normal
1: left
2: right
3: error


bit[2,3]
0: off 
1:  auto 
2: manual
3: custom (bỏ qua )
bit[4,5]
           Reserve
bit[6,7]
Reserve
byte[1]
Effect
unsigned int 
0: Solid
1: Pulse
2: Rainbow
3: ColorCycle
4: Blink
5: Comet
byte[2]
Reserve
_
0x00
byte[3]
Reserve
_
0x00
byte[4]
Reserve
_
0x00
byte[5]
Reserve
_
0x00
byte[6]
Reserve
_
0x00
byte[7]
Reserve
_
0x00


ID package
Cycle(ms)
Receive -timeout(ms)
Data length


0x02
-
-
0x08


Location
Function
Data type 
Description
byte[0]
 Led_front_left
unsigned int8
[0,255]
byte[1]
Led_front_right
Unsigned int8
[0,255]
byte[2]
Led_right
unsigned int8
[0,255]
byte[3]
Led_back_right
unsigned int8
[0,255]
byte[4]
Led_back
unsigned int8
[0,255]
byte[5]
Led_back_left
unsigned int8
[0,255]
byte[6]
Led_left
unsigned int8
[0,255]
byte[7]
Led_flag
unsigned int8
[0,255]

