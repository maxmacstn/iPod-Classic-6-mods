import serial
import time

serial_port = "/dev/cu.usbserial-0001"


MODE_SW = 0x00
MODE_VOICE_REC = 0x01
MODE_SIMPLE_REMOTE = 0x02
MODE_REQ_MODE_STA = 0x03
MODE_AIR = 0x04

def read_serial():
    # if(ser.in_waiting >0):
    read = ser.read_all()
    try:
        print(read.decode("utf-8"), end="")
        
    except UnicodeDecodeError:
        print(read.hex())

    # ser.flush()

def construct_request(mode, cmd, param=bytearray(), large = False):
    mode = bytes([mode])  # Ensure mode is a byte
    header = b"\xFF\x55"

    if large:
        header += b"\x00"
    
    # # Ensure cmd and param are bytes
    # if isinstance(cmd, str):
    #     cmd = cmd.encode()  # Convert string to bytes if necessary
    # if isinstance(param, str):
    #     param = param.encode()

    length = len(mode) + len(cmd) + len(param)

    data = mode + cmd + param
    

    checksum = (0x100 - ((sum(data) + length) & 0xFF)) & 0xFF # Compute checksum correctly


        # checksum = (0x100 - ((sum(data) + length) & 0xFF)) & 0xFF # Compute checksum correctly



    packet = header + bytes([length]) + data + bytes([checksum])

    if large:
        length = length.to_bytes(2,'big')
        packet = header + length + data + bytes([checksum])

    print(f"Packet: {packet.hex().upper()}")  # Print hex representation

    return packet  # Return packet for further use

# 0xFF 0x55 (standard header)
# 0x6E (length of data in this block)
# 0x04 (mode of the command)
# 0x00 0x32 (command for picture display)

# 0x00 0x00 (indicates the first block (block zero))
# 0x01 ??
# 0x00 0x78 (width of picture to be displayed)
# 0x00 0x40 (height of picture to be displayed)
# 0x00 0x00 0x00 0x20 (number of bytes you're sending for each line of the display - must be a multiple of 8 and has to fit the needed bits!)
# The rest is a 4 color picture, with every two bits representing a pixel.
# Checksum.

def send_img():

    img_1st_block = b'\x00\x00'     #block_no
    img_1st_block += b'\x01'        #pixel_format_code
    img_1st_block += b'\x00\x78'    #width
    img_1st_block += b'\x00\x40'    #height
    img_1st_block += b'\x00\x00\x00\x20'    #no. of sending in each line.
    img_1st_block += b'\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00'
    #FF 55 00 18 04 00 32 00 00 01 00 78 00 40 00 00 00 20 00 00 00 00 00 0000000000D9
    ser.write(construct_request(MODE_AIR, b"\x00\x32", img_1st_block,large=True)) 
    time.sleep(1)
    read_serial()

    for i in range (1,70):
        img_n_block = i.to_bytes(2,'big')
        img_n_block += b'\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00'
        ser.write(construct_request(MODE_AIR, b"\x00\x32", img_n_block,large=True))  
        time.sleep(1)
        read_serial()



ser = serial.Serial(port=serial_port, baudrate= 19200, parity="N", stopbits=1, timeout=1)

ser.write(construct_request(MODE_SW, b"\x01\x02"))  #Basic Mode
read_serial()
# ser.write(construct_request(MODE_SIMPLE_REMOTE, b"\x00\x00", b"\x02"))  #Just Pause
# time.sleep(0.1)
# ser.write(construct_request(MODE_SIMPLE_REMOTE, b"\x00\x00")) #Release

time.sleep(3)
ser.write(construct_request(MODE_SW, b"\x01\x04"))  #Switch to AiR Mode
read_serial()
# ser.write(construct_request(MODE_AIR, b"\x00\x14"))  #Request iPod's name.
# time.sleep(0.1)

# ser.write(construct_request(MODE_AIR, b"\x00\x29", b"\x01"))  #AiR Play/Pause

send_img()
time.sleep(0.1)



while(1):
    # read_serial()
    pass