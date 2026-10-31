/**
 * ============================================================================
 * BANK MANAGEMENT SYSTEM - ACCOUNTS CONTROLLER
 * ============================================================================
 */

let allAccountsCache = [];

document.addEventListener('DOMContentLoaded', async () => {
  initAppShell('accounts');
  const user = AuthStore.getUser();
  if (!user) return;

  loadAccounts();

  // Create Account Modal Triggers
  const openModalBtn = document.getElementById('btn-open-create-account');
  if (openModalBtn) {
    openModalBtn.addEventListener('click', async () => {
      // If admin, populate customer dropdown
      if (user.role === 'ADMIN') {
        const custSelectGroup = document.getElementById('create-acc-customer-group');
        const custSelect = document.getElementById('new-acc-customer-id');
        if (custSelectGroup && custSelect) {
          custSelectGroup.style.display = 'block';
          try {
            const customers = await apiRequest('/api/customers');
            custSelect.innerHTML = customers.map(c => `
              <option value="${c.user_id}">${c.full_name} (${c.email})</option>
            `).join('');
          } catch (e) {
            console.error(e);
          }
        }
      }
      openModal('modal-create-account');
    });
  }

  // Create Account Form Submission
  const createForm = document.getElementById('form-create-account');
  if (createForm) {
    createForm.addEventListener('submit', async (e) => {
      e.preventDefault();
      const accountType = document.getElementById('new-acc-type').value;
      const initialDeposit = parseFloat(document.getElementById('new-acc-deposit').value) || 0;
      const submitBtn = createForm.querySelector('button[type="submit"]');

      const payload = {
        account_type: accountType,
        initial_deposit: initialDeposit
      };

      if (user.role === 'ADMIN') {
        const custSelect = document.getElementById('new-acc-customer-id');
        if (custSelect) {
          payload.user_id = parseInt(custSelect.value, 10);
        }
      }

      try {
        submitBtn.disabled = true;
        submitBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Creating...';

        const res = await apiRequest('/api/accounts', {
          method: 'POST',
          body: JSON.stringify(payload)
        });

        showToast('Account Created', `Account ${res.account_number} created successfully!`, 'success');
        closeModal('modal-create-account');
        createForm.reset();
        loadAccounts();
      } catch (err) {
        showToast('Creation Failed', err.message, 'danger');
      } finally {
        submitBtn.disabled = false;
        submitBtn.innerHTML = '<span>Create Account</span>';
      }
    });
  }

  // Search filter
  const searchInput = document.getElementById('account-search');
  if (searchInput) {
    searchInput.addEventListener('input', (e) => {
      const q = e.target.value.toLowerCase().trim();
      const filtered = allAccountsCache.filter(a => 
        a.account_number.toLowerCase().includes(q) ||
        (a.customer_name && a.customer_name.toLowerCase().includes(q)) ||
        a.account_type.toLowerCase().includes(q)
      );
      renderAccountsTable(filtered);
    });
  }
});

async function loadAccounts() {
  const tbody = document.getElementById('accounts-tbody');
  if (!tbody) return;

  try {
    tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding: 20px;"><i class="fas fa-spinner fa-spin"></i> Loading accounts...</td></tr>`;
    const accounts = await apiRequest('/api/accounts');
    allAccountsCache = accounts;
    renderAccountsTable(accounts);
  } catch (err) {
    showToast('Error', err.message, 'danger');
    tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; color: var(--danger); padding: 20px;">Failed to load accounts.</td></tr>`;
  }
}

function renderAccountsTable(accounts) {
  const tbody = document.getElementById('accounts-tbody');
  if (!tbody) return;

  const user = AuthStore.getUser();
  const isAdmin = user && user.role === 'ADMIN';

  if (!accounts || accounts.length === 0) {
    tbody.innerHTML = `
      <tr>
        <td colspan="7" style="text-align: center; padding: 32px; color: var(--text-muted);">
          No accounts found.
        </td>
      </tr>
    `;
    return;
  }

  tbody.innerHTML = accounts.map(a => {
    let statusBadge = 'badge-success';
    if (a.status === 'FROZEN') statusBadge = 'badge-warning';
    if (a.status === 'CLOSED') statusBadge = 'badge-danger';

    let actionsHtml = `
      <a href="/deposit" class="btn btn-outline btn-sm" title="Deposit"><i class="fas fa-circle-arrow-down"></i></a>
      <a href="/withdraw" class="btn btn-outline btn-sm" title="Withdraw"><i class="fas fa-circle-arrow-up"></i></a>
      <a href="/transfer" class="btn btn-outline btn-sm" title="Transfer"><i class="fas fa-paper-plane"></i></a>
    `;

    if (isAdmin) {
      actionsHtml = `
        <button class="btn btn-outline btn-sm" onclick="toggleAccountStatus(${a.account_id}, '${a.status}')">
          <i class="fas ${a.status === 'ACTIVE' ? 'fa-snowflake' : 'fa-check'}"></i> 
          ${a.status === 'ACTIVE' ? 'Freeze' : 'Activate'}
        </button>
      `;
    }

    return `
      <tr>
        <td><strong>${isAdmin ? a.account_number : (a.masked_account_number || a.account_number)}</strong></td>
        <td><strong>${a.customer_name || '-'}</strong></td>
        <td><span class="badge badge-info">${a.account_type}</span></td>
        <td style="font-weight: 700; color: var(--primary);">${formatRupees(a.balance_rupees)}</td>
        <td><span class="badge ${statusBadge}">${a.status}</span></td>
        <td>${formatDate(a.created_at)}</td>
        <td>
          <div style="display: flex; gap: 6px;">
            ${actionsHtml}
          </div>
        </td>
      </tr>
    `;
  }).join('');
}

// Admin toggle account status
window.toggleAccountStatus = async function(accountId, currentStatus) {
  const newStatus = currentStatus === 'ACTIVE' ? 'FROZEN' : 'ACTIVE';
  try {
    await apiRequest(`/api/accounts/${accountId}/status`, {
      method: 'PATCH',
      body: JSON.stringify({ status: newStatus })
    });
    showToast('Status Updated', `Account status set to ${newStatus}.`, 'success');
    loadAccounts();
  } catch (err) {
    showToast('Update Failed', err.message, 'danger');
  }
};
