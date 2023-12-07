#!/usr/bin/python
import serial, sys, io, os
import intelhex
import time

# MCUBOOT_MAGIC_NUMBER = 0x96f3b83d
MCUBOOT_MAGIC_NUMBER = 0x3db8f396   # Magic number reversed
UART_OAD_BLOCK_SIZE = 64

class OADWriteBin():
  def __init__(self, port=None, oad_file=None, serial_setup_ext=None, transmit_ext=None, receive_ext=None):

    if port is not None:
      self.port = port
    elif (port is None) and (serial_setup_ext is not None and transmit_ext is not None and receive_ext is not None):
      self.serial_setup = serial_setup_ext
      self.transmit = transmit_ext
      self.receive = receive_ext
    else:
      print("You must either set port or provide callbacks for serial_setup_ext, transmit_ext, and receive_ext.")
      sys.exit(1)

    if oad_file is None:
      print("You must provide an OAD image file.")
      sys.exit(1)

    self.oad_file = oad_file

    self.f_stream = None
    self.ser = None

    self.open_stream()
    self.serial_setup()

  def _convert_file_to_bin(self):
    """
    Converts .hex file to .bin if .hex is provided
    """
    if self.oad_file.endswith(".hex"):
      file_bin = os.path.splitext(self.oad_file)[0] + ".bin"
      intelhex.hex2bin(self.oad_file, file_bin)
      self.oad_file = file_bin

  def __print_bytes(self, bytes_buffer):
    hex_list = list(map(hex, bytes_buffer))
    hex_list_len = len(hex_list)
    line_len = 16
    line_count = int(hex_list_len / line_len) + (hex_list_len % line_len > 0)

    start = 0
    for i in range(0, line_count):
        print(hex_list[start:start+line_len])
        start += line_len

  def open_stream(self):
    """
    Opens a file stream with the OAD file
    """
    self._convert_file_to_bin()
    self.f_stream = open(self.oad_file, 'rb')

  def close_stream(self):
    """
    Closes the file stream for the OAD file
    """
    self.f_stream.close()

  def is_mcuboot_used(self):
    """
    Checks if the OAD image is an MCUBoot image

    Returns:
      bool: True or False
    """
    with open(self.oad_file, 'rb') as f:
      magic_number = f.read(4)
      magic_number = int(str(magic_number.hex()), 16)

    if(MCUBOOT_MAGIC_NUMBER == magic_number):
      return True
    return False

  def serial_setup(self):
    """
    Sets up serial port
    """
    self.ser = serial.Serial()

    self.ser.port = self.port
    self.ser.baudrate = 115200
    self.ser.bytesize = serial.EIGHTBITS
    self.ser.parity = serial.PARITY_NONE
    self.ser.stopbits = serial.STOPBITS_ONE
    self.ser.timeout = 240 # Non-block read (20s time out)
    self.ser.writeTimeout = 1

    try:
      self.ser.open()
    except Exception as e:
      print("Error opening serial port: %s", str(e))
      exit()

    if self.ser.isOpen():
      try:
        self.ser.flushInput()  # Flush input buffer, discarding all its contents
        self.ser.flushOutput() # Flush output buffer, aborting current output and discard all that is in buffer
      except Exception as e1:
        print("Error communicating...: %s", str(e1))
    else:
      print("Cannot open serial port")

  def get_size(self):
    """
    Gets size of the OAD file

    Returns:
      int: Size of OAD file
    """
    if self.f_stream is None:
      print("Make sure to call self.open_stream() first")
      return 0

    self.f_stream.seek(0, os.SEEK_END)
    size = self.f_stream.tell()
    self.f_stream.seek(0,0)
    return size

  def transmit(self, tx_bytes):
    """
    Transmits an array of bytes via the serial port

    Args:
      tx_bytes (bytes): Bytes to transmit
    """
    self.__print_bytes(tx_bytes)
    self.ser.write(tx_bytes)

  def flush(self):
    """
    Flushes the serial port
    """
    # self.ser.flush()
    pass

  def receive(self, count):
    """
    Receives bytes from the serial port

    Args:
      count (int): Number of bytes to receive

    Returns:
      bytes: Received bytes - on byte object with <count> bytes
    """
    rx_bytes = self.ser.read(count)
    return rx_bytes

  def transmit_header(self):
    """
    Transmits the header of the OAD image for validation by the OAD Server
    """
    if (self.is_mcuboot_used()):
      mcuboot_header = self.f_stream.read(32)
      entry_section = self.f_stream.read(256)

    byte_idx = 0
    # 22 Byte image ID Payload (+ removed bytes)
    while(byte_idx < (22 + 14)):
      # Extract Image Identify Payload from header
      # Discard CRC
      if((byte_idx == 8) or (byte_idx == 9) or (byte_idx == 10) or (byte_idx == 11) or
        # Discard Tech Type
        (byte_idx == 15) or (byte_idx == 16) or
        # Discard Img Valid
        (byte_idx == 20) or (byte_idx == 21) or (byte_idx == 22) or (byte_idx == 23) or
        # Discard Prg Entry
        (byte_idx == 28) or (byte_idx == 29) or (byte_idx == 30) or (byte_idx == 31)):
        # Read and discard byte
        dummy_byte = self.f_stream.read(1)
        byte_idx += 1
        continue

      tx_bytes = self.f_stream.read(1)
      byte_idx += 1

      self.transmit(tx_bytes)
      self.flush()

    self.f_stream.seek(0,0)
    print("OAD Header transmitted.")

  def transmit_oad_blocks(self):
    """
    Transmits the entire OAD image block by block
    """
    blocks_sent = 0
    size = self.get_size()

    self.f_stream.seek(0,0)
    # sys.stdout.flush()

    while(size > 0):

      block_num = bytes(reversed(self.receive(2)))

      if len(block_num) <= 0:
        continue

      print("Block number: " + block_num[1].to_bytes(1, "little").hex() + "" + block_num[0].to_bytes(1, "little").hex())

      self.transmit(block_num)
      self.flush()

      block_byte_idx = 0
      if((size == 0) or (size < UART_OAD_BLOCK_SIZE)):
        block_size = size
      else:
        block_size = UART_OAD_BLOCK_SIZE

      print("Reading: " + str(block_size) + "bytes")
      print("Blocks sent: " + str(blocks_sent))
      blocks_sent += 1

      block = self.f_stream.read(UART_OAD_BLOCK_SIZE)
      self.transmit(block)

      size = size - UART_OAD_BLOCK_SIZE

      remainder = UART_OAD_BLOCK_SIZE - block_size

    # Send dummy bytes to complete the block
    print("Writing " + str(remainder) + " dummy bytes")
    dummy_bytes = [0xFF] * remainder
    self.transmit(dummy_bytes)

    print("All blocks sent")

def main():
  oad_write_bin = OADWriteBin(port=sys.argv[1], oad_file=sys.argv[2])
  oad_write_bin.transmit_header()
  oad_write_bin.transmit_oad_blocks()

if __name__ == '__main__':
  main()
