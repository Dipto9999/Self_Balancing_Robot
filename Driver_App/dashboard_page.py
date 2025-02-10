from kivy.uix.gridlayout import GridLayout

from kivy.uix.label import Label

class DashboardPageLayout(GridLayout):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)

        self.cols = 1
        self.spacing = 10
        self.padding = 10

        self.status_label = Label(text = "Dashboard", size_hint_y = None, height = 50)
        self.add_widget(self.status_label)