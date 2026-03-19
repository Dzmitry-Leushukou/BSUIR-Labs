// Analytics dashboard JavaScript
// Гарантируем, что глобальные функции переводов всегда определены
window.translateActionType = window.translateActionType || function(x) { return x; };
window.translateDescription = window.translateDescription || function(x) { return x; };
let activityChart = null;
let crudChart = null;
let trendsChart = null;
let topUsersChart = null;

// Ensure auth.js is loaded before this script
// All auth/token logic now uses functions from auth.js

// Initialize date inputs
document.addEventListener('DOMContentLoaded', () => {
    // Set default dates (last 7 days)
    const endDate = new Date();
    const startDate = new Date();
    startDate.setDate(startDate.getDate() - 7);
    
    document.getElementById('end-date').value = endDate.toISOString().split('T')[0];
    document.getElementById('start-date').value = startDate.toISOString().split('T')[0];
    
    // Load analytics on page load
    loadAnalytics();
});


// Use getAuthHeaders from auth.js


// Use authenticatedFetch from auth.js

// Load all analytics data
async function loadAnalytics() {
    const startDate = document.getElementById('start-date').value;
    const endDate = document.getElementById('end-date').value;
    const period = document.getElementById('period-select').value;
    const timePeriod = document.getElementById('time-period').value;
    
    const params = new URLSearchParams();
    if (startDate) params.append('start_date', startDate + 'T00:00:00Z');
    if (endDate) params.append('end_date', endDate + 'T23:59:59Z');
    
    try {
        // Load all reports in parallel
        await Promise.all([
            loadUserActivity(period, startDate, endDate),
            loadOperationsDistribution(startDate, endDate),
            loadTimeSeries(timePeriod, startDate, endDate),
            loadTopUsers(startDate, endDate),
            loadAnomalies(startDate, endDate)
        ]);
        
        updateSummaryStats();
    } catch (error) {
        console.error('Error loading analytics:', error);
        showError('Ошибка при загрузке аналитики: ' + error.message);
    }
}

// Show error message
function showError(message) {
    const container = document.getElementById('error-container');
    container.innerHTML = `<div class="error-message">${message}</div>`;
    setTimeout(() => container.innerHTML = '', 5000);
}

// Load user activity stats
async function loadUserActivity(period, startDate, endDate) {
    try {
        const params = new URLSearchParams(`period=${period}`);
        if (startDate) params.append('start_date', startDate + 'T00:00:00Z');
        if (endDate) params.append('end_date', endDate + 'T23:59:59Z');
        
        const response = await authenticatedFetch(`/analytics/user-activity?${params}`);
        const data = await response.json();
        
        renderActivityChart(data);
    } catch (error) {
        console.error('Error loading user activity:', error);
    }
}

// Render activity chart
function renderActivityChart(data) {
    const ctx = document.getElementById('activityChart').getContext('2d');
    
    if (activityChart) {
        activityChart.destroy();
    }
    
    const labels = data.map(item => item.period);
    const totalActions = data.map(item => item.total_actions);
    const uniqueUsers = data.map(item => item.unique_users_count);
    
    activityChart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels: labels,
            datasets: [
                {
                    label: 'Всего действий',
                    data: totalActions,
                    backgroundColor: 'rgba(54, 162, 235, 0.8)',
                    borderColor: 'rgba(54, 162, 235, 1)',
                    borderWidth: 1
                },
                {
                    label: 'Уникальные пользователи',
                    data: uniqueUsers,
                    backgroundColor: 'rgba(255, 99, 132, 0.8)',
                    borderColor: 'rgba(255, 99, 132, 1)',
                    borderWidth: 1
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    beginAtZero: true
                }
            }
        }
    });
}

// Load operations distribution
async function loadOperationsDistribution(startDate, endDate) {
    try {
        const params = new URLSearchParams();
        if (startDate) params.append('start_date', startDate + 'T00:00:00Z');
        if (endDate) params.append('end_date', endDate + 'T23:59:59Z');
        
        const response = await authenticatedFetch(`/analytics/operations-distribution?${params}`);
        const data = await response.json();
        
        renderCrudChart(data);
        document.getElementById('total-operations').textContent = data.total_operations;
    } catch (error) {
        console.error('Error loading operations distribution:', error);
    }
}

// Render CRUD chart
function renderCrudChart(data) {
    const ctx = document.getElementById('crudChart').getContext('2d');
    
    if (crudChart) {
        crudChart.destroy();
    }
    
    const crud = data.crud_distribution;
    
    crudChart = new Chart(ctx, {
        type: 'doughnut',
        data: {
            labels: ['Create', 'Read', 'Update', 'Delete'],
            datasets: [{
                data: [
                    crud.Create.count,
                    crud.Read.count,
                    crud.Update.count,
                    crud.Delete.count
                ],
                backgroundColor: [
                    'rgba(75, 192, 192, 0.8)',
                    'rgba(54, 162, 235, 0.8)',
                    'rgba(255, 206, 86, 0.8)',
                    'rgba(255, 99, 132, 0.8)'
                ],
                borderColor: [
                    'rgba(75, 192, 192, 1)',
                    'rgba(54, 162, 235, 1)',
                    'rgba(255, 206, 86, 1)',
                    'rgba(255, 99, 132, 1)'
                ],
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    position: 'bottom'
                }
            }
        }
    });
}

// Load time series trends
async function loadTimeSeries(period, startDate, endDate) {
    try {
        const params = new URLSearchParams(`period=${period}`);
        if (startDate) params.append('start_date', startDate + 'T00:00:00Z');
        if (endDate) params.append('end_date', endDate + 'T23:59:59Z');
        
        const response = await authenticatedFetch(`/analytics/time-series?${params}`);
        const data = await response.json();
        
        renderTrendsChart(data, period);
        document.getElementById('avg-actions').textContent = data.avg_actions.toFixed(1);
    } catch (error) {
        console.error('Error loading time series:', error);
    }
}

// Render trends chart
function renderTrendsChart(data, period) {
    const ctx = document.getElementById('trendsChart').getContext('2d');
    
    if (trendsChart) {
        trendsChart.destroy();
    }
    
    const labels = data.trend_data.map(item => item.period);
    const counts = data.trend_data.map(item => item.count);
    
    trendsChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: 'Количество действий',
                data: counts,
                borderColor: 'rgba(75, 192, 192, 1)',
                backgroundColor: 'rgba(75, 192, 192, 0.2)',
                tension: 0.4,
                fill: true
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    beginAtZero: true
                }
            }
        }
    });
}

// Load top users
async function loadTopUsers(startDate, endDate) {
    try {
        const params = new URLSearchParams('limit=10');
        if (startDate) params.append('start_date', startDate + 'T00:00:00Z');
        if (endDate) params.append('end_date', endDate + 'T23:59:59Z');
        
        const response = await authenticatedFetch(`/analytics/top-users?${params}`);
        const data = await response.json();
        
        renderTopUsersChart(data);
        renderTopUsersTable(data);
        document.getElementById('active-users').textContent = data.length;
    } catch (error) {
        console.error('Error loading top users:', error);
    }
}

// Render top users chart
function renderTopUsersChart(data) {
    const ctx = document.getElementById('topUsersChart').getContext('2d');
    
    if (topUsersChart) {
        topUsersChart.destroy();
    }
    
    const labels = data.map(u => u.email || `User ${u.user_id}`);
    const actions = data.map(u => u.total_actions);
    
    topUsersChart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels: labels,
            datasets: [{
                label: 'Действий',
                data: actions,
                backgroundColor: 'rgba(153, 102, 255, 0.8)',
                borderColor: 'rgba(153, 102, 255, 1)',
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            indexAxis: 'y',
            scales: {
                x: {
                    beginAtZero: true
                }
            }
        }
    });
}

// Render top users table
function renderTopUsersTable(data) {
    const tbody = document.getElementById('top-users-body');
    tbody.innerHTML = '';
    
    // Используем глобальную функцию translateActionType из admin_action_logs_translations.js
    data.forEach((user, index) => {
        const row = document.createElement('tr');
        const actionsStr = Object.entries(user.actions_by_type || {})
            .map(([type, count]) => {
                const translated = typeof translateActionType === 'function' ? translateActionType(type) : type;
                return `${translated}: ${count}`;
            })
            .join('<br>');
        
        row.innerHTML = `
            <td>${index + 1}</td>
            <td>${user.email || `User ${user.user_id}`}</td>
            <td><strong>${user.total_actions}</strong></td>
            <td>${new Date(user.first_action).toLocaleString('ru-RU')}</td>
            <td>${new Date(user.last_action).toLocaleString('ru-RU')}</td>
            <td style="font-size: 12px; color: #666; line-height: 1.4; word-break: break-word;">${actionsStr}</td>
        `;
        tbody.appendChild(row);
    });
}

// Load anomalies
async function loadAnomalies(startDate, endDate) {
    try {
        const params = new URLSearchParams();
        if (startDate) params.append('start_date', startDate + 'T00:00:00Z');
        if (endDate) params.append('end_date', endDate + 'T23:59:59Z');
        
        const response = await authenticatedFetch(`/analytics/anomalies?${params}`);
        const data = await response.json();
        
        renderAnomaliesTable(data.anomalies);
        document.getElementById('anomalies-count').textContent = data.anomalies_detected || 0;
    } catch (error) {
        console.error('Error loading anomalies:', error);
    }
}

// Render anomalies table
function renderAnomaliesTable(anomalies) {
    const tbody = document.getElementById('anomalies-body');
    tbody.innerHTML = '';
    
    if (!anomalies || anomalies.length === 0) {
        tbody.innerHTML = '<tr><td colspan="5" style="text-align: center; color: #999;">Аномалий не обнаружено ✅</td></tr>';
        return;
    }
    
    anomalies.forEach(anomaly => {
        const row = document.createElement('tr');
        
        const badgeHtml = (anomaly.anomaly_types || []).map(type => {
            const className = type === 'high_activity' ? 'anomaly-high' : 'anomaly-diverse';
            const text = type === 'high_activity' ? 'Высокая активность' : 
                         type === 'low_activity' ? 'Низкая активность' : 'Разнообразные действия';
            return `<span class="anomaly-badge ${className}">${text}</span>`;
        }).join('');
        
        row.innerHTML = `
            <td>${anomaly.email || `User ${anomaly.user_id}`}</td>
            <td><strong>${anomaly.action_count}</strong></td>
            <td>${anomaly.unique_action_types_count}</td>
            <td>${anomaly.deviation > 0 ? '+' : ''}${anomaly.deviation}σ</td>
            <td>${badgeHtml}</td>
        `;
        tbody.appendChild(row);
    });
}

// Update summary statistics
function updateSummaryStats() {
    // This can be enhanced to calculate changes from previous periods
    document.getElementById('operations-change').textContent = 'за выбранный период';
    document.getElementById('users-change').textContent = 'активных';
    document.getElementById('avg-change').textContent = 'в среднем';
    document.getElementById('anomalies-change').textContent = 'требуют внимания';
}

// Reset filters
function resetFilters() {
    const endDate = new Date();
    const startDate = new Date();
    startDate.setDate(startDate.getDate() - 7);
    
    document.getElementById('end-date').value = endDate.toISOString().split('T')[0];
    document.getElementById('start-date').value = startDate.toISOString().split('T')[0];
    document.getElementById('period-select').value = 'day';
    document.getElementById('time-period').value = 'hour';
    
    loadAnalytics();
}

// Export all reports
async function exportAllReports() {
    const period = document.getElementById('period-select').value;
    const reports = [
        { url: `/analytics/export/user-activity/json?period=${period}`, name: `user_activity_${period}.json` },
        { url: `/analytics/export/top-users/json`, name: 'top_users.json' },
        { url: `/analytics/export/operations-distribution/json`, name: 'operations_distribution.json' },
        { url: `/analytics/export/anomalies/json`, name: 'anomalies.json' }
    ];
    reports.forEach(report => {
        // Для экспорта с авторизацией используем fetch с getAuthHeaders
        fetch(report.url, { headers: getAuthHeaders() })
            .then(response => response.blob())
            .then(blob => {
                const link = document.createElement('a');
                link.href = URL.createObjectURL(blob);
                link.download = report.name;
                link.style.display = 'none';
                document.body.appendChild(link);
                link.click();
                document.body.removeChild(link);
            });
    });
}
