/**
 * ============================================================================
 * BANK MANAGEMENT SYSTEM - ADMIN CUSTOMERS CONTROLLER
 * ============================================================================
 */

let allCustomersCache = [];

document.addEventListener('DOMContentLoaded', () => {
  initAppShell('customers');
  const user = AuthStore.getUser();
  if (!user || user.role !== 'ADMIN') {
    window.location.href = '/dashboard';
    return;
  }

  loadCustomers();

  // Search filter
  const searchInput = document.getElementById('customer-search');
  if (searchInput) {
    searchInput.addEventListener('input', (e) => {
      const q = e.target.value.toLowerCase().trim();
      const filtered = allCustomersCache.filter(c => 
        c.full_name.toLowerCase().includes(q) ||
        c.email.toLowerCase().includes(q) ||
        (c.phone && c.phone.toLowerCase().includes(q)) ||
        (c.address && c.address.toLowerCase().includes(q))
      );
      renderCustomersTable(filtered);
    });
  }

  // Add Customer Form Submit
  const addForm = document.getElementById('form-add-customer');
  if (addForm) {
    addForm.addEventListener('submit', async (e) => {
      e.preventDefault();
      const fullName = document.getElementById('add-cust-name').value.trim();
      const email = document.getElementById('add-cust-email').value.trim();
      const phone = document.getElementById('add-cust-phone').value.trim();
      const address = document.getElementById('add-cust-address').value.trim();
      const dob = document.getElementById('add-cust-dob').value;
      const password = document.getElementById('add-cust-password').value || 'Customer123!';
      const submitBtn = addForm.querySelector('button[type="submit"]');

      try {
        submitBtn.disabled = true;
        submitBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Saving...';

        await apiRequest('/api/customers', {
          method: 'POST',
          body: JSON.stringify({
            full_name: fullName,
            email,
            phone,
            address,
            date_of_birth: dob,
            password
          })
        });

        showToast('Customer Created', `Customer ${fullName} successfully added.`, 'success');
        closeModal('modal-add-customer');
        addForm.reset();
        loadCustomers();
      } catch (err) {
        showToast('Error', err.message, 'danger');
      } finally {
        submitBtn.disabled = false;
        submitBtn.innerHTML = '<span>Save Customer</span>';
      }
    });
  }

  // Edit Customer Form Submit
  const editForm = document.getElementById('form-edit-customer');
  if (editForm) {
    editForm.addEventListener('submit', async (e) => {
      e.preventDefault();
      const customerId = document.getElementById('edit-cust-id').value;
      const fullName = document.getElementById('edit-cust-name').value.trim();
      const phone = document.getElementById('edit-cust-phone').value.trim();
      const address = document.getElementById('edit-cust-address').value.trim();
      const dob = document.getElementById('edit-cust-dob').value;
      const submitBtn = editForm.querySelector('button[type="submit"]');

      try {
        submitBtn.disabled = true;
        submitBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Updating...';

        await apiRequest(`/api/customers/${customerId}`, {
          method: 'PUT',
          body: JSON.stringify({
            full_name: fullName,
            phone,
            address,
            date_of_birth: dob
          })
        });

        showToast('Customer Updated', 'Profile updated successfully.', 'success');
        closeModal('modal-edit-customer');
        loadCustomers();
      } catch (err) {
        showToast('Error', err.message, 'danger');
      } finally {
        submitBtn.disabled = false;
        submitBtn.innerHTML = '<span>Update Customer</span>';
      }
    });
  }
});

async function loadCustomers() {
  const tbody = document.getElementById('customers-tbody');
  if (!tbody) return;

  try {
    tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding: 20px;"><i class="fas fa-spinner fa-spin"></i> Loading customer directory...</td></tr>`;
    const customers = await apiRequest('/api/customers');
    allCustomersCache = customers;
    renderCustomersTable(customers);
  } catch (err) {
    showToast('Error', err.message, 'danger');
    tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; color: var(--danger); padding: 20px;">Failed to load customers.</td></tr>`;
  }
}

function renderCustomersTable(customers) {
  const tbody = document.getElementById('customers-tbody');
  if (!tbody) return;

  if (!customers || customers.length === 0) {
    tbody.innerHTML = `
      <tr>
        <td colspan="7" style="text-align: center; padding: 32px; color: var(--text-muted);">
          No customers found.
        </td>
      </tr>
    `;
    return;
  }

  tbody.innerHTML = customers.map(c => {
    let statusBadge = c.status === 'ACTIVE' ? 'badge-success' : 'badge-danger';
    return `
      <tr>
        <td><strong>#CUST${c.user_id}</strong></td>
        <td>
          <div style="font-weight: 600;">${c.full_name}</div>
          <div style="font-size: 0.78rem; color: var(--text-muted);">${c.email}</div>
        </td>
        <td>${c.phone || '-'}</td>
        <td><span class="badge badge-info">${c.account_count || 0} Account(s)</span></td>
        <td style="font-weight: 700; color: var(--primary);">${formatRupees(c.total_balance_rupees || 0)}</td>
        <td><span class="badge ${statusBadge}">${c.status}</span></td>
        <td>
          <div style="display: flex; gap: 6px;">
            <button class="btn btn-outline btn-sm" onclick="openEditModal(${c.user_id})" title="Edit Customer Details">
              <i class="fas fa-edit"></i> Edit
            </button>
            <button class="btn btn-outline btn-sm" onclick="toggleCustomerStatus(${c.user_id}, '${c.status}')" title="Change Status">
              <i class="fas ${c.status === 'ACTIVE' ? 'fa-ban' : 'fa-check'}"></i> 
              ${c.status === 'ACTIVE' ? 'Deactivate' : 'Activate'}
            </button>
          </div>
        </td>
      </tr>
    `;
  }).join('');
}

window.openEditModal = function(customerId) {
  const cust = allCustomersCache.find(c => c.user_id === customerId);
  if (!cust) return;

  document.getElementById('edit-cust-id').value = cust.user_id;
  document.getElementById('edit-cust-name').value = cust.full_name;
  document.getElementById('edit-cust-email').value = cust.email;
  document.getElementById('edit-cust-phone').value = cust.phone || '';
  document.getElementById('edit-cust-address').value = cust.address || '';
  document.getElementById('edit-cust-dob').value = cust.date_of_birth || '';

  openModal('modal-edit-customer');
};

window.toggleCustomerStatus = async function(customerId, currentStatus) {
  const newStatus = currentStatus === 'ACTIVE' ? 'INACTIVE' : 'ACTIVE';
  try {
    await apiRequest(`/api/customers/${customerId}/status`, {
      method: 'PATCH',
      body: JSON.stringify({ status: newStatus })
    });
    showToast('Status Updated', `Customer status changed to ${newStatus}.`, 'success');
    loadCustomers();
  } catch (err) {
    showToast('Update Failed', err.message, 'danger');
  }
};
