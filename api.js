/**
 * ============================================================================
 * BANK MANAGEMENT SYSTEM - API UTILITIES & APPLICATION SHELL
 * Handles Fetch API requests, token lifecycle, formatting, and UI helpers.
 * ============================================================================
 */

const API_BASE = window.location.origin;

// Session Management
const AuthStore = {
  getToken: () => localStorage.getItem('bank_token'),
  setToken: (token) => localStorage.setItem('bank_token', token),
  clearToken: () => {
    localStorage.removeItem('bank_token');
    localStorage.removeItem('bank_user');
  },
  getUser: () => {
    try {
      const u = localStorage.getItem('bank_user');
      return u ? JSON.parse(u) : null;
    } catch (e) {
      return null;
    }
  },
  setUser: (user) => localStorage.setItem('bank_user', JSON.stringify(user))
};

// Generic API Request Client
async function apiRequest(endpoint, options = {}) {
  const url = endpoint.startsWith('http') ? endpoint : `${API_BASE}${endpoint}`;
  const token = AuthStore.getToken();

  const headers = {
    'Content-Type': 'application/json',
    'Accept': 'application/json',
    ...(options.headers || {})
  };

  if (token) {
    headers['Authorization'] = `Bearer ${token}`;
  }

  const config = {
    ...options,
    headers
  };

  try {
    const response = await fetch(url, config);
    const data = await response.json().catch(() => ({}));

    if (!response.ok) {
      // If unauthorized, redirect to login unless on public pages
      if (response.status === 401) {
        const publicPages = ['/login', '/register', '/'];
        const currentPath = window.location.pathname;
        if (!publicPages.includes(currentPath) && !currentPath.endsWith('login.html') && !currentPath.endsWith('register.html') && !currentPath.endsWith('index.html')) {
          AuthStore.clearToken();
          window.location.href = '/login';
        }
      }
      const errorMsg = data.error || data.message || `Request failed with status ${response.status}`;
      throw new Error(errorMsg);
    }

    return data;
  } catch (error) {
    console.error(`[API Error] ${endpoint}:`, error.message);
    throw error;
  }
}

// Currency Formatter in Indian Rupees (INR)
function formatRupees(amount) {
  const num = Number(amount) || 0;
  return new Intl.NumberFormat('en-IN', {
    style: 'currency',
    currency: 'INR',
    maximumFractionDigits: 2
  }).format(num);
}

// Date Formatter
function formatDate(dateStr) {
  if (!dateStr) return '-';
  const d = new Date(dateStr);
  if (isNaN(d.getTime())) return dateStr;
  return d.toLocaleDateString('en-IN', {
    year: 'numeric',
    month: 'short',
    day: 'numeric',
    hour: '2-digit',
    minute: '2-digit'
  });
}

// Toast Notification Manager
function showToast(title, message, type = 'info') {
  let container = document.querySelector('.toast-container');
  if (!container) {
    container = document.createElement('div');
    container.className = 'toast-container';
    document.body.appendChild(container);
  }

  const toast = document.createElement('div');
  toast.className = `toast toast-${type}`;

  let iconClass = 'fa-info-circle';
  if (type === 'success') iconClass = 'fa-check-circle';
  if (type === 'danger') iconClass = 'fa-exclamation-circle';
  if (type === 'warning') iconClass = 'fa-triangle-exclamation';

  toast.innerHTML = `
    <i class="fas ${iconClass} toast-icon"></i>
    <div class="toast-content">
      <div class="toast-title">${title}</div>
      <div class="toast-message">${message}</div>
    </div>
  `;

  container.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transform = 'translateX(50px)';
    setTimeout(() => toast.remove(), 300);
  }, 4000);
}

// Modal Dialog Helpers
function openModal(modalId) {
  const modal = document.getElementById(modalId);
  if (modal) {
    modal.classList.add('show');
  }
}

function closeModal(modalId) {
  const modal = document.getElementById(modalId);
  if (modal) {
    modal.classList.remove('show');
  }
}

// Initialize Global Shell Elements (Sidebar, Topbar, User profile chip)
function initAppShell(activeMenuId = '') {
  const user = AuthStore.getUser();
  if (!user) {
    window.location.href = '/login';
    return;
  }

  // Populate Topbar User Info
  const userNameElem = document.getElementById('topbar-user-name');
  const userRoleElem = document.getElementById('topbar-user-role');
  const userAvatarElem = document.getElementById('topbar-user-avatar');
  const dateElem = document.getElementById('topbar-current-date');

  if (userNameElem) userNameElem.textContent = user.name || 'User';
  if (userRoleElem) userRoleElem.textContent = user.role || 'CUSTOMER';
  if (userAvatarElem) {
    const initials = (user.name || 'U').split(' ').map(n => n[0]).join('').substring(0, 2).toUpperCase();
    userAvatarElem.textContent = initials;
  }

  if (dateElem) {
    const today = new Date();
    dateElem.textContent = today.toLocaleDateString('en-IN', {
      weekday: 'short',
      month: 'short',
      day: 'numeric',
      year: 'numeric'
    });
  }

  // Sidebar Role-Based Menu Rendering
  const menuContainer = document.getElementById('sidebar-menu-items');
  if (menuContainer) {
    const isAdmin = user.role === 'ADMIN';

    const menuItems = [
      { id: 'dashboard', label: 'Dashboard', icon: 'fa-table-columns', url: '/dashboard' },
      { id: 'accounts', label: isAdmin ? 'Account Management' : 'My Accounts', icon: 'fa-wallet', url: '/accounts' },
      ...(isAdmin ? [
        { id: 'customers', label: 'Customers', icon: 'fa-users', url: '/customers' }
      ] : []),
      { id: 'transactions', label: 'Transactions', icon: 'fa-arrow-right-arrow-left', url: '/transactions' },
      ...(!isAdmin ? [
        { id: 'deposit', label: 'Deposit Money', icon: 'fa-circle-arrow-down', url: '/deposit' },
        { id: 'withdraw', label: 'Withdraw Money', icon: 'fa-circle-arrow-up', url: '/withdraw' },
        { id: 'transfer', label: 'Transfer Money', icon: 'fa-paper-plane', url: '/transfer' }
      ] : []),
      ...(isAdmin ? [
        { id: 'reports', label: 'Reports & Analytics', icon: 'fa-chart-pie', url: '/reports' }
      ] : []),
      { id: 'profile', label: 'Profile', icon: 'fa-user-circle', url: '/profile' }
    ];

    let html = '';
    html += '<div class="nav-section-title">Main Banking Menu</div>';
    menuItems.forEach(item => {
      const isActive = item.id === activeMenuId ? 'active' : '';
      html += `
        <a href="${item.url}" class="nav-item ${isActive}">
          <i class="fas ${item.icon}"></i>
          <span>${item.label}</span>
        </a>
      `;
    });

    menuContainer.innerHTML = html;
  }

  // Logout Handler
  const logoutBtn = document.getElementById('btn-logout');
  if (logoutBtn) {
    logoutBtn.addEventListener('click', async () => {
      try {
        await apiRequest('/api/auth/logout', { method: 'POST' });
      } catch (e) {
        // Ignore logout network error and clear local token
      }
      AuthStore.clearToken();
      window.location.href = '/login';
    });
  }

  // Mobile Menu Toggle
  const toggleBtn = document.getElementById('menu-toggle');
  const sidebar = document.querySelector('.app-sidebar');
  const backdrop = document.querySelector('.sidebar-backdrop');

  if (toggleBtn && sidebar && backdrop) {
    toggleBtn.addEventListener('click', () => {
      sidebar.classList.toggle('show-sidebar');
      backdrop.classList.toggle('show');
    });

    backdrop.addEventListener('click', () => {
      sidebar.classList.remove('show-sidebar');
      backdrop.classList.remove('show');
    });
  }
}
