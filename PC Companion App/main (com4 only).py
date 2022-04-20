import sys
import serial
import time
import tkinter as tk
from tkinter import ttk, messagebox
import amk
import amk_gui
# import ComArduino

"""
protocol:
    recieve:
        op code := <char> (1 Byte)
        header := <char> (1 Bytes)
        length := integer (1 Byte)
        data := any (1*length Bytes)
    send:
        op code := <char> (1 Byte)
        header := <char> (1 Bytes)
        length := integer (1 Byte)
        data := any (1*length Bytes)
"""


"""insert handshake function here"""
#~ hi! tell me your communication protocol, does it match mine? else error

#region debug flags
debug_parsing = False
debug_set = False
debug_out_comms = True
debug_in_comms = True
debug_test = False
debug_layout = False
debug_resolution = False
debug_manual_comms = False
with_gui = True
#endregion

print(f'debug_parsing : {debug_parsing}')
print(f'debug_set : {debug_set}')
print(f'debug_out_comms : {debug_out_comms}')
print(f'debug_in_comms : {debug_in_comms}')
print(f'debug_test : {debug_test}')
print(f'debug_layout : {debug_layout}')
print(f'debug_resolution : {debug_resolution}')
print(f'debug_manual_comms : {debug_manual_comms}')

#region serial port
arduino = serial.Serial(port='COM4', baudrate=9600, timeout=1) # also opens the serial port
encoding = 'utf-8'
end_marker = '\n'
arduino_ready = False
###device detection/selection
#endregion

#region get OS
if sys.platform == "win32":
    import ctypes
    try:    #is os windows >= 8.1
        ctypes.windll.shcore.SetProcessDpiAwareness(2) # if your windows version >= 8.1
    except: #is os windows < 8.1 
        ctypes.windll.user32.SetProcessDPIAware() # win 8.0 or less 
    RATIO = 1
elif sys.platform == "darwin":
    RATIO = 1.375
#endregion

if(with_gui):
    #region general gui functions
    def grid_hide(widget):
        widget._grid_info = widget.grid_info()
        widget.grid_remove()

    def grid_show(widget):
        widget.grid(**widget._grid_info)

    #endregion

#region serial communication
def int_from_unicode(x):
    return x - ord('0')
    
def chuncks(lst, n):
    """Yield successive n-sized chunks from lst."""
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

def readlines():
    data = arduino.readlines()
    return data

def write_readlines(message):
    message_type = type(message)

    if message_type == str:
        message = bytes(message, encoding)
    elif message_type == bytes:
        message = message

    arduino.write(message)
    time.sleep(0.05)
    data = arduino.readlines()

    if(debug_out_comms):
        print(f'request: {message}')
    if(debug_in_comms):
        print(f'reply: {data}')

    return data

def wait_for_arduino(num_tries=3):
    global arduino_ready
    
    while((not arduino_ready) and (num_tries > 0)):
        print(f'querying arduino. tries left:{num_tries}')
        reply = write_readlines('ao0'+end_marker)
        reply = reply[0].decode(encoding) if len(reply)>0 else ''
        if ('ready' in reply):
            arduino_ready = True
            print('AMK ready!')
        else:
            num_tries -= 1
            time.sleep(0.5)
    
    if(num_tries == 0 and not arduino_ready):
        print('Error: No answer from AMK')

def closeApp(master):
    if tk.messagebox.askokcancel("Quit", "Do you want to quit?"):
        master.destroy()
        reply = write_readlines('ac0\n')
        arduino_ready = False
        arduino.close()
        print('Closing AMKAPP')
        

def parse_message(message, amk):
    if(message):
        for line in message:
            if(len(line)>2):
                op_code = chr(line[0])
                header = chr(line[1])
                length = line[2]
                content = line[3:-1] if (length > 0) else '' #exclude the end of line character

                if(debug_parsing):
                    print(f"op code:  {op_code}, type: {type(op_code)}")
                    print(f"header:  {header}, type: {type(header)}")
                    print(f"length:  {length}, type: {type(length)}")
                    print(f"content:  {content}, type: {type(content)}")

                if(op_code == 's'):
                    if(header == 'c'): # set configuration
                        amk["layout"] = parse_layout(content)
                    elif(header == 'b'): # set bindings
                        amk["bindings"] = parse_binding(amk, content)
                    elif(header == 'o'): # set orientation
                        amk["orientation"] = parse_orientation(content)
                    elif(header == 'a'): # set active set
                        amk["active_set"] = parse_active_set(content)
                    elif(header == 'n'): # set active set
                        amk["name"] = parse_string(content)
                    else:
                        print(f'Parse_Error: recieved unknown header: {header}')
                elif(op_code == 'p'):
                    print(f'Arduino says: {str(content)}')
                elif(op_code == 'e'):
                    print(f'Arduino_Error: {str(content)} for {header}')

                if(debug_parsing):
                    print(amk)
            else:
                print(f'unknown message {line}')
    else:
        print("Parse_Error: empty reply!")
#endregion

#region parse functions
def parse_active_set(content):
    value = list(content)[0]-ord('0')
    if(debug_parsing):
        print("active set is: " + str(value))

    return int(value)

def parse_orientation(content):
    value = list(content)[0]-ord('0')
    if(debug_parsing):
        print("orientation is: " + str(value))

    return value

def parse_layout(content):
    values = [int(i)-ord('0') for i in list(content)]
    if(debug_parsing):
        print("layout is: " + str(values))
    return values
    
def parse_binding(origin, content):
    bindings = list(content) #[int(i)-ord('0') for i in list(content)]
    number_of_keys = origin["num_rows"]*origin["num_cols"]
    bindings = list(chuncks(bindings, number_of_keys))
    if(debug_parsing):
        print("binding sets are: " + str(bindings))
    
    return bindings

def list_to_bytearray(lst):
    temp_list = []
    for sublist in lst:
        temp_list.extend(sublist)
    byte_str = bytes(temp_list)
    # print(byte_str)
    return byte_str

def parse_string(content):
    global encoding
    value = content.decode(encoding)
    if(debug_parsing):
        print("active set is: " + value)

    return value
    
# def int_to_bytes(i: int, *, signed: bool = False) -> bytes:
#     length = ((i + ((i * signed) < 0)).bit_length() + 7 + signed) // 8
#     return i.to_bytes(length, byteorder='big', signed=signed)


#endregion

#region get functions
# these are wrappers/api for sending serial commands
def get_keypad():
    return write_readlines('gc\ngo\nga\ngb\ngn\n')

def get_layout():
    return write_readlines('gc\n')

def get_orientation():
    return write_readlines('go\n')

def get_active_set():
    return write_readlines('ga\n')

def get_bindings():
    return write_readlines('gb\n')

def get_name():
    return write_readlines('gn\n')
#endregion

#region set functions
def send_bindings(bindings, amk):
    # bindings = [[20, 20, 20, 20, 20, 20, 20, 20, 20], [20, 20, 20, 20, 20, 20, 20, 20, 20]]
    op_code = b's'
    header = b'b'
    content = list_to_bytearray(bindings)
    length = b''#bytes([len(content)])
    message = b'' + op_code + header + length + content + bytes('\n', 'utf-8')
    # print(message)
    parse_message(write_readlines(message), amk)

def save_to_arduino(amk_data: dict, amk):
    for name, data in amk_data.items():
        if(name == 'bindings'):
            send_bindings(data, amk)

#endregion

#region init
startMarker = 254
endMarker = 255
specialByte = 253


KEYBOARD_X = 1
KEYBOARD_Y = 4

KEYBOARD_WIDTH = 10
KEYBOARD_HEIGHT = 12

SET_SELECTOR_X = 12
SET_SELECTOR_Y = 6
SET_SELECTOR_WIDTH = 4
# SET_SELECTOR_VALUES = [i for i in range(keypad['num_sets'])]
#endregion
def open_window():
    root = tk.Tk()
    global RATIO
    RATIO *= root.winfo_screenheight()/1080
    ASCPECT_RATIO = root.winfo_screenwidth()/root.winfo_screenheight()

    HEIGHT_SCALE_FACTOR = 2/3
    WIDTH_SCALE_FACTOR = HEIGHT_SCALE_FACTOR * 4/5

    WINDOW_WIDTH = int( root.winfo_screenheight() * RATIO * WIDTH_SCALE_FACTOR )
    WINDOW_HEIGHT = int( root.winfo_screenheight() * RATIO * HEIGHT_SCALE_FACTOR )


    if(with_gui):
        wait_for_arduino()
        if(arduino_ready):
            amk_1 = amk.AMK(arduino_callback=save_to_arduino)
            parse_message(get_keypad(), amk_1)
            app_window = amk_gui.MainFrame(root, WINDOW_WIDTH, WINDOW_HEIGHT, amk_1)
    if(debug_manual_comms):
        while True:
            command = input("Enter a command (i.e. wait_for_arduino, get_keypad etc.): ") #+ '\n' # Taking input from user
            reply = globals()[command]()
            parse_message(reply)
    
    root.protocol("WM_DELETE_WINDOW", lambda arg=root: closeApp(arg))
    root.mainloop()

if __name__ == '__main__':
    if debug_test:
        wait_for_arduino()
        if(arduino_ready):
            root = tk.Tk()
            amk_1 = amk.AMK(arduino_callback=save_to_arduino)
            parse_message(get_keypad(), amk_1)
            amk_1['bindings'] = [[20, 20, 20, 20, 20, 20, 20, 20, 20], [20, 20, 20, 20, 20, 20, 20, 20, 20]]
            amk_1.save_to_arduino()
            closeApp(root)
    else:
        open_window()