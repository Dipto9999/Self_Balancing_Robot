import time
import serial

import pandas as pd
import datetime as dt
import matplotlib.pyplot as plt
import matplotlib.animation as animation

from IPython.display import display

###############
### Classes ###
###############

class Serial(serial.Serial) :
    def __init__(self,
            port = 'COM3',
            baudrate = 115200,
        ):
        super().__init__(
            port = port,
            baudrate = baudrate,
        )

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

class StripChart :
    def __init__(self, conn, data_size = 50, ylim = 25) :
        self.conn = conn
        self.fig = plt.figure(
            figsize = (900 / 100, 755 / 100)
        )

        self.data_size, self.ylim = data_size, ylim

        self.t_data = []
        self.accelerometer_data = []
        self.gyroscope_data = []
        self.complementary_data = []

        self.ax = self.fig.add_subplot(111)
        self.ax.set_title('Angle Strip-Chart')
        self.ax.grid()

        self.ax.set_ylim(-self.ylim, self.ylim) # Angle Expected (0, 360°)

        self.ax.set_xlabel('Samples')
        self.ax.set_ylabel('Angle (°)')

        self.accelerometer_line, = self.ax.plot([], [], linestyle = 'dashed', lw = 1.5, label = 'Accelerometer Angle (°)', color = 'lightblue')
        self.gyroscope_line, = self.ax.plot([], [], linestyle = 'dashed', lw = 1.5, label = 'Gyroscope Angle (°)', color = 'tomato')
        self.complementary_line, = self.ax.plot([], [], lw = 1.5, label = 'Complementary Angle (°)', color = 'purple')
        if conn:
            self.start_animation()
            self.fig.canvas.manager.set_window_title("Arduino Real-Time Plot")

            self.fig.subplots_adjust(bottom = 0.2)
            self.ax.legend(
                loc = 'upper center',
                bbox_to_anchor = (0.5, -0.2),  # Position Above Plot with Padding
                ncol = 3, framealpha = 0.9,  # 3 Columns, Transparent Background
                fontsize = 10,
            )

            self.ax.grid(True)

            plt.show()
            self.conn.close() # Close Serial Connection When Plot Closed

            fig_name = f"Angle_Data_{dt.datetime.now().strftime('%Y%m%d_%H%M%S')}"
            self.save_logs(fig_name)
            self.save_fig(fig_name)

    def save_fig(self, fig_name) :
        file_path = f"Figures/{fig_name}.jpg"
        self.fig.savefig(file_path, format = 'jpg', dpi = 800)
        print(f"Figure Saved to {file_path}")

    def save_logs(self, fig_name):
        data_df = pd.DataFrame({
            'Sample' : self.t_data,
            'Accelerometer Angle (°)' : self.accelerometer_data,
            'Gyroscope Angle (°)' : self.gyroscope_data,
            'Complementary Angle (°)' : self.complementary_data
        })

        file_path = f"LogBook/{fig_name}.csv"
        data_df.to_csv(file_path, index = False)

        print(f"Data Exported to {file_path}\n\nData Preview:\n")
        display(data_df.head(2))

    def start_animation(self):
        self.animation = animation.FuncAnimation(
            fig = self.fig,
            func = self.run,
            frames = self.data_size,
            blit = False, # Prevent Re-rendering Entire Plot
            interval = 50, # 50 ms Interval
        )

    def run(self, frame):
        self.rx_angle()

        self.accelerometer_line.set_data(self.t_data, self.accelerometer_data)
        self.gyroscope_line.set_data(self.t_data, self.gyroscope_data)
        self.complementary_line.set_data(self.t_data, self.complementary_data)

        # Adjust x Limits to Scroll Forward
        if (len(self.t_data) > 0) and (self.t_data[-1] > self.data_size):
            self.ax.set_xlim(self.t_data[0], self.t_data[0] + round(1.25 * self.data_size))
        else:
            self.ax.set_xlim(0, round(1.25 * self.data_size))

    def read_serial(self) :
        if (self.conn is not None) :
            return self.conn.readline().decode('ascii').strip('\r').strip('\n')
        else :
            return None

    def rx_angle(self) :
        try:
            self.conn.write(b'A') # Request Serial Transmission of Angle
        except serial.SerialException:
            self.conn.reconnect()

        try:
            arduinoStream: list = str(self.read_serial()).rstrip('\r').split(' ')

            # print(f"Incoming Value: {arduinoStream}")

            if len(arduinoStream) != 3:
                raise ValueError

            accelerometer_angle: float = float(arduinoStream[0])
            gyroscope_angle: float = float(arduinoStream[1])
            complementary_angle = float(arduinoStream[2])

            print(f"Accelerometer Angle: {accelerometer_angle}°")
            print(f"Gyroscope Angle: {gyroscope_angle}°")
            print(f"Complementary Angle: {complementary_angle}°")

            # Append Data
            if len(self.t_data) > 0:
                self.t_data.append(self.t_data[-1]+1)
            else:
                self.t_data.append(1)
            self.accelerometer_data.append(accelerometer_angle)
            self.gyroscope_data.append(gyroscope_angle)
            self.complementary_data.append(complementary_angle)

            self.t_data = self.t_data[-self.data_size:]
            self.accelerometer_data = self.accelerometer_data[-self.data_size:]
            self.gyroscope_data = self.gyroscope_data[-self.data_size:]
            self.complementary_data = self.complementary_data[-self.data_size:]
        except serial.SerialException:
            self.conn.reconnect()
        except ValueError:
            pass

if __name__ == "__main__":
    conn = Serial()
    time.sleep(2) # Time Delay for Arduino Serial Initialization

    stripchat = StripChart(conn = conn)
