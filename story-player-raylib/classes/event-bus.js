class EventBus {
    constructor() {
        this.events = {};
        this.id = Math.floor(Math.random() * 10000);
    }

    subscribe(event, listener) {
        if (!this.events[event]) {
            this.events[event] = [];
        }
        this.events[event].push(listener);
    }

    unsubscribe(event, listener) {
        if (this.events[event]) {
            this.events[event] = this.events[event].filter(l => l !== listener);
        }
    }

    publish(event, data) {
        if (this.events[event]) {
            this.events[event].forEach(listener => listener(data));
        }
    }
}

export default new EventBus();
