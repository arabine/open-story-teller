import { html } from 'htm/preact';
import eventBus from '../classes/event-bus.js';
import { useState } from 'preact/hooks';


function StoriesList() {

    return html`
        <style>
        .card-container {
            max-height: 80vh;
            overflow-y: auto;
            padding: 20px;
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
        }

        .card-image {
            width: 120px;
            height: 90px;
            object-fit: cover;
            border-radius: 10px;
        }
        </style>
        <div>
            <div class="card-container">
                <div class="block card">
                    <div class="card-content">
                        <h2 class="card-title">Titre de la Carte 1</h2>
                        <p class="card-description">Description de la carte 1. Voici une brève description pour cette carte.</p>
                    </div>
                    <img src="https://placehold.co/100x75" alt="Image de la carte 1" class="card-image" />
                </div>
            </div>
        </div>

    `;
}
export default StoriesList;
