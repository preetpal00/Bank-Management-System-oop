/**
 * ============================================================================
 * BANK MANAGEMENT SYSTEM - TRANSACTIONS CONTROLLER
 * Filterable, searchable, and paginated transaction history with receipt modal.
 * ============================================================================
 */

let currentOffset = 0;
const PAGE_LIMIT = 15;
let currentFilters = {};

document.addEventListener('DOMContentLoaded', () => {
  initAppShell('transactions');
  loadTransactions();

  // Filter Form Submit
  const filterForm = document.getElementById('transaction-filter-form');
  if (filterForm) {
    filterForm.addEventListener('submit', (e) => {
      e.preventDefault();
      currentOffset = 0;
      applyFilters();
    });
  }

  // Reset Filters
  const resetBtn = document.getElementById('btn-reset-filters');
  if (resetBtn) {
    resetBtn.addEventListener('click', () => {
      if (filterForm) filterForm.reset();
      currentFilters = {};
      currentOffset = 0;
      loadTransactions();
    });
  }

  // Pagination Buttons
  const prevBtn = document.getElementById('btn-prev-page');
  const nextBtn = document.getElementById('btn-next-page');

  if (prevBtn) {
    prevBtn.addEventListener('click', () => {
      if (currentOffset >= PAGE_LIMIT) {
        currentOffset -= PAGE_LIMIT;
        loadTransactions();
      }
    });
  }

  if (nextBtn) {
    nextBtn.addEventListener('click', () => {
      currentOffset += PAGE_LIMIT;
      loadTransactions();
    });
  }
});

function applyFilters() {
  const type = document.getElementById('filter-type')?.value || '';
  const search = document.getElementById('filter-search')?.value.trim() || '';
  const startDate = document.getElementById('filter-start-date')?.value || '';
  const endDate = document.getElementById('filter-end-date')?.value || '';
  const minAmount = document.getElementById('filter-min-amount')?.value || '';
  const maxAmount = document.getElementById('filter-max-amount')?.value || '';

  currentFilters = {
    type,
    search,
    start_date: startDate,
    end_date: endDate,
    min_amount: minAmount,
    max_amount: maxAmount
  };

  loadTransactions();
}

async function loadTransactions() {
  const tbody = document.getElementById('transactions-tbody');
  if (!tbody) return;

  tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; padding: 20px;"><i class="fas fa-spinner fa-spin"></i> Loading transactions...</td></tr>`;

  try {
    const params = new URLSearchParams({
      limit: PAGE_LIMIT,
      offset: currentOffset,
      ...currentFilters
    });

    const data = await apiRequest(`/api/transactions?${params.toString()}`);
    renderTransactions(data.transactions);
    updatePaginationControls(data.total_count, currentOffset, PAGE_LIMIT);
  } catch (err) {
    showToast('Error', err.message, 'danger');
    tbody.innerHTML = `<tr><td colspan="7" style="text-align:center; color: var(--danger); padding: 20px;">Failed to load transactions.</td></tr>`;
  }
}

function renderTransactions(transactions) {
  const tbody = document.getElementById('transactions-tbody');
  if (!tbody) return;

  if (!transactions || transactions.length === 0) {
    tbody.innerHTML = `
      <tr>
        <td colspan="7" style="text-align: center; padding: 36px; color: var(--text-muted);">
          <i class="fas fa-receipt" style="font-size: 2rem; margin-bottom: 8px; display: block;"></i>
          No transactions match your criteria.
        </td>
      </tr>
    `;
    return;
  }

  tbody.innerHTML = transactions.map(t => {
    const isCredit = t.transaction_type === 'DEPOSIT' || t.transaction_type === 'TRANSFER_IN';
    const amountClass = isCredit ? 'amount-positive' : 'amount-negative';
    const sign = isCredit ? '+' : '-';

    let badgeClass = 'badge-info';
    if (t.transaction_type === 'DEPOSIT') badgeClass = 'badge-success';
    if (t.transaction_type === 'WITHDRAWAL') badgeClass = 'badge-danger';
    if (t.transaction_type.startsWith('TRANSFER')) badgeClass = 'badge-warning';

    return `
      <tr>
        <td><strong>${t.reference_number}</strong></td>
        <td><span class="badge ${badgeClass}">${t.transaction_type.replace('_', ' ')}</span></td>
        <td>${t.account_number || t.masked_account_number || '-'}</td>
        <td class="${amountClass}">${sign} ${formatRupees(t.amount_rupees)}</td>
        <td>${t.description || '-'}</td>
        <td>${formatDate(t.created_at)}</td>
        <td>
          <button class="btn btn-outline btn-sm" onclick="viewReceipt('${t.reference_number}')" title="View Digital Receipt">
            <i class="fas fa-eye"></i> Receipt
          </button>
        </td>
      </tr>
    `;
  }).join('');
}

function updatePaginationControls(totalCount, offset, limit) {
  const prevBtn = document.getElementById('btn-prev-page');
  const nextBtn = document.getElementById('btn-next-page');
  const infoText = document.getElementById('pagination-info');

  const startRecord = totalCount === 0 ? 0 : offset + 1;
  const endRecord = Math.min(offset + limit, totalCount);

  if (infoText) {
    infoText.textContent = `Showing ${startRecord}–${endRecord} of ${totalCount} transactions`;
  }

  if (prevBtn) prevBtn.disabled = offset === 0;
  if (nextBtn) nextBtn.disabled = endRecord >= totalCount;
}

// Digital Transaction Receipt Modal
window.viewReceipt = async function(ref) {
  try {
    const rows = await apiRequest(`/api/transactions/${ref}`);
    if (!rows || rows.length === 0) return;

    const t = rows[0];
    document.getElementById('receipt-ref').textContent = t.reference_number;
    document.getElementById('receipt-type').textContent = t.transaction_type.replace('_', ' ');
    document.getElementById('receipt-account').textContent = t.account_number || t.masked_account_number;
    document.getElementById('receipt-holder').textContent = t.account_holder || '-';
    document.getElementById('receipt-amount').textContent = formatRupees(t.amount_rupees);
    document.getElementById('receipt-desc').textContent = t.description || '-';
    document.getElementById('receipt-date').textContent = formatDate(t.created_at);
    document.getElementById('receipt-status').textContent = t.transaction_status;

    openModal('modal-receipt');
  } catch (err) {
    showToast('Receipt Error', err.message, 'danger');
  }
};
