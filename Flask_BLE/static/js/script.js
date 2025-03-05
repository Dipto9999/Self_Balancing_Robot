/*******************/
/* DOM References */
/*******************/

/* Device Selection and Connection */
const scanButton = document.getElementById('scanButton');
const deviceSelect = document.getElementById('deviceSelect');
const connectButton = document.getElementById('connectButton');
const disconnectButton = document.getElementById('disconnectButton');
const statusLabel = document.getElementById('statusLabel');

/* Movement Buttons */
const driveButton = document.getElementById('driveButton');
const downButton = document.getElementById('downButton');
const leftButton = document.getElementById('leftButton');
const rightButton = document.getElementById('rightButton');

/******************/
/* Device Actions */
/******************/

/* Scanning for BLE Devices */
scanButton.addEventListener('click', () => {
    statusLabel.textContent = "Scanning for BLE Devices...";

    fetch('/scan') // Send Scan Request
    .then(res => res.json()) // Parse JSON Response
    .then(devices => { // Iterate Over Devices
        if (devices.length > 0) {
            statusLabel.textContent = `Found ${devices.length} Devices.`;
            deviceSelect.innerHTML = '<option value="">Select Device</option>';

            devices.forEach(dev => {
                const option = document.createElement('option');
                // Set Device Name and Address
                option.textContent = `${dev.name}`;
                option.value = dev.address;

                deviceSelect.appendChild(option);
            });

            // Enable Device Select and Connect Button
            deviceSelect.disabled = false;
            connectButton.disabled = false;
        } else {
            statusLabel.textContent = "No BLE Devices Found.";

            // Disable Device Select and Connect Button
            deviceSelect.disabled = true;
            connectButton.disabled = true;
        }
    })
    .catch(err => { // Log Errors
        console.error('Scan Error:', err);
        statusLabel.textContent = "Scan Error.";
    });
});

/* Connecting to Selected Device */
connectButton.addEventListener('click', () => {
    const deviceName = deviceSelect.options[deviceSelect.selectedIndex].text;
    const deviceAddress = deviceSelect.value;

    statusLabel.textContent = "Connecting to " + deviceName + "...";
    if (!deviceAddress) {
      statusLabel.textContent = "No Device Selected.";
      return;
    }
    fetch('/connect', {
      method: 'POST', // Send POST Request
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ deviceAddress }) // Send Device Address
    })
    .then(res => res.json()) // Parse JSON Response
    .then(data => { // Update Status Label
        if (data.status === 'Connected') {
            statusLabel.textContent = "Connected!";

            // Disable Scan Button
            connectButton.disabled = true;
            disconnectButton.disabled = false;

            // Enable Movement Buttons
            driveButton.disabled = false;
            downButton.disabled = false;
            leftButton.disabled = false;
            rightButton.disabled = false;
        } else {
          statusLabel.textContent = "Connection Failed.";
        }
    })
    .catch(err => { // Log Errors
        console.error('Connect Error:', err);
        statusLabel.textContent = "Connect Error.";
    });
});

/* Disconnecting from Device */
disconnectButton.addEventListener('click', () => {
    fetch('/disconnect') // Send Disconnect Request
    .then(res => res.json()) // Parse JSON Response
    .then(data => { // Update Status Label
        statusLabel.textContent = data.status;

        // Enable Scan Button
        connectButton.disabled = false;
        disconnectButton.disabled = true;

        // Disable Movement Buttons
        driveButton.disabled = true;
        downButton.disabled = true;
        leftButton.disabled = true;
        rightButton.disabled = true;
    })
    .catch(err => {
        console.error('Disconnect Error:', err);
        statusLabel.textContent = "Disconnect Error.";
    });
});

/********************/
/* Movement Actions */
/********************/

/* Sending Movement Commands */
function sendCommand(cmd) {
    fetch('/move', {
      method: 'POST', // Send POST Request
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ command: cmd }) // Send Command
    })
    .then(res => res.json()) // Parse JSON Response
    .then(data => {
        statusLabel.textContent = data.msg || data.error;
        console.log(data.msg || data.error);
    })
    .catch(err => { // Log Errors
      console.error('Error Sending Command:', err);
      statusLabel.textContent = "Command Error.";
    });
}

/* Bind Movement Commands */
driveButton.addEventListener('click', () => sendCommand('^'));
downButton.addEventListener('click', () => sendCommand('v'));
leftButton.addEventListener('click', () => sendCommand('<'));
rightButton.addEventListener('click', () => sendCommand('>'));