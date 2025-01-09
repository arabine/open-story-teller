import apiClient  from './classes/api-client.js'
import eventBus from './classes/event-bus.js';
import storage from './classes/storage.js';


import { render } from 'preact';
import { html } from 'htm/preact';
import TopMenu  from './components/TopMenu.js'
import ParametersDialog from './components/ParametersDialog.js';

export function App() {

    this.params = storage.getItem('server') || {
        serverUrl: '127.0.0.1',
        serverPort: 8081, 
    };

    storage.setItem('server', this.params);
    
    // try to connect to the server
    apiClient.setBaseUrl(`http://${this.params.serverUrl}:${this.params.serverPort}/api/v1`);
    apiClient.get('/library/list')
        .then(data => {
            console.log('Server is up and running', data);
            eventBus.publish('server-state-changed', {connected: true});
        })
        .catch(error => {
            console.error('Server is down', error);
            eventBus.publish('server-state-changed', {connected: false});
        });


    return html`
    <${TopMenu} />
    <${ParametersDialog} />
    `;
}

render(html`<${App} />`, document.getElementById('app'));
