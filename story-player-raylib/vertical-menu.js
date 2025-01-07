class VerticalMenu extends HTMLElement {
    constructor() {
        super();

        // Create a wrapper nav element using Bulma
        this.innerHTML = `
            <nav>
            <ul>
                <li><strong>OpenStoryTeller</strong></li>
            </ul>
            <ul>
                <li><a href="#">Paramètres</a></li>
                <li><a href="#">À propos</a></li>
            </ul>
            </nav>
        `;
       

    }

    connectedCallback() {

    }
}

// Define the new element
customElements.define('vertical-menu', VerticalMenu);
