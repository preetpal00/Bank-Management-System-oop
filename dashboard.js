/**
 * ============================================================================
 * BANK MANAGEMENT SYSTEM - DASHBOARD CONTROLLER
 * Fetches real database metrics and renders Chart.js visualizations.
 * ============================================================================
 */

let monthlyChartInstance = null;
let distChartInstance = null;

document.addEventListener('DOMContentLoaded', async () => {
  initAppShell('dashboard');
  const user = AuthStore.getUser();
  if (!user) return;

  if (user.role === 'ADMIN') {
    loadAdminDashboard();
  } else {
    loadCustomerDashboard();
  }
});

// Load Customer View
async function loadCustomerDashboard() {
  const customerView = document.getElementById('customer-dashboard-view');
  const adminView = document.getElementById('admin-dashboard-view');
  if (customerView) customerView.style.display = 'block';
  if (adminView) adminView.style.display = 'none';

  try {
    const data = await apiRequest('/api/dashboard/customer');

    // 1. Metric Cards
    document.getElementById('cust-total-balance').textContent = formatRupees(data.summary.total_balance_rupees);
    document.getElementById('cust-active-accounts').textContent = data.summary.active_accounts;
    document.getElementById('cust-total-deposits').textContent = formatRupees(data.summary.total_deposits_rupees);
    document.getElementById('cust-total-withdrawals').textContent = formatRupees(data.summary.total_withdrawals_rupees);

    // 2. Visual Bank Cards
    const cardsContainer = document.getElementById('cust-bank-cards-list');
    if (cardsContainer) {
      if (data.accounts && data.accounts.length > 0) {
        cardsContainer.innerHTML = data.accounts.map(acc => `
          <div class="bank-card-item">
            <div class="card-top">
              <span class="card-type-tag">${acc.account_type} ACCOUNT</span>
              <div class="card-chip"></div>
            </div>
            <div class="card-number">${acc.masked_account_number || acc.account_number}</div>
            <div class="card-bottom">
              <div>
                <div class="card-holder-label">Account Holder</div>
                <div class="card-holder-name">${acc.customer_name || 'Customer'}</div>
              </div>
              <div style="text-align: right;">
                <div class="card-balance-label">Available Balance</div>
                <div class="card-balance-val">${formatRupees(acc.balance_rupees)}</div>
              </div>
            </div>
          </div>
        `).join('');
      } else {
        cardsContainer.innerHTML = `
          <div class="empty-state">
            <i class="fas fa-credit-card"></i>
            <h4>No Accounts Found</h4>
            <p>You have no active accounts yet. Click 'My Accounts' to create one.</p>
          </div>
        `;
      }
    }

    // 3. Recent Transactions
    renderTransactionsTable('cust-recent-txns-tbody', data.recent_transactions);

    // 4. Render Chart.js Analytics (Real Data)
    renderCustomerCharts(data.charts);

  } catch (err) {
    showToast('Dashboard Error', err.message, 'danger');
  }
}

// Load Administrator View
async function loadAdminDashboard() {
  const customerView = document.getElementById('customer-dashboard-view');
  const adminView = document.getElementById('admin-dashboard-view');
  if (customerView) customerView.style.display = 'none';
  if (adminView) adminView.style.display = 'block';

  try {
    const data = await apiRequest('/api/dashboard/admin');

    // 1. Admin Metric Cards
    document.getElementById('admin-total-balance').textContent = formatRupees(data.summary.total_bank_balance_rupees);
    document.getElementById('admin-total-customers').textContent = data.summary.total_customers;
    document.getElementById('admin-total-accounts').textContent = data.summary.total_active_accounts;
    document.getElementById('admin-total-transfers').textContent = formatRupees(data.summary.total_transfers_rupees);

    // 2. System Recent Transactions
    renderTransactionsTable('admin-recent-txns-tbody', data.recent_transactions);

    // 3. Admin Audit Logs
    const logsTbody = document.getElementById('admin-audit-logs-tbody');
    if (logsTbody && data.admin_logs) {
      if (data.admin_logs.length === 0) {
        logsTbody.innerHTML = `<tr><td colspan="4" class="text-center" style="text-align:center;">No administrative activity logged.</td></tr>`;
      } else {
        logsTbody.innerHTML = data.admin_logs.map(l => `
          <tr>
            <td><span class="badge badge-neutral">${l.action}</span></td>
            <td>${l.description}</td>
            <td><strong>${l.admin_name || 'Admin'}</strong></td>
            <td>${formatDate(l.created_at)}</td>
          </tr>
        `).join('');
      }
    }

    // 4. Render Admin System Charts
    renderAdminCharts(data.charts);

  } catch (err) {
    showToast('Dashboard Error', err.message, 'danger');
  }
}

// Transaction Table Helper
function renderTransactionsTable(tbodyId, transactions) {
  const tbody = document.getElementById(tbodyId);
  if (!tbody) return;

  if (!transactions || transactions.length === 0) {
    tbody.innerHTML = `
      <tr>
        <td colspan="6" style="text-align: center; padding: 24px; color: var(--text-muted);">
          No transactions recorded yet.
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
      </tr>
    `;
  }).join('');
}

// Chart.js: Customer Charts
function renderCustomerCharts(chartsData) {
  // Chart 1: Monthly Income vs Expenses (Bar Chart)
  const barCanvas = document.getElementById('chart-monthly-income-expense');
  if (barCanvas && chartsData.monthly_comparison) {
    if (monthlyChartInstance) monthlyChartInstance.destroy();

    const comp = chartsData.monthly_comparison;
    monthlyChartInstance = new Chart(barCanvas.getContext('2d'), {
      type: 'bar',
      data: {
        labels: comp.labels.length > 0 ? comp.labels : ['Current Period'],
        datasets: [
          {
            label: 'Deposits / Income (₹)',
            data: comp.income.length > 0 ? comp.income : [0],
            backgroundColor: '#16A36A',
            borderRadius: 6
          },
          {
            label: 'Withdrawals / Outflow (₹)',
            data: comp.expense.length > 0 ? comp.expense : [0],
            backgroundColor: '#E05252',
            borderRadius: 6
          }
        ]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { position: 'top' },
          tooltip: {
            callbacks: {
              label: (ctx) => `${ctx.dataset.label}: ₹${ctx.parsed.y.toLocaleString('en-IN')}`
            }
          }
        },
        scales: {
          y: {
            beginAtZero: true,
            ticks: {
              callback: (val) => '₹' + Number(val).toLocaleString('en-IN')
            }
          }
        }
      }
    });
  }

  // Chart 2: Transaction Distribution (Doughnut Chart)
  const doughnutCanvas = document.getElementById('chart-transaction-distribution');
  if (doughnutCanvas && chartsData.distribution) {
    if (distChartInstance) distChartInstance.destroy();

    const labels = chartsData.distribution.map(d => d.transaction_type.replace('_', ' '));
    const counts = chartsData.distribution.map(d => d.count);

    distChartInstance = new Chart(doughnutCanvas.getContext('2d'), {
      type: 'doughnut',
      data: {
        labels: labels.length > 0 ? labels : ['No Data'],
        datasets: [{
          data: counts.length > 0 ? counts : [1],
          backgroundColor: ['#16A36A', '#E05252', '#36A6A0', '#E9A23B', '#245B91']
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
          legend: { position: 'right' }
        }
      }
    });
  }
}

// Chart.js: Administrator Charts
function renderAdminCharts(chartsData) {
  // Chart 1: Monthly Activity
  const barCanvas = document.getElementById('admin-chart-monthly-activity');
  if (barCanvas && chartsData.monthly_activity) {
    const act = chartsData.monthly_activity;
    new Chart(barCanvas.getContext('2d'), {
      type: 'bar',
      data: {
        labels: act.labels.length > 0 ? act.labels : ['Current Period'],
        datasets: [
          {
            label: 'Deposits (₹)',
            data: act.deposits.length > 0 ? act.deposits : [0],
            backgroundColor: '#16A36A',
            borderRadius: 4
          },
          {
            label: 'Withdrawals (₹)',
            data: act.withdrawals.length > 0 ? act.withdrawals : [0],
            backgroundColor: '#E05252',
            borderRadius: 4
          },
          {
            label: 'Transfers (₹)',
            data: act.transfers.length > 0 ? act.transfers : [0],
            backgroundColor: '#245B91',
            borderRadius: 4
          }
        ]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: { legend: { position: 'top' } }
      }
    });
  }

  // Chart 2: Account Distribution
  const doughnutCanvas = document.getElementById('admin-chart-account-distribution');
  if (doughnutCanvas && chartsData.account_distribution) {
    const labels = chartsData.account_distribution.map(a => a.account_type);
    const counts = chartsData.account_distribution.map(a => a.count);

    new Chart(doughnutCanvas.getContext('2d'), {
      type: 'pie',
      data: {
        labels: labels.length > 0 ? labels : ['No Data'],
        datasets: [{
          data: counts.length > 0 ? counts : [1],
          backgroundColor: ['#142D4E', '#245B91', '#36A6A0']
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: { legend: { position: 'right' } }
      }
    });
  }
}
