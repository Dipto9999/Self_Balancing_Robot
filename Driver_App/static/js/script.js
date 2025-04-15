let connected = false;

/*******************/
/* DOM References */
/*******************/

/* Device Selection and Connection */
const statusLabel = document.getElementById('statusLabel');
const scanButton = document.getElementById('scanButton');
const deviceSelect = document.getElementById('deviceSelect');
const connectionButton = document.getElementById('connectionButton');

/* Command Buttons */
const entryButton = document.getElementById('entryButton');
const forwardButton = document.getElementById('forwardButton');
const reverseButton = document.getElementById('reverseButton');
const idleButton = document.getElementById('idleButton');
const leftButton = document.getElementById('leftButton');
const rightButton = document.getElementById('rightButton');

/******************/
/* Device Actions */
/******************/

/* Scanning for WALL-E */
scanButton.addEventListener('click', () => {
    statusLabel.textContent = "Searching for WALL-E...";

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
            connectionButton.disabled = false;
            connectionButton.style.backgroundColor = "rgb(55, 92, 194)";
        } else {
            statusLabel.textContent = "WALL-E MIA";

            // Disable Device Select and Connect Button
            deviceSelect.disabled = true;
            connectionButton.disabled = true;
        }
    })
    .catch(err => { // Log Errors
        console.error('Scan Error:', err);
        statusLabel.textContent = "Scan Error.";
    });
});

/* Connecting to Selected Device */
connectionButton.addEventListener('click', () => {
    const deviceName = deviceSelect.options[deviceSelect.selectedIndex].text;
    const deviceAddress = deviceSelect.value;

    if (!connected) { // Attempt Connection
        if (!deviceAddress) {
            statusLabel.textContent = "No Device Selected.";
            return;
        }
        statusLabel.textContent = "Connecting to " + deviceName + "...";

        fetch('/connect', {
            method: 'POST', // Send POST Request
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ deviceAddress }) // Send Device Address
        })
        .then(res => res.json()) // Parse JSON Response
        .then(data => { // Update Status Label
            if (data.status === 'Connected') {
                connected = true;
                statusLabel.textContent = "Connected!";
                connectionButton.textContent = "Disconnect";
                connectionButton.style.backgroundColor = "rgb(224, 127, 71)"; // Change Button Color

                // Enable Command Buttons
                entryButton.disabled = false;
                forwardButton.disabled = false;
                reverseButton.disabled = false;
                idleButton.disabled = false;
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
    } else { // Disconnecting from Device
        statusLabel.textContent = "Disconnecting...";

        fetch('/disconnect')
        .then(res => res.json())
        .then(data => {
            connected = false;
            statusLabel.textContent = data.status;
            connectionButton.textContent = "Connect";
            connectionButton.style.backgroundColor = "rgb(55, 92, 194)";

            // Disable Command Buttons
            entryButton.disabled = true;
            forwardButton.disabled = true;
            reverseButton.disabled = true;
            idleButton.disabled = true;
            leftButton.disabled = true;
            rightButton.disabled = true;
        })
        .catch(err => {
            console.error('Disconnect Error:', err);
            statusLabel.textContent = "Disconnect Error.";
        });
    }
});

/********************/
/* Movement Actions */
/********************/

function sendEntryCommand() {
    const entryCommand = document.getElementById('entryInput').value; // Get Entry Input Value
    if (entryCommand) {
        fetch('/command', {
            method: 'POST', // Send POST Request
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ command: entryCommand }) // Send Entry Command
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
    } else {
        statusLabel.textContent = "No Command Provided.";
    }
}

/* Sending Movement Commands */
function sendCommand(cmd) {
    fetch('/command', {
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
entryButton.addEventListener('click', () => sendEntryCommand());
forwardButton.addEventListener('click', () => sendCommand('^'));
reverseButton.addEventListener('click', () => sendCommand('v'));
idleButton.addEventListener('click', () => sendCommand('X'));
leftButton.addEventListener('click', () => sendCommand('<'));
rightButton.addEventListener('click', () => sendCommand('>'));