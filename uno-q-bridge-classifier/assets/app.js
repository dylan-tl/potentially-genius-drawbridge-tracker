// SPDX-FileCopyrightText: Copyright (C) Arduino s.r.l. and/or its affiliated companies
//
// SPDX-License-Identifier: MPL-2.0

const recentDetectionsElement = document.getElementById('recentClassifications');
const feedbackContentElement = document.getElementById('feedback-content');
const MAX_RECENT_SCANS = 5;
let scans = [];
const socket = io(`http://${window.location.host}`); // Initialize socket.io connection
let errorContainer = document.getElementById('error-container');

// Start the application
document.addEventListener('DOMContentLoaded', () => {
    initSocketIO();
    renderFeedbackPlaceholder();
    renderClasses();

    const feedbackPopoverText = "Shows whether the bridge is currently UP or DOWN, based on the model's confidence.";

    document.querySelectorAll('.info-btn.feedback').forEach(img => {
        const popover = img.nextElementSibling;
        img.addEventListener('mouseenter', () => {
            popover.textContent = feedbackPopoverText;
            popover.style.display = 'block';
        });
        img.addEventListener('mouseleave', () => {
            popover.style.display = 'none';
        });
    });
});

function initSocketIO() {
    socket.on('connect', () => {
        if (errorContainer) {
            errorContainer.style.display = 'none';
            errorContainer.textContent = '';
        }
    });

    socket.on('disconnect', () => {
        if (errorContainer) {
            errorContainer.textContent = 'Connection to the board lost. Please check the connection.';
            errorContainer.style.display = 'block';
        }
    });

    socket.on('classifications', async (message) => {
        recordClassification(message);
        renderClasses();
    });

    socket.on('bridge_state', (message) => {
        const { is_up } = JSON.parse(message);
        showDetection(is_up ? 'up' : 'down');
    });
}

function renderFeedbackPlaceholder() {
    feedbackContentElement.innerHTML = `
        <img src="img/stars.svg" alt="Stars">
        <p class="feedback-text">Bridge state will appear here</p>
    `;
}

function recordClassification(newDetection) {
    scans.unshift(newDetection);
    if (scans.length > MAX_RECENT_SCANS) { scans.pop(); }
}

function renderClasses() {
    // Clear the list
    recentDetectionsElement.innerHTML = ``;

    if (scans.length === 0) {
        recentDetectionsElement.innerHTML = `
            <div class="no-recent-scans">
                <img src="./img/no-face.svg">
                No bridge state detected yet
            </div>
        `;
        return;
    }

    scans.forEach((iscan) => {
        try {
            const iiscan = JSON.parse(iscan);

            if (iiscan.length === 0) {
                return; // Skip empty detection arrays
            }

            iiscan.forEach((scan) => {
                const row = document.createElement('div');
                row.className = 'scan-container';

                // Create a container for content and time
                const cellContainer = document.createElement('span');
                cellContainer.className = 'scan-cell-container cell-border';

                // Content (text + icon)
                const contentText = document.createElement('span');
                contentText.className = 'scan-content';
                const value = scan.confidence;
                const result = Math.floor(value * 1000) / 10;
                contentText.innerHTML = `${result}% - ${scan.content}`;

                // Time
                const timeText = document.createElement('span');
                timeText.className = 'scan-content-time';
                timeText.textContent = new Date(scan.timestamp).toLocaleString('it-IT').replace(',', ' -');

                // Append content and time to the container
                cellContainer.appendChild(contentText);
                cellContainer.appendChild(timeText);

                row.appendChild(cellContainer);
                recentDetectionsElement.appendChild(row);
            });
        } catch (e) {
            console.error("Failed to parse scan data:", iscan, e);
            // Display an error in the list itself
            if(recentDetectionsElement.getElementsByClassName('scan-error').length === 0) {
                const errorRow = document.createElement('div');
                errorRow.className = 'scan-error';
                errorRow.textContent = `Error processing detection data. Check console for details.`;
                recentDetectionsElement.appendChild(errorRow);
            }
        }
    });
}

function showDetection(result) {
    const display = feedbackContentElement;
    display.innerHTML = ''; // Clear previous content

    if (result === 'up') {
        const handImg = document.createElement('img');
        handImg.src = 'img/hand.gif';
        handImg.alt = 'Bridge up';
        handImg.style.width = '100px';

        const text = document.createElement('div');
        text.textContent = 'Bridge is UP';
        text.className = 'detection-text';

        display.appendChild(handImg);
        display.appendChild(text);
    } else {
        const starsImg = document.createElement('img');
        starsImg.src = 'img/no-face.svg';
        starsImg.alt = 'Bridge down';
        starsImg.style.width = '100px';

        const text = document.createElement('div');
        text.textContent = 'Bridge is DOWN';
        text.className = 'detection-text';

        display.appendChild(starsImg);
        display.appendChild(text);
    }
}
