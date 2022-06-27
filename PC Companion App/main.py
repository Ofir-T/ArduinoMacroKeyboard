"""
Arduino Macro Keyboard Companion App - AMKAPP

"""

#region Imports
import sys
import serial
import serial.tools.list_ports
import time
import tkinter as tk
from tkinter import ttk, messagebox
import logging
import amk
import amk_gui
# import ComArduino
#endregion

"""Start logging"""
logging.basicConfig(style='{', format='{asctime}:{levelname}:{message}', filename='logging.log', encoding='utf-8', level=logging.DEBUG)
logging.info('Starting AMK companion app')

"""Log exceptions instead of print to console"""
def exception_hook(exc_type, exc_value, exc_traceback):
    logging.error(
        "Uncaught exception",
        exc_info=(exc_type, exc_value, exc_traceback)
    )

sys.excepthook = exception_hook # Uncomment for executables

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

#region debug flags
# Set to true every parameter you want to recieve more information on
# debug_test is for  experimenting :)
debug_test = False
debug_out_comms = False
debug_in_comms = False
debug_parsing = False
debug_resolution = False
manual_comms = False
with_gui = True

logging.debug(f'Debug parameters:')
logging.debug(f'debug_test : {debug_test}')
logging.debug(f'debug_parsing : {debug_parsing}')
logging.debug(f'debug_out_comms : {debug_out_comms}')
logging.debug(f'debug_in_comms : {debug_in_comms}')
logging.debug(f'debug_resolution : {debug_resolution}')
logging.debug(f'manual_comms : {manual_comms}')
logging.debug(f'with_gui : {with_gui}')
#endregion

if(with_gui): # maybe move this to amk_gui?
    def grid_hide(widget):
        widget._grid_info = widget.grid_info()
        widget.grid_remove()

    def grid_show(widget):
        widget.grid(**widget._grid_info)

#region serial communication

# def int_from_unicode(x):
#     return x - ord('0')

baudrate = 9600
timeout = 1
encoding = 'utf-8'
end_marker = '\n'
arduino_ready = False
# serial_ports = [] #get rid of this

def arduino_comports():
    """Lists active COM ports on the computer with 'arduino' in their name"""
    logging.info(f'Scanning for Arduino COM devices...')
    comports = [port for port in serial.tools.list_ports.comports() if 'arduino' in port.description.lower()]
    logging.info(f'Arduino COM devices found: {comports}') #make a clearer string to print
    return comports

def list_serial_ports():
    """Returns a list of available, initalized, closed serial ports"""
    logging.info(f'Scanning Arduino devices for serial ports...')
    comports = arduino_comports()
    serial_ports = []
    for port in comports:
        ser = serial.Serial()
        ser.baudrate = baudrate
        ser.port = port.name
        ser.timeout = timeout
        serial_ports.append(ser)
    logging.info(f'Serial ports found: {serial_ports}')
    return serial_ports

def open_serial_at(comport_name):
    logging.info(f'Opening serial port for device: {comport_name}')
    for port in list_serial_ports():
        if (port.port == comport_name):
            port.open()
            logging.info(f'Success')
            # wait_for_arduino() # useful to figure out if amk or any other arduino
            return port
    logging.warning(f'Failed. Comport {comport_name} not found in serial_ports')
    return None

def current_serial_port():
    """Finds the current active device by looking for an open serial port"""
    logging.info(f'Looking for the active serial port:')
    for i, port in enumerate(list_serial_ports()):
        if port.is_open:
            logging.info(f'found port: {port}')
            return port
    logging.warning(f'Didn\'t find any open serial port')
    return None

def close_serial_at(serial_port):
    """Closes communication with AMK device at serial_port"""
    global arduino_ready
    reply = write_readlines('ac0\n') # tell current amk to stop communication with app
    arduino_ready = False
    logging.info(f'Closing port {serial_port.port}')
    serial_port.close()

def switch_device_to(comport_name):
    """Switches active device to device at COM port comport_name"""
    active_port = current_serial_port()
    if active_port != None:
        close_serial_at(active_port)
    # arduino = open_serial_at(comport_name)
    #get amk data. here or outside the function?
    global current_amk 
    current_amk = amk.AMK(arduino_callback=save_to_arduino, serial_port=open_serial_at(comport_name))
    wait_for_arduino() # useful to figure out if amk or any other arduino
    parse_message(get_keypad(), current_amk)
    # -------------------
    # if app_window != None:
    #   app_window.refresh_with(current_amk)
    #  
    #  ---and/or---
    # 
    logging.info(f'successfully switched to device: {current_amk.name}')
    return current_amk


def chuncks(lst, n):
    """Yield successive n-sized chunks from lst."""
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

# def readlines():
#     data = current_amk.serial_port.readlines()
#     return data

def write_readlines(message):
    """Writes a message on serial, expecting an anwser.
    Reads multiple lines, if available"""
    global current_amk
    message_type = type(message)

    if message_type == str:
        message = bytes(message, encoding)
    elif message_type == bytes:
        message = message

    current_amk.serial_port.write(message)
    time.sleep(0.05)
    data = current_amk.serial_port.readlines()

    if(debug_out_comms):
        print(f'request: {message}')
    if(debug_in_comms):
        print(f'reply: {data}')

    return data

def wait_for_arduino(num_tries=3, delay=0.5):
    """Tells the AMK device to get ready for communication"""
    global arduino_ready
    
    while((not arduino_ready) and (num_tries > 0)):
        logging.info(f'querying arduino. tries left:{num_tries}')
        reply = write_readlines('ao0'+end_marker)
        reply = reply[0].decode(encoding) if len(reply)>0 else ''
        if ('ready' in reply):
            arduino_ready = True
            logging.info(f'Arduino: AMK device ready!')
        else:
            num_tries -= 1
            time.sleep(0.5)
    
    if(num_tries == 0 and not arduino_ready):
        logging.Error(f'No answer from AMK device')

def close_window(app_window):
    if tk.messagebox.askokcancel("Quit", "Do you want to quit? Unsaved changes will be discarded"):
        app_window.destroy()
        reply = write_readlines('ac0\n')
        arduino_ready = False
        current_amk.serial_port.close()
        logging.info(f'Closing AMKAPP')

#endregion

#region message parsing functions
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

def parse_message(message, input_amk):
    if(message):
        error = False
        for line in message:
            if(len(line)>2):
                op_code = chr(line[0])
                header = chr(line[1])
                length = line[2]
                content = line[3:-1] if (length > 0) else '' # exclude the end of line character

                if(debug_parsing):
                    print(f"op code:  {op_code}, type: {type(op_code)}")
                    print(f"header:  {header}, type: {type(header)}")
                    print(f"length:  {length}, type: {type(length)}")
                    print(f"content:  {content}, type: {type(content)}")

                if(op_code == 's'):
                    if(header == 'c'): # keyboard configuration: shape, number of sets
                        input_amk['layout'] = parse_layout(content)
                    elif(header == 'b'): # set bindings
                        input_amk["bindings"] = parse_binding(input_amk, content)
                    elif(header == 'o'): # set orientation
                        input_amk["orientation"] = parse_orientation(content)
                    elif(header == 'a'): # set active set
                        input_amk["active_set"] = parse_active_set(content)
                    elif(header == 'n'): # set active set
                        input_amk["name"] = parse_string(content)
                    else:
                        error = True
                        logging.error(f'Received unknown header from AMK: {header}')
                elif(op_code == 'p'):
                    print(f'Arduino says: {str(content)}')
                elif(op_code == 'e'):
                    error = True
                    logging.error(f'Received error from AMK: header: {header}, content: {str(content)}')

                if(debug_parsing):
                    print(input_amk)
            else:
                error = True
                logging.error(f'Received unknown message from AMK: {line}')
    else:
        error = True
        logging.error(f'Received empty reply from AMK!')

    if(error):
        print(f'An error has occured while parsing. See logging.log for more information')
        

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
    logging.info(f'AMK config saved') # maybe check if saving was correct?

def save_to_arduino(amk_data: dict, amk):
    logging.info(f'Saving AMK config to arduino')
    for name, data in amk_data.items():
        if(name == 'bindings'):
            send_bindings(data, amk)

#endregion

#region init
#This whole region should happen after gui is up, or as the last step of initialization
# serial_ports = list_serial_ports()
# amk_list = [amk.AMK(arduino_callback=save_to_arduino, serial_port=serial_ports[i]) for i, device in enumerate(serial_ports)]
# logging.info(f'AMK list: {amk_list}')
# selected_amk = amk_list[0] # selected_amk, number is decided in-app
# selected_amk.serial_port.open() # selected_amk
# arduino = selected_amk.serial_port
#endregion

def ask_for_userinput():
    """Manual communication with AMK. gets wrapper function names"""
    # user_input = raw_input("Give me your command! Just type \"exit\" to close: ")
    user_input = input("Enter a command (i.e. wait_for_arduino, get_keypad etc.): ") #+ '\n' # Taking input from user
    if user_input == "exit":
        root.quit()
    else:
        reply = globals()[user_input]()
        parse_message(reply, current_amk)
        root.after(0, ask_for_userinput)

def open_window():
    """Opens the app GUI window"""
    
    # Check OS for window aspect ratio
    if sys.platform == "win32":
        import ctypes
        try:    #is os windows >= 8.1
            ctypes.windll.shcore.SetProcessDpiAwareness(2) # if your windows version >= 8.1
        except: #is os windows < 8.1 
            ctypes.windll.user32.SetProcessDPIAware() # win 8.0 or less 
        RATIO = 1
    elif sys.platform == "darwin": # macOS
        RATIO = 1.375

    # Open the Tk window
    root = tk.Tk()
    
    RATIO *= root.winfo_screenheight()/1080 # Our window's size depends on it's vertical resolution
    ASCPECT_RATIO = root.winfo_screenwidth()/root.winfo_screenheight()

    HEIGHT_SCALE_FACTOR = 2/3
    WIDTH_SCALE_FACTOR = HEIGHT_SCALE_FACTOR * 4/5

    WINDOW_WIDTH = int( root.winfo_screenheight() * RATIO * WIDTH_SCALE_FACTOR )
    WINDOW_HEIGHT = int( root.winfo_screenheight() * RATIO * HEIGHT_SCALE_FACTOR )

    if(with_gui):
        app_window = amk_gui.MainFrame(root, WINDOW_WIDTH, WINDOW_HEIGHT, devices=arduino_comports(), switch_device_callback = lambda comport_name: switch_device_to(comport_name))
        # app_window.device_selector.refresh_with(arduino_comports())
    if(manual_comms):
        root.after(0, ask_for_userinput)
    
    root.protocol("WM_DELETE_WINDOW", lambda arg=root: close_window(arg)) #
    root.mainloop() # pass control to tkinter's handlers. blocks the rest of the script

if __name__ == '__main__':

    global current_amk 
    current_amk = amk.AMK() # The active AMK object, with which we will interact

    if debug_test:
        debug_open_window()
    else:
        open_window()