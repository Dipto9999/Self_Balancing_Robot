import serial
class ArduinoSerial(serial.Serial) :
    def __init__(self, port = 'COM3', baudrate = 9600):
        super().__init__(port = port, baudrate = baudrate)

        if self.isOpen() :
            print('Serial Port is Open')
        else:
            print('Serial Port is Closed')

    def close(self) :
        super().close()
        print('Serial Port is Closed')

    def reconnect(self) :
        self.close()

        while not self.isOpen() :
            try:
                self.open()
            except:
                pass
        print('Serial Port is Open')