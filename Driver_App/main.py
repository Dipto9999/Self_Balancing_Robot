import asyncio
import threading as td

from kivy.app import App
from kivy.core.window import Window

from kivy.uix.tabbedpanel import TabbedPanel, TabbedPanelItem

from home_page import HomePageLayout
from dashboard_page import DashboardPageLayout

class AppLayout(TabbedPanel):
    def __init__(self, app, **kwargs):
        super().__init__(**kwargs)

        # Home Tab
        home_tab = TabbedPanelItem(text = 'Home')
        home_tab.add_widget(HomePageLayout(app))
        self.add_widget(home_tab)

        # Dashboard Tab
        dashboard_tab = TabbedPanelItem(text = 'Dashboard')
        dashboard_tab.add_widget(DashboardPageLayout())
        self.add_widget(dashboard_tab)

        self.default_tab = home_tab
        # Set default tab


class DriverApp(App):
    def build(self) -> AppLayout:
        self.async_loop = asyncio.new_event_loop() # Create Asyncio Event Loop
        td.Thread(target = self._start_async_loop, daemon = True).start() # Start Event Loop in Separate Thread

        # Configure Window
        Window.size = (1000, 700)
        Window.resizable = False
        self.title = "Robot Driver App"
        return AppLayout(app = self)

    def _start_async_loop(self):
        """Start Asyncio Event Loop."""
        asyncio.set_event_loop(self.async_loop)
        self.async_loop.run_forever()

if __name__ == "__main__":
    DriverApp().run()
