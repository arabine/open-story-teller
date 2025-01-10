import { html } from 'htm/preact';
import eventBus from '../classes/event-bus.js';
import { useState } from 'preact/hooks';


function StoryPlayer() {

    return html`
        <style>

        </style>
        <div style="width: 400px; height: 500px;">
            <div class="block fixed" style="width: 320px; height: 240px;">
                
            </div>
            <div style="display: flex; ">
                <div class="block accent btn">⇦</div>
                <div class="block accent btn">✓</div>
                <div class="block accent btn">⇨</div>
                <div class="block accent btn">⏎</div>
                
            </div>
        </div>
    `;
}
export default StoryPlayer;
