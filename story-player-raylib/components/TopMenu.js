import { html } from 'htm/preact';
import eventBus from '../classes/event-bus.js';
import { useState } from 'preact/hooks';


function MessageComponent({ show, message }) {
    return html`
      <div>
        ${show
          ? html`<div class="themed">
                <div class="block fixed accent">
                ${message}
            </div>
        </div>`
          : html`<p />`}
      </div>
    `;

}


function TopMenu() {

    const [message, setMessage] = useState('Erreur ');
    const [error, setError] = useState(false);

    function handleClick() {
        eventBus.publish('show-modal', { type: 'parameters' });

        eventBus.subscribe('server-state-changed', function(data) {
            if (data.connected) {
                setMessage('Connecté au serveur');
            } else {
                setMessage('Serveur non trouvé');
            }
        });
    }

    return html`
    <style>

        .flexRow {
            display: flex;
            flex-direction: row;
            justify-content: space-between;
            margin-bottom: 12px;
            width: 100%;
        }

        .themed {
            --block-accent-color: #EA471A;

            <!-- background: #abcdef; -->
        }

        </style>
        <div class="flexRow">
            <${MessageComponent} show=${error} message=${message} />
            <div class="block accent" onClick="${handleClick}">
                Paramètres
            </div>
        </div>
    `;
}
export default TopMenu;
