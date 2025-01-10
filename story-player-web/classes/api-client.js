class ApiClient {
    constructor(baseURL) {
        this.baseURL = baseURL;
    }

    setBaseUrl(baseURL) {
        this.baseURL = baseURL;
    }

    async request(endpoint, method = 'GET', data = null, headers = {}) {
        const controller = new AbortController();
        const timeout = setTimeout(() => controller.abort(), 5000);
    
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


        const promise = fetch(`${this.baseURL}${endpoint}`, config).then(async (response) => {

        const data = await response.json();
            if (response.ok) {
            return data;
            } else {
            return Promise.reject(data);
            }
        });
        return promise.finally(() => clearTimeout(timeout));
        
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
