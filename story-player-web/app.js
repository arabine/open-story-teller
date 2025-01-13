// Preact
import { useContext, useEffect } from 'preact/hooks';
import { render } from 'preact';
import { html } from 'htm/preact';

// Classes
import apiClient  from './classes/api-client.js'
import eventBus from './classes/event-bus.js';
import storage from './classes/storage.js';

// Components
import TopMenu  from './components/TopMenu.js'
import ParametersDialog from './components/ParametersDialog.js';
import StoryPlayer from './components/StoryPlayer.js';
import StoriesList from './components/StoriesList.js';

// Project
import { AppProvider, AppContext } from './app-context.js';

export function App() {
   // const { setStories } = useContext(AppContext);

    this.params = storage.getItem('server') || {
        serverUrl: '127.0.0.1',
        serverPort: 8081, 
    };

    // The useEffect hook with an empty dependency array simulates the componentDidMount lifecycle method. 
    // It runs the provided function after the component is first rendered. 
    // This is a good place to perform data fetching or initial setup.
    useEffect(() => {
        console.log("App ready");
        return () => {
            console.log("App unmount");
        }
    }, []);


    storage.setItem('server', this.params);
    apiClient.setBaseUrl(`http://${this.params.serverUrl}:${this.params.serverPort}/api/v1`);

    return html`
    <${AppProvider}>
        <${TopMenu} />
        <${ParametersDialog} />

        <div style="display: flex; flex-direction:row;">
            <${StoryPlayer} />
            <${StoriesList} />
        </div>
    </${AppProvider}>
    `;
}

render(html`<${App} />`, document.getElementById('app'));
