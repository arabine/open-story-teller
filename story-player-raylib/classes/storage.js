class Storage {
    constructor(prefix = '') {
        this.prefix = prefix;
    }

    setItem(key, value) {
        try {
            const data = JSON.stringify(value);
            localStorage.setItem(this.prefix + key, data);
        } catch (error) {
            console.error('Error saving to localStorage', error);
        }
    }

    getItem(key) {
        try {
            const data = localStorage.getItem(this.prefix + key);
            return data ? JSON.parse(data) : null;
        } catch (error) {
            console.error('Error reading from localStorage', error);
            return null;
        }
    }

    removeItem(key) {
        try {
            localStorage.removeItem(this.prefix + key);
        } catch (error) {
            console.error('Error removing from localStorage', error);
        }
    }

    clear() {
        try {
            const keys = Object.keys(localStorage);
            keys.forEach(key => {
                if (key.startsWith(this.prefix)) {
                    localStorage.removeItem(key);
                }
            });
        } catch (error) {
            console.error('Error clearing localStorage', error);
        }
    }
}

// Exemple d'utilisation
export default new Storage('ost_player_v1_');