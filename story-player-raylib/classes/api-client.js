class ApiClient {
    constructor(baseURL) {
        this.baseURL = baseURL;
    }

    setBaseUrl(baseURL) {
        this.baseURL = baseURL;
    }

    async request(endpoint, method = 'GET', data = null, headers = {}) {
        const config = {
            method,
            headers: {
                'Content-Type': 'application/json',
                ...headers
            }
        };

        if (data) {
            config.body = JSON.stringify(data);
        }

        try {
            const response = await fetch(`${this.baseURL}${endpoint}`, config);
            if (!response.ok) {
                const errorData = await response.json();
                throw new Error(errorData.message || 'Something went wrong');
            }
            return await response.json();
        } catch (error) {
            console.error('API request error:', error);
            throw error;
        }
    }

    get(endpoint, headers = {}) {
        return this.request(endpoint, 'GET', null, headers);
    }

    post(endpoint, data, headers = {}) {
        return this.request(endpoint, 'POST', data, headers);
    }

    put(endpoint, data, headers = {}) {
        return this.request(endpoint, 'PUT', data, headers);
    }

    delete(endpoint, headers = {}) {
        return this.request(endpoint, 'DELETE', null, headers);
    }
}
// Export de l'instance ApiClient pour l'importer facilement
const apiClient = new ApiClient('127.0.0.1:8081');
export default apiClient;
