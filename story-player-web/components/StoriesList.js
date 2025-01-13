import { html } from 'htm/preact';

// Classes
import eventBus from '../classes/event-bus.js';
import apiClient from '../classes/api-client.js';

import { AppContext } from '../app-context.js';
import { useContext, useEffect } from 'preact/hooks'


function StoriesList() {

    const { state, setStories } = useContext(AppContext);


    useEffect(() => {
        console.log("Fetch stories");
        getStories();

        return () => {
            console.log("Component will unmount");
        }
    }, []);


    function getStories() {

        // try to connect to the server
        
        apiClient.get('/library/list')
        .then(data => {
            console.log('Server is up and running', data);
            eventBus.publish('server-state-changed', {connected: true});
            setStories(data);
        })
        .catch(error => {
            console.log('Api error: '  + error);
            eventBus.publish('server-state-changed', {connected: false});
        });
    }

    function handleClickOnStory() {
        console.log("Clicked")
    }

    return html`
        <style>
        .card-container {
            max-height: 80vh;
            overflow-y: auto;
            padding: 4px;
            width: 400px;
        }

        .card {
            display: flex;
            align-items: center;
        }

        .card-content {
            flex: 1;
            margin-right: 20px;
        }

        .card-title {
            font-size: 1.2em;
            margin: 0 0 10px 0;
            font-weight: bold;
        }

        .card-description {
            font-size: 0.9em;
            margin: 0;

            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            width: 220px;

        }

        .card-image {
            width: 120px;
            height: 90px;
            object-fit: cover;
            border-radius: 10px;
        }
        </style>
        <div style="max-height: 400px; overflow-y: auto; border: 1px solid #ccc; ">
            ${state.stories.map(story => html`
                <div class="card-container" onClick="${handleClickOnStory}">
                    <div class="block card">
                        <div class="card-content">
                            <h2 class="card-title">${story.title}</h2>
                            <p class="card-description">${story.description}</p>
                        </div>
                        <img src="https://placehold.co/100x75" alt="Image de la carte 1" class="card-image" />
                    </div>
                </div>
            `)}
        </div>

    `;
}
export default StoriesList;
