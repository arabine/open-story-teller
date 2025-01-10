import { render } from 'preact';
import { html } from 'htm/preact';
import { useState } from 'preact/hooks';
import eventBus from '../classes/event-bus.js';

function ParametersDialog() {

    const [serverUrl, setServerUrl] = useState('127.0.0.1:8081');
      
    // Function to show the modal
    function showModal() {
        modal.style.display = 'block';
    }
      
    // Function to hide the modal
    function hideModal() {
        modal.style.display = 'none';
    }
    
    // Event listener for the close button
    function handleCloseClick() {
        hideModal();
    }
    
    // Event listener for the submit button
    function handleOkClick () {
        const urlInput = document.getElementById('url-input').value;
        console.log('URL entered:', urlInput);
        hideModal();
    }

    eventBus.subscribe('show-modal', function(data) {
        showModal();
    });  

    return html`

        <style>

        /* Modal Styles */
        .modal {
            display: none; /* Hidden by default */
            position: fixed;
            z-index: 1000;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            overflow: auto;
            background-color: rgba(0, 0, 0, 0.4); /* Black w/ opacity */
        }


        .close-button {
            color: #aaa;
            float: right;
            font-size: 28px;
            font-weight: bold;
        }

        .close-button:hover,
        .close-button:focus {
            color: black;
            text-decoration: none;
            cursor: pointer;
        }
        </style>
        <div id="modal" class="modal">
            <div class="block fixed">
                <span class="close-button" onClick="${handleCloseClick}">⨯</span>
                <label for="url-input">URL du serveur:</label>
                <div class="wrapper block">
                    <input type="text" id="url-input" name="url-input" placeholder="127.0.0.1:8080" value="${serverUrl}" />
                </div>
                <button id="submit-button" class="block accent" onClick="${handleOkClick}">Ok</button>
            </div>
        </div>
    `;

}

export default ParametersDialog;