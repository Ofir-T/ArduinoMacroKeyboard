"""
Arduino Macro Keyboard Companion App - AMKAPP
Ofir Temelman

version number: 2.x
version date: 23102022
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
import inspect
import com_arduino
import amk
import gui
#endregion

# Start logging
path_to_log = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'logging2.log')
LOGGING_LEVEL = logging.INFO
logging.basicConfig(
    style='{', format='{asctime}:{levelname}:{message}',
    filename=path_to_log, encoding='utf-8', level=LOGGING_LEVEL)

logging.info('Starting AMK companion app')
logging.info(f'Logging level: {LOGGING_LEVEL} (10: DEBUG, 20: INFO)')

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
DEBUG_TEST = False
DEBUG_OUT_COMMS = False
DEBUG_IN_COMMS = False
DEBUG_PARSING = True
DEBUG_RESOLUTION = False
MANUAL_COMMS = False
WITH_GUI = True

logging.debug(f'Debug parameters:')
logging.debug(f'DEBUG_TEST : {DEBUG_TEST}')
logging.debug(f'DEBUG_PARSING : {DEBUG_PARSING}')
logging.debug(f'DEBUG_OUT_COMMS : {DEBUG_OUT_COMMS}')
logging.debug(f'DEBUG_IN_COMMS : {DEBUG_IN_COMMS}')
logging.debug(f'DEBUG_RESOLUTION : {DEBUG_RESOLUTION}')
logging.debug(f'MANUAL_COMMS : {MANUAL_COMMS}')
logging.debug(f'WITH_GUI : {WITH_GUI}')
#endregion

# if WITH_GUI: # maybe move this to gui?
#     def grid_hide(widget):
#         widget._grid_info = widget.grid_info()
#         widget.grid_remove()

#     def grid_show(widget):
#         widget.grid(**widget._grid_info)

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
BAUDRATE = 56700
TIMEOUT = 1
ENCODING = 'utf-8'
end_marker = '\n'
arduino_ready = False

# def int_from_unicode(x): #NTS: Consider deleting
#     return x - ord('0')

def switch_device_to(comport_name): #NTS: it's actually scanning ports twice: on close, and on open.
    """Switches active device to device at COM port comport_name.

    This function closes the active connection, if it exists, updates
    the global AMK object, and updates it with the new AMK device's data
    
    :param comport_name: The name of the port to activate
    """
    global   CURRENT_AMK
    new_port = com_arduino.switch_port_to(comport_name)
    CURRENT_AMK = amk.AMK(arduino_callback=save_to_arduino,
                          serial_port=new_port)
    com_arduino.ser = new_port
    com_arduino.sendToArduino("ao")
    com_arduino.waitForArduino()
    # wait_for_arduino() # Handshake with AMK device
    parse_message(get_keypad(),   CURRENT_AMK) # Send and receive
    logging.info(f'successfully switched device to: {  CURRENT_AMK.name}')
    return   CURRENT_AMK

def chuncks(lst, n):
    """Yield successive n-sized chunks from lst."""
    for i in range(0, len(lst), n):
        yield lst[i:i + n]


def close_window(app_window):
    """Closes the GUI window, and informs the AMK Device.
    
    :param app_window: The window to close.
    :type app_window: gui.MainFrame object.
    """
    logging.info(f'Closing AMKAPP...')
    # parse_message(com_arduino.send_recv2('ac'), CURRENT_AMK)
    com_arduino.send_recv2('ac')
    if tk.messagebox.askokcancel("Quit", 'Do you want to quit? Unsaved changes will be discarded'):
        com_arduino.close_serial_at(  CURRENT_AMK.serial_port)
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
    value = ord(content) #list(content)[0]-ord('0')
    if DEBUG_PARSING:
        print("active set is: " + str(value))

    return value

def parse_orientation(content):
    """Converts a message payload containing orientation to it's value.
    
    :param content: Payload to parse.
    :returns: orientation value.
    """
    value = ord(content) #list(content)[0]-ord('0')
    if DEBUG_PARSING:
        print("orientation is: " + str(value))

    return value

def parse_layout(content):
    """Converts a message payload containing layout to a list of values.
    
    :param content: Payload to parse.
    :returns: list of layout values.
    """

    values = [int(i) for i in list(content)] #[int(i)-ord('0') for i in list(content)]
    if DEBUG_PARSING:
        print("layout is: " + str(values))
    return values
    
def parse_binding(target, content):
    """Converts a 1d list of key bindings into a 2d list with \
        dimensions of target.
    
    :param target: AMK object, whose keypad dimensions we want to use.
    :param content: String payload to parse.
    :returns: Key bindings matrix.
    :rtype: List[List[int]]
    """
    cont = content.split('\x00')[:-1] # cut trailing empty string
    number_of_keys = target["num_rows"]*target["num_cols"]
    bindings = list(chuncks(cont, number_of_keys)) #[cont[i, cont.index(chr(0))] for i in range(len(cont))]
    if DEBUG_PARSING:
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
    global ENCODING
    value = content.decode(ENCODING)
    if DEBUG_PARSING:
        print("Message string is: " + value)
    return value

def parse_message(message, target_amk): #NTS: make target_amk optional, and a condition to saving the data
    """Converts a multi-line message from AMK device according to \
        the protocol, and inserts it to the specified AMK object.

    
    :param message: Message to parse. Can be multi-line.
    """#NTS: incomplete docstring``
    error = False
    if message:
        error = False #NTS: redundant?
        for line in message:
            if len(line)>2: # split the message to it's parts, excludes the end of line character
                print(f'line: {line}')
                length = ord(line[0])#line[0]
                op_code = line[1]#chr(line[1])
                header = line[2]#chr(line[2])
                content = line[3:] if (length > 2) else ''

                if DEBUG_PARSING:
                    print(f"op code:  {op_code}, type: {type(op_code)}")
                    print(f"header:  {header}, type: {type(header)}")
                    print(f"length:  {length}, type: {type(length)}")
                    print(f"content:  {content}, type: {type(content)}")

                if op_code == 's':
                    if header == 'c': # keyboard shape, number of sets
                        target_amk['layout'] = parse_layout(content)
                    elif header == 'b':
                        target_amk["bindings"] = parse_binding(target_amk,
                                                              content)
                    elif header == 'o':
                        target_amk["orientation"] = parse_orientation(content)
                    elif header == 'a':
                        target_amk["active_set"] = parse_active_set(content)
                    elif header == 'n':
                        target_amk["name"] = content
                    else:
                        error = True
                        logging.error(f'Received unknown '
                                      'header from AMK: {header}')
                elif op_code == 'p':
                    print(f'Arduino says: {str(content)}')
                elif op_code == 'e':
                    error = True
                    logging.error(f'Received error from AMK: header: {header},'
                                  ' content: {str(content)}')

                if DEBUG_PARSING:
                    print(target_amk)
            else:
                error = True
                logging.error(f'Received unknown message from AMK: {line}')
    else:
        error = True
        logging.error(f'Received empty reply from AMK!')

    if error:
        print(f'An error has occured while parsing. See logging.log for more information')
#endregion

#region get functions
# these are wrappers/api for sending serial commands
def get_keypad():
    """Asks AMK device for all of it's data over serial.  \
        Expects an answer"""
    # return write_readlines('gc\ngo\nga\ngb\ngn\n')
    com_arduino.message_queue += ['gc', 'go', 'ga', 'gb']
    return com_arduino.send_recv2('gn')

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
    op_code = 's'
    header = 'b'

    flat_list = []
    for sublist in bindings: # "Flatten" the list to 1d
        flat_list.extend(sublist)
    flat_list = [bind+'\x00' for bind in flat_list] # null-terminate bindings
    content = ''.join(flat_list)
    print(content)
    # content = list_to_bytearray(null_term_bindings)
    # length = b''#bytes([len(content)])
    message = op_code + header + content
    print(message)
    parse_message(com_arduino.send_recv2(message), amk) # Send data, echo back #NTS: maybe enable sending without target_amk?
    logging.info(f'AMK config saved') # check if saving was correct?

def save_to_arduino(amk_data: dict, amk):
    logging.info(f'Saving AMK config to arduino')
    for name, data in amk_data.items():
        if name == 'bindings':
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
    """Refreshes the devices list on the gui. Happens every <period> \
        miliseconds"""
    app_window.device_selector.refresh_with(com_arduino.list_comports())
    logging.info(f'Refreshing device list')
    # root.after(period, update_device_list)
    app_window.master.after(period, lambda arg=app_window: update_device_list(app_window=arg))

def ask_for_userinput():
    """Manual communication with AMK.  Legal input is get function names"""
    # user_input = raw_input("Give me your command! Just type \"exit\" to close: ")
    user_input = input('Enter a command (i.e. wait_for_arduino, get_keypad etc.): ') # Take input from user
    if user_input == "exit":
        root.quit()
    else:
        reply = globals()[user_input]()
        parse_message(reply,   CURRENT_AMK)
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

    if WITH_GUI:
        app_window = gui.MainFrame(root, WINDOW_WIDTH, WINDOW_HEIGHT, 
            update_devices_callback=com_arduino.list_comports, 
            switch_device_callback=lambda port: switch_device_to(port)) # = lambda port: global   CURRENT_AMK = amk.AMK(arduino_callback=save_to_arduino, serial_port=open_serial_at(comport_name))
        # app_window.device_selector.refresh_with(list_comports())
    if MANUAL_COMMS:
        root.after(0, ask_for_userinput)
    
    root.protocol("WM_DELETE_WINDOW", lambda arg=root: close_window(arg))
    root.after(1, lambda: logging.debug(f'GUI loaded'))
    root.mainloop() # Pass control to tkinter's handlers. Blocking function

if __name__ == '__main__':

    global   CURRENT_AMK 
    CURRENT_AMK = amk.AMK() # Active AMK object, with which we will interact

    if DEBUG_TEST:
        debug_open_window()
    else:
        # NTS:
        # run_app()
        # if WITH_GUI:
        #   load_gui()
        # else:
        #   ask_for_user_input()
        open_window()