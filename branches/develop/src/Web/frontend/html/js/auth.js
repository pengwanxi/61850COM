/************************************************
 * 认证模块 - 登录/登出/会话管理
 ************************************************/
// WEB_FCGI_URL 定义在 config.js 中
const AUTH_LOGIN = "?login";
const AUTH_LOGOUT = "?auth_logout";
const CHANGE_PASSWORD = "?change_password";

/************************************************
 * 修改密码模块 - 模态框交互
 ************************************************/
(function () {
    // 等待 DOM 加载完成后再绑定事件，避免元素未找到
    document.addEventListener('DOMContentLoaded', function () {
        const modal = document.getElementById('pwdModal');
        const changeBtn = document.getElementById('changePwdBtn');
        // 如果模态框或按钮不存在（例如在登录页），则直接返回
        if (!modal || !changeBtn) return;

        const closeSpan = modal.querySelector('.close-modal');
        const cancelBtn = modal.querySelector('.btn-cancel');
        const pwdForm = document.getElementById('changePwdForm');

        // 打开模态框
        changeBtn.onclick = () => {
            modal.style.display = 'flex';
            pwdForm.reset();
        };

        // 关闭模态框
        const closeModal = () => {
            modal.style.display = 'none';
        };
        if (closeSpan) closeSpan.onclick = closeModal;
        if (cancelBtn) cancelBtn.onclick = closeModal;
        window.onclick = (e) => {
            if (e.target === modal) closeModal();
        };

        // 提交修改密码
        pwdForm.onsubmit = async (e) => {
            e.preventDefault();

            const oldPwd = document.getElementById('oldPwd').value.trim();
            const newPwd = document.getElementById('newPwd').value.trim();
            const confirmPwd = document.getElementById('confirmPwd').value.trim();

            // 前端校验
            if (!oldPwd || !newPwd || !confirmPwd) {
                showToast('请填写完整信息', 'error');
                return;
            }
            if (newPwd.length < 5) {
                showToast('新密码长度不能少于5位', 'error');
                return;
            }
            if (newPwd !== confirmPwd) {
                showToast('两次输入的新密码不一致', 'error');
                return;
            }

            // 在 pwdForm.onsubmit 中，获取用户名
            const user = getCurrentUser();
            if (!user || !user.username) {
                showToast('请先登录', 'error');
                return;
            }
            const username = user.username;

            // 构造请求参数
            const params = new URLSearchParams();
            params.append('username', username);
            params.append('old_password', oldPwd);
            params.append('new_password', newPwd);

            try {
                const url = WEB_FCGI_URL + CHANGE_PASSWORD;
                const response = await fetch(url, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded'
                    },
                    body: params.toString()
                });
                const data = await response.json();

                if (data.success) {
                    showToast('密码修改成功', 'success');
                    closeModal();
                    // 可选：提示用户重新登录
                    setTimeout(() => {
                        if (confirm('密码已修改，为安全起见建议重新登录。现在退出吗？')) {
                            logout();
                        }
                    }, 1000);
                } else {
                    showToast(data.message || '修改失败', 'error');
                }
            } catch (err) {
                console.error('修改密码请求失败', err);
                showToast('网络错误，请稍后重试', 'error');
            }
        };
    });
})();

// 检查是否已登录
function checkAuth() {
    const session = sessionStorage.getItem('emu_session');
    if (!session) {
        // 未登录，检查当前是否在登录页
        if (!window.location.href.includes('index.html')) {
            window.location.href = 'index.html';
        }
    } else {
        // 已登录，如果在登录页则跳转到配置页
        if (window.location.href.includes('index.html')) {
            window.location.href = 'config.html';
        }
    }
}

// 处理登录
function handleLogin(event) {
    event.preventDefault();

    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    const errorMsg = document.getElementById('errorMsg');

    // 构建请求URL
    const url = WEB_FCGI_URL + AUTH_LOGIN

    fetch(url, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: `username=${encodeURIComponent(username)}&password=${encodeURIComponent(password)}`
    })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                // 登录成功
                sessionStorage.setItem('emu_session', JSON.stringify({
                    username: username,
                    token: data.token,
                    timestamp: Date.now()
                }));
                window.location.href = 'config.html';
            } else {
                // 登录失败
                errorMsg.textContent = data.message || '用户名或密码错误';
            }
        })
        .catch(error => {
            console.error('登录请求失败:', error);
            errorMsg.textContent = '网络错误，请重试';

            // 开发测试：模拟登录成功
            if (username === 'admin' && password === 'admin') {
                sessionStorage.setItem('emu_session', JSON.stringify({
                    username: username,
                    token: 'test_token_123',
                    timestamp: Date.now()
                }));
                window.location.href = 'config.html';
            }
        });

    return false;
}

// 登出
function logout() {
    // 可选：通知服务器登出
    const url = WEB_FCGI_URL + AUTH_LOGOUT;

    fetch(url, {
        method: 'GET',
        credentials: 'same-origin'
    })
        .finally(() => {
            // 清除会话
            sessionStorage.removeItem('emu_session');
            window.location.href = 'index.html';
        });
}

// 获取当前登录用户信息
function getCurrentUser() {
    const session = sessionStorage.getItem('emu_session');
    if (session) {
        return JSON.parse(session);
    }
    return null;
}

// 检查会话是否过期（可选：2小时过期）
function isSessionExpired() {
    const session = getCurrentUser();
    if (session) {
        const now = Date.now();
        const twoHours = 2 * 60 * 60 * 1000;
        if (now - session.timestamp > twoHours) {
            logout();
            return true;
        }
    }
    return false;
}

/**
 * 更新页面中显示的用户名（新增）
 */
function updateCurrentUserDisplay() {
    const currentUserSpan = document.getElementById('currentUser');
    if (!currentUserSpan) return;

    const user = getCurrentUser();
    if (user && user.username) {
        currentUserSpan.textContent = user.username;
    } else {
        // 未登录时理论上不会显示该页面，但保留默认值
        currentUserSpan.textContent = 'admin';
    }
}

// 在页面加载时更新用户名（确保在 config.html 中生效）
document.addEventListener('DOMContentLoaded', function () {
    updateCurrentUserDisplay();
});

// 显示消息提示（最多同时显示6条）
let toastContainer = null;
function showToast(message, type = 'success') {
    // 获取或创建容器
    if (!toastContainer) {
        toastContainer = document.createElement('div');
        toastContainer.className = 'toast-container';
        document.body.appendChild(toastContainer);
    }

    // 限制最大显示条数为 6
    const MAX_TOASTS = 6;
    while (toastContainer.children.length >= MAX_TOASTS) {
        toastContainer.removeChild(toastContainer.firstChild);
    }

    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.textContent = message;
    toastContainer.appendChild(toast);

    // 2秒后自动移除本条消息
    setTimeout(() => {
        if (toast.parentNode === toastContainer) {
            toast.remove();
        }
        // 如果没有剩余消息，移除容器
        if (toastContainer.children.length === 0) {
            toastContainer.remove();
            toastContainer = null;
        }
    }, 2000);
}
/**
 * 显示重要提醒悬浮窗（中央固定，可关闭，跨页面持久）
 * @param {string} message 提醒内容
 * @param {Object} options 配置项
 * @param {Function} options.onClose 关闭后的回调
 * @param {boolean} options.persistAcrossPages 是否跨页面持久（默认true，挂载到body）
 */
function showImportantReminder(message, options = {}) {
    const {
        onClose = null,
        persistAcrossPages = true
    } = options;

    // 防止重复创建
    const existing = document.querySelector('.reminder-floating');
    if (existing) {
        // 如果已存在，只更新消息内容
        existing.querySelector('.reminder-content').textContent = message;
        return;
    }

    // 创建容器
    const reminderDiv = document.createElement('div');
    reminderDiv.className = 'reminder-floating';

    reminderDiv.innerHTML = `
        <div class="reminder-content">${escapeHtml(message)}</div>
        <div class="reminder-close">✕</div>
    `;

    // 挂载到 body，确保路由切换不影响
    document.body.appendChild(reminderDiv);

    // 关闭事件
    const closeBtn = reminderDiv.querySelector('.reminder-close');
    const removeReminder = () => {
        reminderDiv.remove();
        if (onClose && typeof onClose === 'function') onClose();
    };
    closeBtn.addEventListener('click', removeReminder);

    // 可选：点击外部不关闭，但为了用户体验，不额外处理
    // 注意：由于没有遮罩，用户可自由操作下方页面元素
}

// 辅助函数：防 XSS
function escapeHtml(str) {
    return str.replace(/[&<>]/g, function (m) {
        if (m === '&') return '&amp;';
        if (m === '<') return '&lt;';
        if (m === '>') return '&gt;';
        return m;
    });
}