import { jwtDecode } from 'jwt-decode';

export const isTokenExpired = (token) => {
    if (!token) return true;

    const decoded = jwtDecode(token);
    const currentTime = Date.now() / 1000;
    return decoded.exp < currentTime;
};

const refreshAccessToken = async () => {
    const refreshToken = localStorage.getItem('refresh_token');

    if (!refreshToken)
        throw new Error("No refresh token available");

    if (isTokenExpired(refreshToken))
        throw new Error("Refresh token expired");

    try {
        const response = await fetch('http://localhost:8000/refresh', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ refresh_token: refreshToken }),
        });

        if (!response.ok)
            throw new Error('Failed to refresh token');

        const data = await response.json();
        return data.access_token;
    } catch (error) {
        console.error('Error refreshing token:', error);
        throw error
    }
};

const authFetch = async (url, options = {}, accessToken, saveToken) => {
    if (!accessToken)
        throw new Error("No access token available");

    options.headers = {
        ...options.headers,
        'Authorization': `Bearer ${accessToken}`,
    };

    try {
        if (!isTokenExpired(accessToken)) {
            const response = await fetch(url, options);
            if (response.status !== 401) {
                if (!response.ok) {
                    throw new Error('Request failed with status ' + response.status);
                }
                return response;
            }
        }
        console.log('Access token expired, attempting to refresh...');
        const newAccessToken = await refreshAccessToken();
        saveToken(newAccessToken);
        options.headers['Authorization'] = `Bearer ${newAccessToken}`;
        return fetch(url, options);
    } catch (error) {
        console.error('Error in authFetch:', error);
        throw error;
    }
};

export default authFetch;