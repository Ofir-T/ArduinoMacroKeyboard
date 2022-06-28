"""
Arduino Macro Keyboard Companion App - AMKAPP
Ofir Temelman
"""

#region Imports
import os
import sys
import serial
import serial.tools.list_ports
import time
import tkinter as tk
from tkinter import ttk, messagebox
import logging
import amk
import amk_gui
#endregion

# Start logging
path_to_log = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'logging.log')
logging_level = logging.INFO
logging.basicConfig(
    style='{', format='{asctime}:{levelname}:{message}',
    filename=path_to_log, encoding='utf-8', level=logging_level)

logging.info('Starting AMK companion app')
logging.info(f'Logging level: {logging_level} (10: DEBUG, 20: INFO)')

def exception_hook(exc_type, exc_value, exc_traceback): #NTS: only logs exceptions in my modules?
    """Log exceptions instead of print to console"""
    logging.error(
        "Uncaught exception",
        exc_info=(exc_type, exc_value, exc_traceback)
    )

sys.excepthook = exception_hook # Uncomment for executables

#region debug flags
# Set to true every parameter for which you want to
# recieve more information.  debug_test is for  experimenting :).
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
"""
protocol:
    message:
        op code := <char> (1 Byte)
        header := <char> (1 Bytes)
        length := integer (1 Byte)
        data := any (1*length Bytes)
        end of line character: '\n'
"""

# Serial parameters
baudrate = 9600 #NTS: Try 115200
timeout = 1
encoding = 'utf-8'
end_marker = '\n'
arduino_ready = False

# def int_from_unicode(x): #NTS: Consider deleting
#     return x - ord('0')

def list_comports(keyword='arduino'): # Set keyword to '' to allow all.
    """Lists active COM ports on the computer with 'arduino' in \
        their name.

    :param keyword: A sub-string to look for in port descriptions.\
    set to '' to allow all strings.
    :returns: List of COM ports
    """
    logging.info(f'Scanning for Arduino COM devices...')
    comports = [port for port in serial.tools.list_ports.comports() 
                if keyword in port.description.lower()]

    logging.info(f'Arduino COM devices found: {comports}') #NTS: make a clearer string to print
    return comports

def list_serial_ports():
    """Initializes available serial ports, and returns them in a list.

    :param comports: A list of available COM ports.
    :returns: a list of serial ports
    :rtype: list of serial.Serial objects
    """
    logging.info(f'Scanning Arduino devices for serial ports...')
    serial_ports = []
    for port in list_comports():
        ser = serial.Serial()
        ser.baudrate = baudrate
        ser.port = port.name
        ser.timeout = timeout
        serial_ports.append(ser)
    logging.info(f'Serial ports found: {serial_ports}')
    return serial_ports

def open_serial_at(comport_name):
    """starts serial communication at the port with the given name.

    :param comport_name: The name of the target port. e.g. 'COM4'
    :returns: The port that has been opened
    :rtype: serial.Serial object
    """
    logging.info(f'Opening serial port for device: {comport_name}')
    for port in list_serial_ports():
        if (port.port == comport_name):
            port.open()
            logging.info(f'Serial port opened successfully')
            # wait_for_arduino() # handshake with AMK #NTS: what's the best location for this?
            return port
    logging.warning(f'Failed. Comport {comport_name} not found in \
                    serial_ports')
    return None

def current_serial_port(): #NTS: maybe check arduino ready? to prevent redundant scanning
    """Finds the current active device by \
    looking for an open serial port.
    
    :returns: The active port, if there is one, or None
    :rtype: serial.Serial object, or None
    """
    logging.info(f'Looking for the active serial port:')
    for i, port in enumerate(list_serial_ports()):
        if port.is_open:
            logging.info(f'found port: {port}')
            return port
    logging.warning(f'Didn\'t find any open serial port')
    return None

def close_serial_at(serial_port):
    """Closes serial communication with AMK device at serial_port.
    
    :param serial_port: The serial port to be closed
    :type serial_port: serial.Serial object
    """
    global arduino_ready
    if(arduino_ready):
        reply = write_readlines('ac0\n') # Tell current amk app is closing
        arduino_ready = False
        serial_port.close()
        logging.info(f'Closing port {serial_port.port}')

def switch_device_to(comport_name): #NTS: it's actually scanning ports twice: on close, and on open.
    """Switches active device to device at COM port comport_name.

    This function closes the active connection, if it exists, updates
    the global AMK object, and updates it with the new AMK device's data
    
    :param comport_name: The name of the port to activate
    """
    close_serial_at(current_serial_port())

    global current_amk 
    current_amk = amk.AMK(arduino_callback=save_to_arduino,
                          serial_port=open_serial_at(comport_name))
    wait_for_arduino() # Handshake with AMK device
    parse_message(get_keypad(), current_amk) # Send and receive
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
    """Writes a message on serial, expecting an anwser.  \
    Reads multiple lines, if available.

    :param message: A message to AMK in the specified protocol.
    :returns: The reply from AMK device
    """
    global current_amk
    message_type = type(message) #NTS: maybe just write type(message)? or match-case?

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

def wait_for_arduino(num_tries=3, delay=0.5): #NTS: maybe change name to handshake/wait_for_handshake?
    """Tells the AMK device to get ready for communication, and waits \
        for a reply
    
    :param num_tries: The number of time to try the handshake.
    :param delay: Time to wait between tries.
    """
    global arduino_ready
    
    while((not arduino_ready) and (num_tries > 0)):
        logging.info(f'Querying arduino. tries left:{num_tries}')
        reply = write_readlines('ao0'+end_marker)
        reply = reply[0].decode(encoding) if len(reply)>0 else ''
        if ('ready' in reply):
            arduino_ready = True
            logging.info(f'Arduino: AMK device ready!')
        else:
            num_tries -= 1
            time.sleep(0.5)
    
    if(num_tries == 0 and not arduino_ready):
        logging.error(f'No answer from AMK device')

def close_window(app_window):
    """Closes the GUI window, and informs the AMK Device.
    
    :param app_window: The window to close.
    :type app_window: amk_gui.MainFrame object.
    """
    logging.info(f'Closing AMKAPP...')
    if tk.messagebox.askokcancel("Quit", "Do you want to quit? \
                                 Unsaved changes will be discarded"):
        close_serial_at(current_amk.serial_port)
        logging.info(f'Closed serial port')
        app_window.destroy()
        logging.info(f'Window destroyed')

#endregion

#region message parsing functions
def parse_active_set(content):
    """Converts a message payload containing active set to it's index.
    
    :param content: Payload to parse.
    :returns: active set's index.
    """
    value = list(content)[0]-ord('0')
    if(debug_parsing):
        print("active set is: " + str(value))

    return int(value)

def parse_orientation(content):
    """Converts a message payload containing orientation to it's value.
    
    :param content: Payload to parse.
    :returns: orientation value.
    """
    value = list(content)[0]-ord('0')
    if(debug_parsing):
        print("orientation is: " + str(value))

    return value

def parse_layout(content):
    """Converts a message payload containing layout to a list of values.
    
    :param content: Payload to parse.
    :returns: list of layout values.
    """
    values = [int(i)-ord('0') for i in list(content)]
    if(debug_parsing):
        print("layout is: " + str(values))
    return values
    
def parse_binding(target, content):
    """Converts a 1d list of key bindings into a 2d list with \
        dimensions of target.
    
    :param target: AMK object, whose keypad dimensions we want to use.
    :param content: Payload to parse.
    :returns: Key bindings matrix.
    :rtype: List[List[int]]
    """
    bindings = list(content)
    number_of_keys = target["num_rows"]*target["num_cols"]
    bindings = list(chuncks(bindings, number_of_keys))
    if(debug_parsing):
        print("binding sets are: " + str(bindings))
    
    return bindings

def list_to_bytearray(lst):
    """Converts a 2d list of key bindings into a bytestring to send over serial.
    
    :param lst: Payload to parse.
    :returns: active set's index.
    """
    temp_list = []
    for sublist in lst: # "Flatten" the list to 1d
        temp_list.extend(sublist)
    byte_str = bytes(temp_list)
    # print(byte_str)
    return byte_str

def parse_string(content):
    """Converts a message payload containing string to a python string.
    
    :param content: Payload to parse.
    :type content: bytestring
    :returns: Decoded str in utf-8.
    """
    global encoding
    value = content.decode(encoding)
    if(debug_parsing):
        print("Message string is: " + value)
    return value

def parse_message(message, target_amk): #NTS: make target_amk optional, and a condition to saving the data
    """Converts a multi-line message from AMK device according to \
        the protocol, and inserts it to the specified AMK object.

    
    :param message: Message to parse. Can be multi-line.
    """#NTS: incomplete docstring
    if(message):
        error = False
        for line in message:
            if(len(line)>2): # split the message to it's parts, excludes the end of line character
                op_code = chr(line[0])
                header = chr(line[1])
                length = line[2]
                content = line[3:-1] if (length > 0) else '' 

                if(debug_parsing):
                    print(f"op code:  {op_code}, type: {type(op_code)}")
                    print(f"header:  {header}, type: {type(header)}")
                    print(f"length:  {length}, type: {type(length)}")
                    print(f"content:  {content}, type: {type(content)}")

                if(op_code == 's'):
                    if(header == 'c'): # keyboard shape, number of sets
                        target_amk['layout'] = parse_layout(content)
                    elif(header == 'b'):
                        target_amk["bindings"] = parse_binding(target_amk,
                                                              content)
                    elif(header == 'o'):
                        target_amk["orientation"] = parse_orientation(content)
                    elif(header == 'a'):
                        target_amk["active_set"] = parse_active_set(content)
                    elif(header == 'n'):
                        target_amk["name"] = parse_string(content)
                    else:
                        error = True
                        logging.error(f'Received unknown header from \
                                      AMK: {header}')
                elif(op_code == 'p'):
                    print(f'Arduino says: {str(content)}')
                elif(op_code == 'e'):
                    error = True
                    logging.error(f'Received error from AMK: header: \
                                  {header}, content: {str(content)}')

                if(debug_parsing):
                    print(target_amk)
            else:
                error = True
                logging.error(f'Received unknown message from AMK: {line}')
    else:
        error = True
        logging.error(f'Received empty reply from AMK!')

    if(error):
        print(f'An error has occured while parsing. See logging.log \
              for more information')
        

# def int_to_bytes(i: int, *, signed: bool = False) -> bytes:
#     length = ((i + ((i * signed) < 0)).bit_length() + 7 + signed) // 8
#     return i.to_bytes(length, byteorder='big', signed=signed)

#endregion

#region get functions
# these are wrappers/api for sending serial commands
def get_keypad():
    """Asks AMK device for all of it's data over serial.  \
        Expects an answer"""
    return write_readlines('gc\ngo\nga\ngb\ngn\n')

def get_layout():
    """Asks AMK device for it's layout data over serial.  \
        Expects an answer"""
    return write_readlines('gc\n')

def get_orientation():
    """Asks AMK device for it's orientation data over serial.  \
        Expects an answer"""
    return write_readlines('go\n')

def get_active_set():
    """Asks AMK device for it's active set data over serial.  \
        Expects an answer"""
    return write_readlines('ga\n')

def get_bindings():
    """Asks AMK device for it's key binding data over serial.  \
        Expects an answer"""
    return write_readlines('gb\n')

def get_name():
    """Asks AMK device for it's name over serial.  \
        Expects an answer"""
    return write_readlines('gn\n')
#endregion

#region set functions
def send_bindings(bindings, amk):
    """Sends the given key bindings to AMK device to update.
    
    :param bindings: Key binding data.
    :param amk: amk.AMK object to recieve reply.
    """
    op_code = b's'
    header = b'b'
    content = list_to_bytearray(bindings)
    length = b''#bytes([len(content)])
    message = b'' + op_code + header + length + content + bytes('\n', 'utf-8')
    # print(message)
    parse_message(write_readlines(message), amk) # Send data, echo back #NTS: maybe enable sending without target_amk?
    logging.info(f'AMK config saved') # check if saving was correct?

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

def update_device_list(app_window, period=3000):
    """Refreshes the devices list on the gui. Happens every <period> seconds"""
    app_window.device_selector.refresh_with(list_comports())
    logging.info(f'Refreshing device list')
    # root.after(period, update_device_list)
    app_window.master.after(period, lambda arg=app_window: update_device_list(app_window=arg))

def ask_for_userinput():
    """Manual communication with AMK.  Legal input is get function names"""
    # user_input = raw_input("Give me your command! Just type \"exit\" to close: ")
    user_input = input("Enter a command (i.e. wait_for_arduino, \
                       get_keypad etc.): ") # Take input from user
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
            ctypes.windll.shcore.SetProcessDpiAwareness(2)
        except: #is os windows < 8.1 
            ctypes.windll.user32.SetProcessDPIAware()
        RATIO = 1
    elif sys.platform == "darwin": # macOS
        RATIO = 1.375

    # Open the Tk window
    root = tk.Tk()
    
    # Window size depends on it's vertical resolution
    RATIO *= root.winfo_screenheight()/1080
    ASCPECT_RATIO = root.winfo_screenwidth()/root.winfo_screenheight()

    HEIGHT_SCALE_FACTOR = 2/3
    WIDTH_SCALE_FACTOR = HEIGHT_SCALE_FACTOR * 4/5

    WINDOW_WIDTH = int( root.winfo_screenheight() * RATIO * WIDTH_SCALE_FACTOR )
    WINDOW_HEIGHT = int( root.winfo_screenheight() * RATIO * HEIGHT_SCALE_FACTOR )

    if(with_gui):
        app_window = amk_gui.MainFrame(root, WINDOW_WIDTH, WINDOW_HEIGHT, 
            update_devices_callback=list_comports, 
            switch_device_callback=lambda port: switch_device_to(port))
        # app_window.device_selector.refresh_with(list_comports())
    if(manual_comms):
        root.after(0, ask_for_userinput)
    
    root.protocol("WM_DELETE_WINDOW", lambda arg=root: close_window(arg))
    root.after(1, lambda: logging.debug(f'GUI loaded'))
    root.mainloop() # Pass control to tkinter's handlers. Blocking function

if __name__ == '__main__':

    global current_amk 
    current_amk = amk.AMK() # Active AMK object, with which we will interact

    if debug_test:
        debug_open_window()
    else:
        # NTS:
        # run_app()
        # if(with_gui):
        #   load_gui()
        # else:
        #   ask_for_user_input()
        open_window()