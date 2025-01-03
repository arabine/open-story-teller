// A lancer avec Bun !

async function getCommercialStoreDb() {
    console.log("Fetching db from commercial store");

    try {
        // Première requête pour obtenir le token
        const authResponse = await fetch('https://server-auth-prod.lunii.com/guest/create');
        if (!authResponse.ok) {
            throw new Error(`Failed to fetch auth token: ${authResponse.statusText}`);
        }

        const authData = await authResponse.json();
        const token = authData?.response?.token?.server;
        if (!token) {
            throw new Error('Token not found in response');
        }

        // Deuxième requête pour récupérer les données
        const dataResponse = await fetch('https://server-data-prod.lunii.com/v2/packs', {
            headers: {
                'X-AUTH-TOKEN': token,
            },
        });

        if (!dataResponse.ok) {
            throw new Error(`Failed to fetch packs: ${dataResponse.statusText}`);
        }

        const jsonData = await dataResponse.json(); // Récupérer les données en JSON

        // Enregistrer les données dans un fichier local
        const filePath = "./commercial_store_db.json";
        await Bun.write(filePath, JSON.stringify(jsonData, null, 2));
        console.log(`Data saved to ${filePath}`);

        return jsonData;
    } catch (error) {
        console.error(error);
        throw error;
    }
}

await getCommercialStoreDb();