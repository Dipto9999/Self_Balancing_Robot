import asyncio
import threading as td

from kivy.app import App
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
        self.title = "Robot Driver App"
        # Create an asyncio event loop and store it as an attribute
        self.async_loop = asyncio.new_event_loop()

        # Start Asyncio Event Loop in Separate Thread
        td.Thread(target = self._start_async_loop, daemon = True).start()
        return AppLayout(app = self)

    def _start_async_loop(self):
        """Start the Asyncio Event Loop."""
        asyncio.set_event_loop(self.async_loop)
        self.async_loop.run_forever()

if __name__ == "__main__":
    DriverApp().run()
