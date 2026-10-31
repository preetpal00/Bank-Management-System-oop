/**
 * ============================================================================
 * BANK MANAGEMENT SYSTEM - AUTHENTICATION MODULE
 * ============================================================================
 */

document.addEventListener('DOMContentLoaded', () => {
  // Check if user is already logged in when visiting login/register
  const currentPath = window.location.pathname;
  const isAuthPage = currentPath.includes('login') || currentPath.includes('register');
  const token = AuthStore.getToken();

  if (isAuthPage && token) {
    window.location.href = '/dashboard';
    return;
  }

  // Password Visibility Toggle
  document.querySelectorAll('.password-toggle-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      const targetId = btn.getAttribute('data-target');
      const input = document.getElementById(targetId);
      if (input) {
        if (input.type === 'password') {
          input.type = 'text';
          btn.innerHTML = '<i class="fas fa-eye-slash"></i>';
        } else {
          input.type = 'password';
          btn.innerHTML = '<i class="fas fa-eye"></i>';
        }
      }
    });
  });

  // Login Form Submission
  const loginForm = document.getElementById('login-form');
  if (loginForm) {
    loginForm.addEventListener('submit', async (e) => {
      e.preventDefault();
      const email = document.getElementById('login-email').value.trim();
      const password = document.getElementById('login-password').value;
      const submitBtn = loginForm.querySelector('button[type="submit"]');

      if (!email || !password) {
        showToast('Validation Error', 'Please enter your email and password.', 'warning');
        return;
      }

      try {
        submitBtn.disabled = true;
        submitBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Authenticating...';

        const data = await apiRequest('/api/auth/login', {
          method: 'POST',
          body: JSON.stringify({ email, password })
        });

        AuthStore.setToken(data.token);
        AuthStore.setUser(data.user);

        showToast('Login Successful', `Welcome back, ${data.user.name}!`, 'success');
        setTimeout(() => {
          window.location.href = '/dashboard';
        }, 600);
      } catch (err) {
        showToast('Authentication Failed', err.message, 'danger');
        submitBtn.disabled = false;
        submitBtn.innerHTML = '<span>Sign In to Your Account</span> <i class="fas fa-arrow-right"></i>';
      }
    });

    // 1-Click Demo Login Fill Buttons
    const demoAdminBtn = document.getElementById('fill-demo-admin');
    if (demoAdminBtn) {
      demoAdminBtn.addEventListener('click', () => {
        document.getElementById('login-email').value = 'admin@bank.com';
        document.getElementById('login-password').value = 'AdminPassword123!';
      });
    }

    const demoCustomerBtn = document.getElementById('fill-demo-customer');
    if (demoCustomerBtn) {
      demoCustomerBtn.addEventListener('click', () => {
        document.getElementById('login-email').value = 'rahul.sharma@example.com';
        document.getElementById('login-password').value = 'Customer123!';
      });
    }
  }

  // Registration Form Submission
  const registerForm = document.getElementById('register-form');
  if (registerForm) {
    registerForm.addEventListener('submit', async (e) => {
      e.preventDefault();
      const fullName = document.getElementById('reg-name').value.trim();
      const email = document.getElementById('reg-email').value.trim();
      const phone = document.getElementById('reg-phone').value.trim();
      const address = document.getElementById('reg-address').value.trim();
      const dob = document.getElementById('reg-dob').value;
      const password = document.getElementById('reg-password').value;
      const confirmPassword = document.getElementById('reg-confirm-password').value;
      const submitBtn = registerForm.querySelector('button[type="submit"]');

      if (!fullName || !email || !password) {
        showToast('Missing Fields', 'Full name, email, and password are required.', 'warning');
        return;
      }

      if (password !== confirmPassword) {
        showToast('Password Mismatch', 'The passwords entered do not match.', 'danger');
        return;
      }

      if (password.length < 6) {
        showToast('Weak Password', 'Password must be at least 6 characters.', 'warning');
        return;
      }

      try {
        submitBtn.disabled = true;
        submitBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Creating Account...';

        const data = await apiRequest('/api/auth/register', {
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

        showToast('Registration Successful', data.message || 'Account opened! Please sign in.', 'success');
        setTimeout(() => {
          window.location.href = '/login';
        }, 1200);
      } catch (err) {
        showToast('Registration Failed', err.message, 'danger');
        submitBtn.disabled = false;
        submitBtn.innerHTML = '<span>Complete Registration</span> <i class="fas fa-check"></i>';
      }
    });
  }
});
