import sys
import serial
import serial.tools.list_ports
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
debug_test = False
debug_out_comms = False
debug_in_comms = False
debug_parsing = False
debug_resolution = False
debug_manual_comms = False
with_gui = True

print(f'debug_test : {debug_test}')
print(f'debug_parsing : {debug_parsing}')
print(f'debug_out_comms : {debug_out_comms}')
print(f'debug_in_comms : {debug_in_comms}')
print(f'debug_resolution : {debug_resolution}')
print(f'debug_manual_comms : {debug_manual_comms}')
print(f'with_gui : {with_gui}')
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
# def int_from_unicode(x):
#     return x - ord('0')
baudrate = 9600
timeout = 1
encoding = 'utf-8'
end_marker = '\n'
arduino_ready = False
serial_ports = []

def arduino_comports():
    return [port for port in serial.tools.list_ports.comports() if 'arduino' in port.description.lower()]

def arduino_serial_ports(): #initialize serial_ports
    comports = arduino_comports()
    serial_ports = []
    for port in comports:
        ser = serial.Serial()
        ser.baudrate = baudrate
        ser.port = port.name
        ser.timeout = timeout
        serial_ports.append(ser)
    return serial_ports

def open_serial_at(comport):
    for port in serial_ports:
        if (port.port == comport):
            port.open()
            wait_for_arduino() # useful to figure out if amk or any other arduino
            return port
    print(f'comport {comport} not found in serial_ports')
    return None

def current_serial_port():
    print(f'serial_ports: {serial_ports}')
    for i, port in enumerate(serial_ports):
        if port.is_open:
            return port
    print(f'Error: current serial port not found')
    return None

def close_serial_at(serial_port):
    global arduino_ready
    reply = write_readlines('ac0\n')
    arduino_ready = False
    print(f'Closing port {serial_port.port}')
    serial_port.close()

def switch_comport(comport):
    global arduino_amk
    active_port = current_serial_port()
    if active_port != None:
        close_serial_at(active_port)
    arduino = open_serial_at(comport)
    parse_message(get_keypad(), arduino_amk)



def chuncks(lst, n):
    """Yield successive n-sized chunks from lst."""
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

# def readlines():
#     data = arduino.readlines()
#     return data

def write_readlines(message):
    global arduino
    message_type = type(message)

    if message_type == str:
        message = bytes(message, encoding)
    elif message_type == bytes:
        message = message

    # arduino, index = current_serial_port()
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
    if tk.messagebox.askokcancel("Quit", "Do you want to quit? Changes will not be saved"):
        master.destroy()
        reply = write_readlines('ac0\n')
        arduino_ready = False
        arduino.close()
        print('Closing AMKAPP')

serial_ports = arduino_serial_ports()
arduino = serial_ports[0] # serial.Serial(port='COM4', baudrate=9600, timeout=1) # also opens the serial port
open_serial_at('COM4')
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
                        amk['layout'] = parse_layout(content)
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
# serial_ports = arduino_serial_ports()
amk_list = [amk.AMK(arduino_callback=save_to_arduino)]*len(serial_ports)
# arduino = serial_ports[0]   #= serial.Serial(port='COM4', baudrate=9600, timeout=1)
arduino_amk = amk.AMK(arduino_callback=save_to_arduino) # amk_list[0]
# arduino.open()


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
        app_window = amk_gui.MainFrame(root, WINDOW_WIDTH, WINDOW_HEIGHT, comports=arduino_comports(), comport_callback = switch_comport)#lambda port:switch_comport(port))
        wait_for_arduino()
        if(arduino_ready):
            # amk_1 = amk.AMK(arduino_callback=save_to_arduino)
            parse_message(get_keypad(), arduino_amk)
            app_window.refresh_with(arduino_amk)
    if(debug_manual_comms):
        while True:
            command = input("Enter a command (i.e. wait_for_arduino, get_keypad etc.): ") #+ '\n' # Taking input from user
            reply = globals()[command]()
            parse_message(reply)
    
    root.protocol("WM_DELETE_WINDOW", lambda arg=root: closeApp(arg))
    root.mainloop()

if __name__ == '__main__':
    if debug_test:
        if(with_gui):
            root = tk.Tk()
            RATIO = 1
            RATIO *= root.winfo_screenheight()/1080
            ASCPECT_RATIO = root.winfo_screenwidth()/root.winfo_screenheight()

            HEIGHT_SCALE_FACTOR = 2/3
            WIDTH_SCALE_FACTOR = HEIGHT_SCALE_FACTOR * 4/5

            WINDOW_WIDTH = int( root.winfo_screenheight() * RATIO * WIDTH_SCALE_FACTOR )
            WINDOW_HEIGHT = int( root.winfo_screenheight() * RATIO * HEIGHT_SCALE_FACTOR )
        
            print(arduino_comports())
            port_decriptions = [port.description for port in arduino_comports()]
            app_window = amk_gui.MainFrame(root, WINDOW_WIDTH, WINDOW_HEIGHT, comports=port_decriptions)
            wait_for_arduino()
            if(arduino_ready):
                amk_1 = amk.AMK(arduino_callback=save_to_arduino)
                parse_message(get_keypad(), amk_1)
                app_window.refresh_with(amk_1)
            root.mainloop()
    else:
        open_window()