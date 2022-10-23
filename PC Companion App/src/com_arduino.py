# 12 Mar 2014

# in case any of this upsets Python purists it has been converted from an equivalent JRuby program

# this is designed to work with ... ArduinoPC.ino ...

# the purpose of this program and the associated Arduino program is to demonstrate a system for sending 
#   and receiving data between a PC and an Arduino.

# The key functions are:
#    sendToArduino(str) which sends the given string to the Arduino. The string may 
#                       contain characters with any of the values 0 to 255
#
#    recvFromArduino()  which returns an array. 
#                         The first element contains the number of bytes that the Arduino said it included in
#                             message. This can be used to check that the full message was received.
#                         The second element contains the message as a string


# the overall process followed by the demo program is as follows
#   open the serial connection to the Arduino - which causes the Arduino to reset
#   wait for a message from the Arduino to give it time to reset
#   loop through a series of test messages
#      send a message and display it on the PC screen
#      wait for a reply and display it on the PC

# to facilitate debugging the Arduino code this program interprets any message from the Arduino
#    with the message length set to 0 as a debug message which is displayed on the PC screen

# the actual process of sending a message to the Arduino involves
#   prefacing the message with a byte value of 254 (startMarker)
#   following that startMarker with a byte whose value is the number of characters in the original message
#   then the message follows
#      any bytes in the message with values of 253, 254 or 255 into a pair of bytes
#          253 0    253 1   or 253 2       as appropriate
#   suffixing the message with a byte value of 255 (endMarker)


# receiving a message from the Arduino involves
#    waiting until the startMarker is detected
#    saving all subsequent bytes until the end marker is detected
#    converting the pairs of bytes (253 0 etc) back into the intended single byte



# NOTES
#       this program does not include any timeouts to deal with delays in communication
#
#       for simplicity the program does NOT search for the comm port - the user must modify the
#         code to include the correct reference.
#         search for the line "ser = serial.Serial("/dev/ttyS80", 57600)"
#
#       the function bytesToString(str) is just a convenience to show the contents of a string as
#          a series of byte values to make it easy to verify data with non-ascii characters
#
#       this program does NOT include a checkbyte that could be used to verify that there are no
#          errors in the message. This could easily be added.
#
#       as written the Arduino program can only receive a maximum of 16 bytes. 
#          This must include the start- and end-markers, the length byte and any extra bytes needed 
#             to encode values of 253 or over
#          the arduino program could easily be modified to accept longer messages by changing
#                #define maxMessage 16
#
#       as written the Arduino program does NOT check for messages that are too long
#         it is assumed that the PC program will ensure compliance
#         extra code could be added to the Arduino program to deal with too-long messages
#           but it would add a lot of code that may confuse this demo.

import serial
import time
import serial.tools.list_ports
import logging
import inspect

BAUDRATE = 56700
TIMEOUT = 1

startMarker = 123
endMarker = 125
specialByte = 253

arduino_ready = False
waitingForReply = False

message_queue = []
ser = None

def send_recv_queue(msg_queue):
  numLoops = len(msg_queue)
  n = 0
  waitingForReply = False

  while n < numLoops:
    print("LOOP " + str(n))
    msg_str = msg_queue[n]

    if ser.in_waiting == 0 and waitingForReply == False:
      sendToArduino(msg_str)
      print("=====sent from PC==========")
      print("LOOP NUM " + str(n))
      print("BYTES SENT -> " + bytesToString(msg_str))
      print("TEST STR " + msg_str)
      print("===========================")
      waitingForReply = True

    if ser.in_waiting > 0:
      dataRecvd = recvFromArduino()

      if dataRecvd[0] == 0: #show debug message and continue waiting
        displayDebug(dataRecvd[1])

      if dataRecvd[0] > 0:
        displayData(dataRecvd[1])
        print("Reply Received")
        n += 1
        waitingForReply = False

      print()
      print("===========")
      print()

      time.sleep(0.3)

def send_recv(message, print_message=False):
  global message_queue

  message_queue.append(message)
  numLoops = len(message_queue)
  n = 0
  waitingForReply = False

  while n < numLoops:
    print("LOOP " + str(n))
    msg_str = message_queue[n]

    if ser.in_waiting == 0 and waitingForReply == False:
      sendToArduino(msg_str)
      if print_message:
        print("=====sent from PC==========")
        print("LOOP NUM " + str(n))
        print("BYTES SENT -> " + bytesToString(msg_str))
        print("TEST STR " + msg_str)
        print("===========================")
      waitingForReply = True

    if ser.in_waiting > 0:
      dataRecvd = recvFromArduino()

      if dataRecvd[0] == 0: # show debug message and continue waiting
        displayDebug(dataRecvd[1])

      if dataRecvd[0] > 0:
        displayData(dataRecvd[1])
        print("Reply Received")
        n += 1
        waitingForReply = False

      print()
      print("===========")
      print()

      time.sleep(0.3)

def send_recv1(message, proc_func=None, print_message=True): # NTS: return true for success? timeout?
  """Adds message to queue, sends messages in queue, and directs replies\
  to the processing function.

  :param message: A string to send
  :param proc_func: A function to use for processing the replies
  """
  global message_queue

  message_queue.append(message)
  numLoops = len(message_queue)
  n = 0
  waitingForReply = False

  while n < numLoops:
    print("LOOP " + str(n))
    msg_str = message_queue[n]

    if ser.in_waiting == 0 and waitingForReply == False:
      sendToArduino(msg_str)
      if print_message:
        print("=====sent from PC==========")
        print("LOOP NUM " + str(n))
        print("BYTES SENT -> " + bytesToString(msg_str))
        print("TEST STR " + msg_str)
        print("===========================")
      waitingForReply = True

    if ser.in_waiting > 0:
      dataRecvd = recvFromArduino()

      if dataRecvd[0] == 0: #show debug message and continue waiting
        if print_message:
          displayDebug(dataRecvd[1])
        # log message here

      if dataRecvd[0] > 0:
        if print_message:
          displayData(dataRecvd[1])
        print("Reply Received")
        if proc_func is not None:
          proc_func(dataRecvd[1])
        n += 1
        waitingForReply = False

      print()
      print("===========")
      print()

      time.sleep(0.3)

message_queue = []

def send_recv2(message='', print_message=False): # NTS: return true for success? timeout?
  """Adds message to queue, sends messages in queue, and directs replies\
  to the processing function.

  :param message: A string to send
  :param proc_func: A function to use for processing the replies
  """
  global message_queue

  answer_queue = []

  if message:
    message_queue.append(message)

  numLoops = len(message_queue)
  n = 0
  waitingForReply = False

  while n < numLoops:
    print("LOOP " + str(n))
    msg_str = message_queue[n]

    if ser.in_waiting == 0 and waitingForReply == False:
      sendToArduino(msg_str)
      print("=====sent from PC==========")
      print("LOOP NUM " + str(n))
      print("BYTES SENT -> " + bytesToString(msg_str))
      print("TEST STR " + msg_str)
      print("===========================")
      waitingForReply = True

    if ser.in_waiting > 0:
      dataRecvd = recvFromArduino()

      if dataRecvd[0] == 0: #show debug message and continue waiting
        displayDebug(dataRecvd[1])
        # log message here

      if dataRecvd[0] > 0:
        displayData(dataRecvd[1])
        print("Reply Received")
        answer_queue.append(dataRecvd[1])
        print('reply: ' + dataRecvd[1]) ## remove
        n += 1
        waitingForReply = False

      print()
      print("===========")
      print()

      time.sleep(0.3)
  message_queue = []
  return answer_queue


def enqueue_message(message):
    message_queue.append(message)

# def enqueue_message(header, opcode, message):
#   if type(message) == list: # "Flatten" the list to 1d
#     temp_list = []
#     for sublist in lst:
#         temp_list.extend(sublist)
#     message_queue.append('sb' + ''.join(''.join('shi'+chr(0)) for n in range(18)))


#=====================================

#  Serial Ports

#=====================================

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
  # global BAUDRATE, TIMEOUT
  logging.info(f'Scanning Arduino devices for serial ports...')
  serial_ports = []
  for port in list_comports():
      ser = serial.Serial()
      ser.baudrate = BAUDRATE
      ser.port = port.name
      ser.timeout = TIMEOUT
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
  logging.warning(f'Failed. Comport {comport_name} not found in serial_ports')
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

# def switch_device_to(comport_name): #NTS: maybe do the update to amk obj in the gui?
#   """Switches active device to device at COM port comport_name.

#   This function closes the active connection, if it exists, updates
#   the global AMK object, and updates it with the new AMK device's data
  
#   :param comport_name: The name of the port to activate
#   """
#   close_serial_at(current_serial_port())

#   global current_amk 
#   current_amk = amk.AMK(arduino_callback=save_to_arduino,
#                         serial_port=open_serial_at(comport_name))
#   wait_for_arduino() # Handshake with AMK device
#   parse_message(get_keypad(), current_amk) # Send and receive
#   logging.info(f'successfully switched to device: {current_amk.name}')
#   return current_amk

def switch_port_to(comport_name): #NTS: maybe do the update to amk obj in the gui?
  """Switches active device to device at COM port comport_name.

  This function closes the active connection, and opens serial at
  new_port.
  
  :param comport_name: The name of the port to activate
  """

  close_serial_at(current_serial_port())
  new_port = open_serial_at(comport_name)
  logging.info(f'successfully switched port to: {new_port.name}') #NTS: check what name it presents
  return new_port

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

def find_baudrate(serial_port): # WIP
  ser.timeout = 0.5
  for baudrate in serial_port.BAUDRATES:
      if 9600 <= baudrate <= 115200:
          serial_port.baudrate = baudrate
          serial_port.write(packet)
          resp = serial_port.read()
          if resp != '':
              break
  if serial_port.baudrate > 115200:
      raise RuntimeError("Couldn't find appropriate baud rate!")

#=====================================

#  Serial messaging

#=====================================

def sendToArduino(sendStr):
  global startMarker, endMarker
  txLen = chr(len(sendStr))
  adjSendStr = encodeHighBytes(sendStr)
  adjSendStr = chr(startMarker) + txLen + adjSendStr + chr(endMarker)
  ser.write(adjSendStr.encode('utf-8'))


#======================================

def recvFromArduino():
  global startMarker, endMarker
  
  ck = ""
  x = "z" # any value that is not an end- or startMarker
  byteCount = -1 # to allow for the fact that the last increment will be one too many
  
  # wait for the start character
  while  ord(x) != startMarker:
    x = ser.read()
  
  # save data until the end marker is found
  while ord(x) != endMarker:
    ck = ck + chr(ord(x)) 
    x = ser.read()
    byteCount += 1
    
  # save the end marker byte
  ck = ck + chr(ord(x)) 
  
  returnData = []
  returnData.append(ord(ck[1]))
  returnData.append(decodeHighBytes(ck[1:-1])) # removes start-end markers, count byte
  print("RETURNDATA " + str(returnData[0]))
  
  return(returnData)

#======================================

def encodeHighBytes(inStr):
  global specialByte
  
  outStr = ""
  s = len(inStr)
  
  for n in range(0, s):
    x = ord(inStr[n])
    
    if x >= specialByte:
       outStr = outStr + chr(specialByte)
       outStr = outStr + chr(x - specialByte)
    else:
       outStr = outStr + chr(x)
       
  print("encINSTR  " + bytesToString(inStr))
  print("encOUTSTR " + bytesToString(outStr))

  return(outStr)


#======================================

def decodeHighBytes(inStr):

  global specialByte
  
  outStr = ""
  n = 0
  
  while n < len(inStr):
    if ord(inStr[n]) == specialByte:
      n += 1
      x = chr(specialByte + ord(inStr[n]))
    else:
      x = inStr[n]
      outStr = outStr + x
      n += 1
     
  print("decINSTR  " + bytesToString(inStr))
  print("decOUTSTR " + bytesToString(outStr))

  return(outStr)


#======================================

def displayData(data):

  n = len(data) - 3

  print("NUM BYTES SENT->   " + str(ord(data[0])))
  print("DATA RECVD BYTES-> " + bytesToString(data[3:]))
  print("DATA RECVD CHARS-> " + data[3:])


#======================================

def bytesToString(data):

  byteString = ""
  n = len(data)
  
  for s in range(n):
    byteString = byteString + str(ord(data[s]))
    byteString = byteString + "-"
    
  return(byteString)


#======================================

def displayDebug(debugStr):

   n = len(debugStr) - 3
   print("DEBUG MSG-> " + debugStr[1:])


#============================

def waitForArduino():

   # wait until the Arduino sends 'Arduino Ready' - allows time for Arduino reset
   # it also ensures that any bytes left over from a previous message are discarded
   
  global endMarker
  
  msg = ""
  while msg.find("Arduino Ready") == -1:

    while ser.inWaiting() == 0:
      x = 'z'

    # then wait until an end marker is received from the Arduino to make sure it is ready to proceed
    x = "z"
    while ord(x) != endMarker: # gets the initial debugMessage
      x = ser.read()
      msg = msg + chr(ord(x))


    displayDebug(msg)
    print()
    

#======================================

# THE DEMO PROGRAM STARTS HERE

#======================================

if __name__ == '__main__':

  import serial
  import time

  # NOTE the user must ensure that the next line refers to the correct comm port
  ser = serial.Serial("COM4", 57600)


  startMarker = 123
  endMarker = 125
  specialByte = 253

  print(f"Sent {{{chr(2)}ao}}, waiting for arduino")
  sendToArduino("ao")
  waitForArduino()

  print("Arduino is ready")

  message_queue.append('gb')
  # message_queue.append('sb' + ''.join(''.join('kh'+chr(0)) for n in range(18)))
  message_queue.append('sb' + ''.join(''.join('shi'+chr(0)) for n in range(18)))
  # message_queue.append('gb')

  print(send_recv2('gb', False))

  # testData = []
  # testData.append('gb')
  # testData.append('sb' + ''.join(''.join('kh'+chr(0)) for n in range(18)))
  # testData.append('gb')
  # # testData.append("b" + chr(16) + chr(32) + chr(253) + chr(255) + chr(254) + chr(253) + chr(0))
  # testData.append('ac')

  # numLoops = len(testData)
  # n = 0
  # waitingForReply = False

  # while n < numLoops:
  #   print("LOOP " + str(n))
  #   teststr = testData[n]

  #   if ser.in_waiting == 0 and waitingForReply == False:
  #     sendToArduino(teststr)
  #     print("=====sent from PC==========")
  #     print("LOOP NUM " + str(n))
  #     print("BYTES SENT -> " + bytesToString(teststr))
  #     print("TEST STR " + teststr)
  #     print("===========================")
  #     waitingForReply = True
  #     time.sleep(2)

  #   if ser.in_waiting > 0:
  #     dataRecvd = recvFromArduino()

  #     if dataRecvd[0] == 0: #show debug message and continue waiting
  #       displayDebug(dataRecvd[1]) # debug messages don't advance the loop

  #     if dataRecvd[0] > 0:
  #       displayData(dataRecvd[1])
  #       print("Reply Received")
  #       n += 1
  #       waitingForReply = False

  #     print()
  #     print("===========")
  #     print()

  #     time.sleep(0.3)

  ser.close

