/**
 * ============================================================================
 * BANK MANAGEMENT SYSTEM - ADMIN REPORTS & EXPORT CONTROLLER
 * ============================================================================
 */

document.addEventListener('DOMContentLoaded', () => {
  initAppShell('reports');
  const user = AuthStore.getUser();
  if (!user || user.role !== 'ADMIN') {
    window.location.href = '/dashboard';
    return;
  }

  // Load Initial Report Data
  loadDailyReport();
  loadMonthlyReport();
  loadAccountReport();

  // Daily Date Change Handler
  const dailyDateInput = document.getElementById('report-daily-date');
  if (dailyDateInput) {
    dailyDateInput.value = new Date().toISOString().split('T')[0];
    dailyDateInput.addEventListener('change', () => loadDailyReport(dailyDateInput.value));
  }

  // Monthly Date Change Handler
  const monthInput = document.getElementById('report-month-select');
  if (monthInput) {
    const now = new Date();
    const ym = `${now.getFullYear()}-${String(now.getMonth() + 1).padStart(2, '0')}`;
    monthInput.value = ym;
    monthInput.addEventListener('change', () => loadMonthlyReport(monthInput.value));
  }

  // Export to CSV Button
  const exportBtn = document.getElementById('btn-export-csv');
  if (exportBtn) {
    exportBtn.addEventListener('click', async () => {
      try {
        exportBtn.disabled = true;
        exportBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Generating CSV...';

        const res = await apiRequest('/api/reports/export');
        showToast('Export Successful', res.message || 'CSV generated.', 'success');

        // Trigger browser download via direct static URL
        const downloadLink = document.createElement('a');
        downloadLink.href = res.download_url;
        downloadLink.download = 'bank_transactions_report.csv';
        document.body.appendChild(downloadLink);
        downloadLink.click();
        downloadLink.remove();
      } catch (err) {
        showToast('Export Failed', err.message, 'danger');
      } finally {
        exportBtn.disabled = false;
        exportBtn.innerHTML = '<i class="fas fa-file-csv"></i> <span>Export All Transactions to CSV</span>';
      }
    });
  }
});

async function loadDailyReport(dateStr = '') {
  const container = document.getElementById('daily-report-content');
  if (!container) return;

  try {
    const data = await apiRequest(`/api/reports/daily?date=${dateStr}`);
    document.getElementById('daily-turnover-val').textContent = formatRupees(data.total_turnover_rupees);

    const tbody = document.getElementById('daily-report-tbody');
    if (tbody) {
      if (!data.transactions || data.transactions.length === 0) {
        tbody.innerHTML = `<tr><td colspan="6" style="text-align:center; padding: 20px; color: var(--text-muted);">No transactions recorded on this date.</td></tr>`;
      } else {
        tbody.innerHTML = data.transactions.map(t => `
          <tr>
            <td><strong>${t.reference_number}</strong></td>
            <td>${t.account_number}</td>
            <td>${t.full_name}</td>
            <td><span class="badge badge-info">${t.transaction_type}</span></td>
            <td style="font-weight: 600;">${formatRupees(t.amount_rupees)}</td>
            <td>${t.created_at}</td>
          </tr>
        `).join('');
      }
    }
  } catch (err) {
    console.error(err);
  }
}

async function loadMonthlyReport(monthStr = '') {
  try {
    const data = await apiRequest(`/api/reports/monthly?month=${monthStr}`);
    document.getElementById('monthly-volume-val').textContent = formatRupees(data.total_volume_rupees);

    const tbody = document.getElementById('monthly-breakdown-tbody');
    if (tbody && data.breakdown) {
      if (data.breakdown.length === 0) {
        tbody.innerHTML = `<tr><td colspan="3" style="text-align:center; padding: 20px; color: var(--text-muted);">No activity recorded in this month.</td></tr>`;
      } else {
        tbody.innerHTML = data.breakdown.map(b => `
          <tr>
            <td><strong>${b.transaction_type}</strong></td>
            <td>${b.total_count}</td>
            <td style="font-weight: 700; color: var(--primary);">${formatRupees(b.total_rupees)}</td>
          </tr>
        `).join('');
      }
    }
  } catch (err) {
    console.error(err);
  }
}

async function loadAccountReport() {
  const tbody = document.getElementById('account-report-tbody');
  if (!tbody) return;

  try {
    const data = await apiRequest('/api/reports/accounts');
    tbody.innerHTML = data.map(r => `
      <tr>
        <td><strong>${r.account_type}</strong></td>
        <td><span class="badge badge-neutral">${r.status}</span></td>
        <td>${r.account_count}</td>
        <td style="font-weight: 700;">${formatRupees(r.total_balance_rupees)}</td>
        <td>${formatRupees(r.avg_balance_rupees)}</td>
      </tr>
    `).join('');
  } catch (err) {
    console.error(err);
  }
}
