import apiClient  from './classes/api-client.js'
import eventBus from './classes/event-bus.js';
import storage from './classes/storage.js';


import { render } from 'preact';
import { createContext } from "preact";
import { html } from 'htm/preact';
import TopMenu  from './components/TopMenu.js'
import ParametersDialog from './components/ParametersDialog.js';
import StoryPlayer from './components/StoryPlayer.js';
import StoriesList from './components/StoriesList.js';

export const AppContext = createContext("appContext");

export function App() {

    const DefaultContext = {
        stories: []
    };

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
        console.log('Server is down');
        eventBus.publish('server-state-changed', {connected: false});
    });


    return html`
    <AppContext.Provider value="${DefaultContext}">
        <${TopMenu} />
        <${ParametersDialog} />

        <div style="display: flex; flex-direction:row;">
            <${StoryPlayer} />
            <${StoriesList} />
        </div>
    </AppContext.Provider>
    `;
}

render(html`<${App} />`, document.getElementById('app'));
