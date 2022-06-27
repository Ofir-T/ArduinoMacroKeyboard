import sys
import ctypes
import tkinter as tk
from tkinter import ttk
import inspect
import amk
import logging

# KEYBOARD_X = 1
# KEYBOARD_Y = 4

# KEYBOARD_WIDTH = 10
# KEYBOARD_HEIGHT = 12

# SET_SELECTOR_X = 12
# SET_SELECTOR_Y = 6
# SET_SELECTOR_WIDTH = 4
# SET_SELECTOR_VALUES = [i for i in range(keypad['num_sets'])]

class MainFrame:
    def __init__(self, master, width, height, **kwargs): # param reveal_cells=false
        logging.info(f'Loading GUI')
        self.controller = None
        self.master = master
        self.AMK = kwargs.get('AMK', None)
        #self.devices = kwargs.get('devices', None)
        self.grid_rows = 21
        self.grid_cols = 17
        self.grid_labels = [[0]*self.grid_cols]*self.grid_rows

        # set window size
        self.master.geometry(f'{width}x{height}')
        master.title('Macro Keyboard Companion App')
        
        # create main grid for placing the widgets
        self.frame = tk.Frame(master=self.master, borderwidth=0)
        for i in range (self.grid_rows):
            self.frame.rowconfigure(i, weight=1, uniform="window")
            for j in range(self.grid_cols):
                self.frame.columnconfigure(j, weight=2, uniform="window")
                label = tk.Label(master=self.frame, text='', relief=tk.GROOVE, borderwidth=0)#f'{i},{j}', borderwidth=0)
                label.grid(row=i, column=j, sticky='NSEW')
                self.grid_labels[i][j] = label
                # label['text'] = f'{i},{j}'
        self.frame.pack(side='top', fill='both', expand=True)

        self.save_button = tk.Button(self.frame, text ="Save Changes")
        self.save_button.grid(row=9, column=12, columnspan=4, sticky='NSEW')

        self.keyboard = MacroKeyboard(self.frame, self)
        self.set_selector = SetSelector(self.frame, self)
        self.device_selector = DeviceSelector(self.frame, self, devices=kwargs.get('devices', []), callback=kwargs.get('switch_device_callback', None))
        self.subscribers = [self.keyboard, self.set_selector, self.device_selector]

        if(self.AMK != None):
            self.refresh_with(AMK)

    def add_subscriber(self, subscriber):
        if subscriber not in self.subscribers:
            self.subscribers.append(subscriber)
            print(self.subscribers)
        else:
            print(f'{subscriber} already subscribed to {self}')

    def notify(self, event: dict):
        for event, data in event.items():
            if event == 'active_set_changed':
                self.AMK['active_set'] = data
                self.refresh_with(self.AMK)
            if event == 'key_changed':
                set_index, key_index, key_code = data
                self.AMK['bindings'][set_index][key_index] = key_code
                self.refresh_with(self.AMK)
            if event == 'active_device_changed': #in progress
                print(f'Active device changed to: {data.to_string()}')
                self.AMK = data
                self.refresh_with(self.AMK)

    def refresh_with(self, AMK, **kwargs):
        # print(AMK.to_string())
        self.AMK = AMK
        self.keyboard.refresh_with(self.AMK)
        self.set_selector.refresh_with(self.AMK)
        self.save_button.configure(command=self.save_button_callback)
        self.device_selector.refresh_with(kwargs.get('devices', self.device_selector.devices))
        self.master.update()

    def refresh_window(self):
        for sub in self.subscribers:
            sub.update()
        self.master.update()

    def reveal_cells(self):
        # if(debug_layout):
        for i in range (self.grid_rows):
            for j in range(self.grid_cols):
                self.grid_labels[i][j].config(text = f'{i},{j}', borderwidth = 1)# something doesn't work
                print(self.grid_labels[i][j]['text'])
        
        self.refresh_window()

    def hide_cells(self):
        return

    def to_string(self):
        attributes_str = '\n'.join(f'{key}: {value}' for key, value in self.__dict__.items() if not key.startswith('__') and not callable(key))
        self_str = f'this object is a {type(self)} with attributes:\n{attributes_str}'
        return inspect.cleandoc(self_str)
    
    def save_button_callback(self):
        tk.messagebox.showinfo(title=None, message='Changes Saved')
        self.AMK.save_to_arduino()

    # def pub_vars(self):
    #     """Gives the variable names of our instance we want to expose
    #     """
    #     return [k for k in vars(self) if not k.startswith('_')]

class SetSelector:
    def __init__(self, master, controller, selector_values=[], default_value='', pos_x = 12, pos_y = 6, state='readonly'):
        self.controller = controller
        self.master = master

        self.frame = tk.Frame(master=self.master, relief=tk.GROOVE, borderwidth=1)
        self.frame.columnconfigure(0, weight=1, uniform="set")
        self.frame.columnconfigure(1, weight=1, uniform="set")

        self.label = tk.Label(master=self.frame, text='Set:', relief=tk.GROOVE, borderwidth=0)
        self.label.grid(row=0, column=0, sticky='E')

        self.stringvar = tk.StringVar()
        self.combobox = ttk.Combobox(master=self.frame, textvariable=self.stringvar, width=4)
        self.combobox['values'], self.combobox['state'] = selector_values, state
        self.combobox.set(default_value)
        self.combobox.grid(row=0,column=1, columnspan=2, sticky="E")
        self.combobox.bind('<<ComboboxSelected>>', self.notify_set_changed)

        self.frame.grid(row=pos_y, column=pos_x, sticky='NSEW', columnspan=4)

    def set_controller(self, controller: object):
        self.controller = controller
        self.controller.add_subscriber(self)

    def refresh_with(self, AMK):
        selector_values = list(range(AMK['num_sets']))
        default_value = AMK['active_set']
        self.combobox['values'] = selector_values
        self.combobox.set(default_value)

    def notify_set_changed(self, event):
        # set the macro keyboard's active set to set s
        active_set = int(self.stringvar.get())
        self.controller.notify({'active_set_changed':active_set})
        # print(f'changed active set to: {stringvar_set.get()}')

class DeviceSelector:
    def __init__(self, master, controller, devices=[], callback=None, default_value='', pos_x = 12, pos_y = 4, state='readonly'):
        self.controller = controller
        self.master = master
        self.devices = devices
        self.callback = callback

        self.frame = tk.Frame(master=self.master, relief=tk.GROOVE, borderwidth=1)
        self.frame.columnconfigure(0, weight=1, uniform="device")
        self.frame.columnconfigure(1, weight=1, uniform="device")

        self.label = tk.Label(master=self.frame, text='Devices:', relief=tk.GROOVE, borderwidth=0)
        self.label.grid(row=0, column=0, sticky='E')

        self.stringvar = tk.StringVar()
        self.combobox = ttk.Combobox(master=self.frame, textvariable=self.stringvar, width=4)
        self.combobox['values'] = [device.description for device in devices]
        self.combobox['state'] = state
        self.combobox.set(default_value) #self.combobox['values'][0]
        self.combobox.grid(row=0,column=1, columnspan=2, sticky="E")
        self.combobox.bind('<<ComboboxSelected>>', self.notify_device_changed)

        self.frame.grid(row=pos_y, column=pos_x, sticky='NSEW', columnspan=4)

    def set_controller(self, controller: object):
        self.controller = controller
        self.controller.add_subscriber(self)

    def refresh_with(self, devices):
        self.devices = devices # make a refresh device list
        self.combobox['values'] = [device.description for device in self.devices]
        # self.combobox.set(self.combobox['values'][0]) # set the default device

    def notify_device_changed(self, event):
        # set the app's active device
        chosen_device = self.stringvar.get()
        for device in self.devices:
            if(chosen_device == device.description):
                active_device = self.callback(device.name)
        self.controller.notify({'active_device_changed':active_device})

class MacroKeyboard:
    def __init__(self, master, controller, pos_x=1, pos_y=4, size_x=10, size_y=12):
        self.controller = controller
        self.master = master
        self.frame = tk.Frame(master=self.master, relief=tk.GROOVE, borderwidth=0)#, relief=tk.GROOVE, borderwidth=0)
        self.encoder = None
        self.keypad = None

        self.frame.grid(row=pos_y, column=pos_x, sticky='NSEW', rowspan=size_y, columnspan=size_x)
        self.frame.grid_rowconfigure(0, weight=1, uniform="keyboard") # makes the encoder frame
        self.frame.grid_rowconfigure(1, weight=1, uniform="keyboard") # and the keyboard frame uniform in size
        self.frame.grid_columnconfigure(0, weight=1)

    def refresh_with(self, AMK):
        self.encoder = Encoder(self.frame, self.controller, AMK)
        self.keypad = Keypad(self.frame, self.controller, AMK)

    def refresh(self):
        self.keypad.refresh()

class Keypad: # maybe make it a disposable obj, to be destroyed and re-built on every update?
    def __init__(self, master, controller, AMK):
        if(AMK):
            self.master = master
            self.controller = controller
            self.frame = tk.Frame(master=self.master, relief=tk.GROOVE, borderwidth=1)
            self.key_labels = []
            self.key_cbbx = []
            self.key_stringvar = []
            self.active_set = AMK['active_set']

            self.dropdown_width = 12
            self.style = ttk.Style()
            self.style.configure('Key.TCombobox', postoffset=(0, 0, self.dropdown_width, 0))
            
            # goes over the bindings for the active_set, and creates a label for each button
            for i in range(AMK['num_rows']):
                for j in range(AMK['num_cols']):
                    key_index = j + (i*AMK['num_cols'])
                    self.frame.rowconfigure(i, weight=1, uniform='key_height')
                    self.frame.columnconfigure(j, weight=1, uniform='key_width')
                    key_code = AMK['bindings'][AMK['active_set']][key_index]
                    key_name = amk.Keycodes.get_name_of(key_code).split("_",1)[1] # Remove the prefix
                    # self.key_labels.append(tk.Label(master=self.frame, text=key_name))
                    # self.key_labels[key_index].grid(row=i, column=j, sticky='NSEW')
                    # self.key_labels[key_index].update()
                    combobox_key, stringvar_key = self.create_key_combobox(self.frame, key_name)
                    self.key_cbbx.append(combobox_key)
                    self.key_stringvar.append(stringvar_key)
                    self.key_cbbx[key_index].grid(row=i, column=j, sticky='NSEW')
                    self.key_cbbx[key_index].update()
                
            self.frame.grid(row=1, column=0, sticky='NSEW')
                
    def create_key_combobox(self, key_code, key_name):
        stringvar = tk.StringVar()
        combobox = ttk.Combobox(self.frame, textvariable=stringvar, width=6, style='Key.TCombobox')
        combobox['values'] = amk.Keycodes.get_key_names()
        combobox['state'] = 'readonly'
        combobox.set(key_name)
        combobox.bind('<<ComboboxSelected>>',lambda event, arg=combobox: self.notify_key_changed(event, arg))
        return combobox, stringvar

    def notify_key_changed(self, event, combobox):
        key_name = combobox.get()
        key_code = amk.Keycodes.get_code_of(key_name)
        self.controller.notify({'key_changed':[self.active_set, self.key_cbbx.index(combobox), key_code]})
        print(f'binding for key: ({self.active_set},{self.key_cbbx.index(combobox)}) changed to: {key_name} (code: {key_code})')

    def refresh_with(self, AMK):
        print(f'{type(self)} refreshing')
        for i in range(AMK["num_rows"]):
            for j in range(AMK["num_cols"]):
                key_index = j + (i*AMK["num_cols"])
                key_code = AMK["bindings"][AMK["active_set"]][key_index]
                key_name = amk.Keycodes.get_key_name(key_code).split("_",1)[1] # Remove the prefix
                self.key_labels[key_index]['text'] = key_name
                # frame_keyboard.pack()
            
class Encoder:
    def __init__(self, master, controller, AMK):
        self.master = master
        self.frame = tk.Frame(master=self.master, relief=tk.GROOVE, borderwidth=1)#, width=int(3*WINDOW_WIDTH/GRID_COLS), height=int(3*WINDOW_HEIGHT/GRID_ROWS))
        self.frame.grid(row=0, column=0, sticky='NSEW')

    def refresh_with(self, AMK):
        print(f'{type(self)} refreshing')

if __name__ == '__main__':
    root = tk.Tk()
    RATIO = 1
    RATIO *= root.winfo_screenheight()/1080
    ASCPECT_RATIO = root.winfo_screenwidth()/root.winfo_screenheight()

    HEIGHT_SCALE_FACTOR = 2/3
    WIDTH_SCALE_FACTOR = HEIGHT_SCALE_FACTOR * 4/5

    WINDOW_WIDTH = int( root.winfo_screenheight() * RATIO * WIDTH_SCALE_FACTOR )
    WINDOW_HEIGHT = int( root.winfo_screenheight() * RATIO * HEIGHT_SCALE_FACTOR )

    # if(debug_resolution):
        #     print(f'screen dimensions: {root.winfo_screenwidth()}x{root.winfo_screenheight()}')
        #     print(f'window dimensions: {WINDOW_WIDTH}x{WINDOW_HEIGHT}')
        #     # print(f'aspect ratio: {ASCPECT_RATIO}')
        #     print(f'grid size: {grid_rows}x{grid_cols}')

    keypad = {'num_rows': 3, 'num_cols': 3, 'num_sets': 2, 'bindings': [[104, 105, 106, 107, 108, 109, 110, 111, 112], [30, 20, 9, 7, 33, 26, 5, 8, 21]], 'orientation': 3, 'active_set': 0}
    amk_1 = amk.AMK.init_from_dict(keypad)
    app_window = MainFrame(root, WINDOW_WIDTH, WINDOW_HEIGHT)
    app_window.refresh_with(keypad)
    root.mainloop()
    
