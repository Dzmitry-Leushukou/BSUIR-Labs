// JWT Authentication utilities for frontend

// Store JWT token in localStorage
function setAuthToken(token) {
    localStorage.setItem('access_token', token);
}

// Get JWT token from localStorage
function getAuthToken() {
    return localStorage.getItem('access_token');
}

// Remove JWT token from localStorage
function removeAuthToken() {
    localStorage.removeItem('access_token');
}

// Store user data in localStorage
function setUserData(userData) {
    localStorage.setItem('user_data', JSON.stringify(userData));
}

// Get user data from localStorage
function getUserData() {
    const data = localStorage.getItem('user_data');
    return data ? JSON.parse(data) : null;
}

// Remove user data from localStorage
function removeUserData() {
    localStorage.removeItem('user_data');
}

// Decode JWT token to get payload (without verification)
function decodeJWT(token) {
    try {
        const base64Url = token.split('.')[1];
        const base64 = base64Url.replace(/-/g, '+').replace(/_/g, '/');
        const jsonPayload = decodeURIComponent(atob(base64).split('').map(function(c) {
            return '%' + ('00' + c.charCodeAt(0).toString(16)).slice(-2);
        }).join(''));
        return JSON.parse(jsonPayload);
    } catch (e) {
        console.error('Error decoding JWT:', e);
        return null;
    }
}

// Check if token is expired
function isTokenExpired(token) {
    const payload = decodeJWT(token);
    if (!payload || !payload.exp) {
        return true;
    }
    // exp is in seconds, multiply by 1000 for milliseconds
    return Date.now() >= payload.exp * 1000;
}

// Get authentication headers for API requests
function getAuthHeaders() {
    const token = getAuthToken();
    if (!token) {
        return {};
    }

    // Check if token is expired
    if (isTokenExpired(token)) {
        console.warn('Token expired, please login again');
        removeAuthToken();
        removeUserData();
        return {};
    }

    return {
        'Authorization': `Bearer ${token}`
    };
}

// Make authenticated fetch request
async function authenticatedFetch(url, options = {}) {
    const headers = getAuthHeaders();

    // Merge headers
    options.headers = {
        ...options.headers,
        ...headers
    };

    // Set Content-Type only if not FormData (browser sets it automatically for FormData)
    if (!(options.body instanceof FormData) && !options.headers['Content-Type']) {
        options.headers['Content-Type'] = 'application/json';
    }

    try {
        const response = await fetch(url, options);

        // Handle 401 Unauthorized (token expired or invalid)
        if (response.status === 401) {
            const clonedResponse = response.clone();
            const errorData = await clonedResponse.json().catch(() => ({}));
            if (errorData.detail && (errorData.detail.includes('Токен') ||
                errorData.detail.includes('token') ||
                errorData.detail.includes('аутентифицирован'))) {
                // Token is invalid or expired
                removeAuthToken();
                removeUserData();
                // Don't reload - let the calling function handle the UI update
            }
        }

        // Handle 403 Forbidden (user blacklisted or banned)
        if (response.status === 403) {
            const clonedResponse = response.clone();
            const errorData = await clonedResponse.json().catch(() => ({}));
            if (errorData.detail && errorData.detail.includes('заблокирован')) {
                // User is blacklisted or banned
                removeAuthToken();
                removeUserData();
                alert(errorData.detail);
            }
        }

        return response;
    } catch (error) {
        console.error('Fetch error:', error);
        throw error;
    }
}

// Login function
async function login(email, password) {
    try {
        const response = await fetch('/users/login', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ email, password })
        });
        
        if (response.ok) {
            const data = await response.json();
            
            // Check if response contains access_token (JWT format)
            if (data.access_token) {
                // Save JWT token
                setAuthToken(data.access_token);
                
                // Remove token from user data object before saving
                const userData = { ...data };
                delete userData.access_token;
                delete userData.token_type;
                
                // Save user data
                setUserData(userData);
                return { success: true, user: userData };
            } else if (data.id) {
                // Legacy authentication (for backward compatibility)
                setUserData(data);
                return { success: true, user: data };
            }
            
            return { success: false, error: 'Invalid response format' };
        } else {
            const errorData = await response.json();
            return { success: false, error: errorData.detail || 'Login failed' };
        }
    } catch (error) {
        console.error('Login error:', error);
        return { success: false, error: error.message || 'Network error' };
    }
}

// Register function
async function register(email, password, name, surname) {
    try {
        const response = await fetch('/users/register', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ email, password, name, surname })
        });
        
        if (response.ok) {
            const data = await response.json();
            return { success: true, user: data };
        } else {
            const errorData = await response.json();
            return { success: false, error: errorData.detail || 'Registration failed' };
        }
    } catch (error) {
        console.error('Registration error:', error);
        return { success: false, error: error.message || 'Network error' };
    }
}

// Logout function
async function logout() {
    try {
        // Try to call logout endpoint to log the action
        await authenticatedFetch('/users/logout', {
            method: 'POST'
        });
    } catch (error) {
        console.error('Logout error:', error);
        // Continue with local cleanup even if API call fails
    } finally {
        removeAuthToken();
        removeUserData();
    }
}

// Check if user is authenticated
function isAuthenticated() {
    const token = getAuthToken();
    if (!token) {
        return false;
    }
    return !isTokenExpired(token);
}

// Get current user
function getCurrentUser() {
    if (!isAuthenticated()) {
        return null;
    }
    return getUserData();
}

// Clear all auth data and redirect to home
function clearAuthAndRedirect() {
    removeAuthToken();
    removeUserData();
    window.location.href = '/';
}
