/************************************************
 * 文件上传下载模块
 ************************************************/
// WEB_FCGI_URL 定义在 config.js 中
const UPLOAD_FIRMWARE = "?upload_firmware";
const UPLOAD_CONFIG = "?upload_config";
const DOWNLOAD_61850LOG = "?download_61850log";
const EXPORT_CONFIG = "?export_config";
const RESET_FACTORY = "?reset_factory";
const UPGRADE_STATUS = "?upgrade_status";
const SYS_REBOOT = "?reboot";
//轮询控制器
let upgradePollingTimer = null;        // 轮询定时器ID
let isUpgradeFinished = false;         // 升级是否已结束（成功/失败）
const FETCH_TIMEOUT = 5000;            // 请求超时5秒
// 带超时的 fetch 封装
async function fetchWithTimeout(url, options = {}, timeout = FETCH_TIMEOUT) {
    const controller = new AbortController();
    const id = setTimeout(() => controller.abort(), timeout);
    return fetch(url, { ...options, signal: controller.signal })
        .finally(() => clearTimeout(id));
}
// 停止轮询
function stopUpgradePolling() {
    if (upgradePollingTimer) {
        clearInterval(upgradePollingTimer);
        upgradePollingTimer = null;
    }
}

// 文件选择处理
document.addEventListener('DOMContentLoaded', function () {
    const firmwareFile = document.getElementById('firmwareFile');
    const configFile = document.getElementById('configFile');

    // 固件 change 事件：存储到全局，更新显示
    if (firmwareFile) {
        firmwareFile.addEventListener('change', function (e) {
            const file = e.target.files[0];
            if (file) {
                window.selectedFirmware = file;
                document.getElementById('firmwareFileName').innerText = file.name;
                document.getElementById('firmwareStatus').innerText = "已选择，可点击上传";
                document.getElementById('firmwareStatus').className = "status-info";
            } else {
                window.selectedFirmware = null;
                document.getElementById('firmwareFileName').innerText = "";
                document.getElementById('firmwareStatus').innerText = "";
            }
        });
    }


    if (configFile) {
        configFile.addEventListener('change', function (e) {
            const fileName = e.target.files[0]?.name || '';
            document.getElementById('configFileName').textContent = fileName;
        });
    }

    // 拖拽上传支持
    setupDragDrop();
});

//文件选择时清空状态
document.getElementById("firmwareFile").addEventListener("change", function () {
    const file = this.files[0];
    const name = document.getElementById("firmwareFileName");
    const status = document.getElementById("firmwareStatus");

    if (file) {
        name.innerText = file.name;
    }

    // 清空状态
    status.innerText = "";
    status.className = "";
});

// 设置拖拽上传
function setupDragDrop() {
    const dropZones = document.querySelectorAll('.file-drop-zone');

    dropZones.forEach(zone => {
        zone.addEventListener('dragover', (e) => {
            e.preventDefault();
            zone.style.borderColor = '#667eea';
            zone.style.backgroundColor = '#f0f4ff';
        });

        zone.addEventListener('dragleave', (e) => {
            e.preventDefault();
            zone.style.borderColor = '#ccc';
            zone.style.backgroundColor = '';
        });

        zone.addEventListener('drop', (e) => {
            e.preventDefault();
            zone.style.borderColor = '#ccc';
            zone.style.backgroundColor = '';

            const files = e.dataTransfer.files;
            if (files.length === 0) return;

            const file = files[0];
            // 判断是哪个区域
            if (zone.parentElement.querySelector('#firmwareFile')) {
                // 固件区域：可以加类型校验
                if (!file.name.toLowerCase().endsWith('.bin')) {
                    showToast("请上传 .bin 固件文件", "error");
                    return;
                }
                window.selectedFirmware = file;
                document.getElementById('firmwareFileName').innerText = file.name;
                document.getElementById('firmwareStatus').innerText = "已选择，可点击上传";
                document.getElementById('firmwareStatus').className = "status-info";
            } else if (zone.parentElement.querySelector('#configFile')) {
                // 配置区域
                if (!file.name.toLowerCase().endsWith('.cfg') && !file.name.toLowerCase().endsWith('.json')) {
                    showToast("请上传 .cfg 或 .json 配置文件", "error");
                    return;
                }
                window.selectedConfig = file;
                document.getElementById('configFileName').innerText = file.name;
            }
        });
    });
}

//CRC校验
async function crc32(file) {
    const buf = await file.arrayBuffer();

    let table = [];

    for (let i = 0; i < 256; i++) {
        let c = i;

        for (let j = 0; j < 8; j++)
            c = (c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1);

        table[i] = c;
    }

    let crc = 0 ^ (-1);

    let data = new Uint8Array(buf);

    for (let i = 0; i < data.length; i++)
        crc = (crc >>> 8) ^ table[(crc ^ data[i]) & 0xFF];

    return (crc ^ (-1)) >>> 0;
}

// 上传固件（分块）
const CHUNK_SIZE = 256 * 1024;
async function uploadFirmware() {

    const uploadBtn = document.getElementById("uploadBtn");
    const file = window.selectedFirmware;
    if (!file) {
        const inputFile = document.getElementById("firmwareFile").files[0];
        if (inputFile) file = inputFile;
    }
    // 检查文件
    if (!file) {
        showToast("请选择固件", "error");
        return;
    }
    //锁按钮
    uploadBtn.disabled = true;
    uploadBtn.innerText = "上传中...";

    // 显示进度条
    const progressBar = document.getElementById('uploadProgress');
    const progressFill = document.getElementById("progressFill");
    const progressText = document.getElementById("progressText");
    progressBar.style.display = 'block';
    progressFill.style.width = '0%';
    progressText.innerText = '0%';

    const size = file.size;
    let offset = 0;

    try {
        while (offset < size) {
            let chunk = file.slice(offset, offset + CHUNK_SIZE);

            let res = await fetch(WEB_FCGI_URL + "?upload_firmware&chunk", {
                method: "POST",
                headers: {
                    "X-FILENAME": file.name,
                    "X-Expected-Offset": offset //告知后端当前已上传大小
                },
                body: chunk
            });

            let json = await res.json();
            const status = document.getElementById("firmwareStatus");
            if (!json.success) {
                // 显示后端返回的错误信息
                showToast("上传失败: " + (json.message || "未知错误"), "error");
                status.innerText = "✖ 上传失败";
                status.className = "status-error";
                return;
            }

            offset = json.offset;
            let percent = Math.floor(offset / size * 100);
            progressFill.style.width = percent + "%";
            progressText.innerText = percent + "%";
        }

        // 所有分块上传完成，计算 CRC 并发送 finish 请求
        const crc = await crc32(file);
        let finishRes = await fetch(WEB_FCGI_URL + "?upload_firmware&finish", {
            method: "POST",
            headers: {
                "X-CRC32": crc.toString(16),
                "X-FILENAME": file.name
            }
        });

        const status = document.getElementById("firmwareStatus");
        let finishJson = await finishRes.json();
        if (!finishJson.success) {
            showToast("升级启动失败: " + (finishJson.message || "CRC校验错误"), "error");
            status.innerText = "✖ 上传失败";
            status.className = "status-error";
        } else {
            showToast("上传bin文件完成");
            status.innerText = "✔ 上传成功";
            status.className = "status-success";
            // 重置完成标志（保证新升级正常轮询）
            isUpgradeFinished = false;
            // 开始轮询升级状态
            pollUpgradeStatus();
        }
    } catch (err) {
        console.error("上传异常:", err);
        showToast("上传过程中发生异常", "error");
    } finally {
        //解锁按钮
        const uploadBtn = document.getElementById("uploadBtn");
        uploadBtn.disabled = false;
        uploadBtn.innerText = "上传固件";
        // 无论成功或失败，隐藏进度条并重置
        setTimeout(function () {
            progressBar.style.display = 'none';
            progressFill.style.width = '0%';
            progressText.innerText = '0%';
        }, 2000);
    }
}

function pollUpgradeStatus() {
    // 如果升级已经完成，不再启动新轮询
    if (isUpgradeFinished) return;

    // 清除之前可能存在的定时器，确保只有一个轮询实例
    stopUpgradePolling();

    upgradePollingTimer = setInterval(async () => {
        try {
            // 使用带超时的 fetch，避免请求无限挂起
            let res = await fetchWithTimeout(WEB_FCGI_URL + UPGRADE_STATUS);
            let data = await res.json();

            // 如果升级已经结束，忽略后续响应
            if (isUpgradeFinished) return;

            if (!data.success) {
                stopUpgradePolling();
                showToast("获取升级状态失败", "error");
                return;
            }

            const statusEl = document.getElementById("firmwareStatus");
            const progressBar = document.getElementById('uploadProgress');
            const progressFill = document.getElementById("progressFill");
            const progressText = document.getElementById("progressText");

            if (data.progress !== undefined) {
                progressBar.style.display = 'block';
                progressFill.style.width = data.progress + '%';
                progressText.innerText = data.progress + '%';
            }

            switch (data.status) {
                case 1: // RUNNING
                    statusEl.innerText = "⏳ 升级中... (" + (data.message || "") + ")";
                    statusEl.className = "status-running";
                    break;

                case 2: // SUCCESS
                    isUpgradeFinished = true;
                    stopUpgradePolling();

                    statusEl.innerText = "✔ 升级成功";
                    statusEl.className = "status-success";
                    progressFill.style.width = '100%';
                    progressText.innerText = '100%';
                    showToast("升级成功");
                    loadLastUpgradeInfo();
                    setTimeout(() => { progressBar.style.display = 'none'; }, 2000);

                    setTimeout(() => {
                        if (confirm('升级成功，需要重启系统使配置生效。是否立即重启？')) {
                            rebootWithoutConfirm();
                        }
                    }, 400);
                    break;

                case 3: // FAILED
                    isUpgradeFinished = true;
                    stopUpgradePolling();

                    statusEl.innerText = "✖ 升级失败: " + (data.message || "未知错误");
                    statusEl.className = "status-error";
                    progressFill.style.width = '100%';
                    progressText.innerText = '失败';
                    showToast("升级失败: " + data.message, "error");
                    setTimeout(() => { progressBar.style.display = 'none'; }, 2000);
                    break;

                default:
                    // IDLE 状态，继续轮询
                    break;
            }
        } catch (err) {
            // 网络波动或超时，不停止轮询，等待网络恢复
            console.warn("轮询状态失败（网络波动）", err);
            showToast("获取升级状态失败:" + err, "error");
        }
    }, 850);
}

async function loadLastUpgradeInfo() {
    try {
        let res = await fetch(WEB_FCGI_URL + UPGRADE_STATUS);
        let data = await res.json();
        // console.log("后端返回的upgrade status data", data);
        if (data.success && data.status !== 0) {
            // 显示最近一次升级信息（即使 status=1 也显示）
            document.getElementById('last-filename').innerText = data.filename || '未知';
            document.getElementById('last-time').innerText = data.time || '未知';
            document.getElementById('last-status').innerText =
                data.status === 2 ? '成功' : (data.status === 3 ? '失败' : '进行中');
            document.getElementById('last-message').innerText = data.message || '';
            // 如果状态是进行中（status === 1），且页面未在轮询，则自动启动
            if (data.status === 1 && !upgradePollingTimer) {
                isUpgradeFinished = false;
                pollUpgradeStatus();
            }
        } else {
            // 无记录
        }
    } catch (err) {
        console.error("Failed to load last upgrade info", err);
    }
}

// 页面加载时调用
window.addEventListener('load', loadLastUpgradeInfo);

// 下载61850日志文件
function download61850Log() {
    const url = WEB_FCGI_URL + DOWNLOAD_61850LOG;

    fetch(url, {
        method: 'GET',
        credentials: 'same-origin'
    })
        .then(response => {
            if (!response.ok) throw new Error('下载失败');
            return response.blob();
        })
        .then(blob => {
            const downloadUrl = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = downloadUrl;
            const dateStr = new Date().toISOString().slice(0, 10);
            a.download = `logs_${dateStr}.tar.gz`;
            document.body.appendChild(a);
            a.click();
            window.URL.revokeObjectURL(downloadUrl);
            document.body.removeChild(a);
            showToast('日志打包下载成功');
        })
        .catch(error => {
            console.error('下载失败:', error);
            showToast('下载失败，请重试', 'error');
        });
}

// 导出配置
function exportConfig() {
    const url = WEB_FCGI_URL + EXPORT_CONFIG;

    fetch(url)
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                // 创建下载
                const blob = new Blob([JSON.stringify(data.config, null, 2)], { type: 'application/json' });
                const downloadUrl = window.URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = downloadUrl;
                a.download = 'EMU2000_export_' + new Date().toISOString().slice(0, 10) + '.json';
                document.body.appendChild(a);
                a.click();
                window.URL.revokeObjectURL(downloadUrl);
                document.body.removeChild(a);
                showToast('配置导出成功');
            }
        })
        .catch(error => {
            console.error('导出失败:', error);
            showToast('导出失败', 'error');
        });
}

// 上传配置文件
function uploadConfig() {
    const fileInput = document.getElementById('configFile');
    const file = fileInput.files[0] || window.selectedConfig;

    if (!file) {
        showToast('请选择配置文件', 'error');
        return;
    }

    if (!file.name.endsWith('.cfg') && !file.name.endsWith('.json')) {
        showToast('请选择 .cfg 或 .json 格式的配置文件', 'error');
        return;
    }

    const formData = new FormData();
    formData.append('file', file);
    formData.append('type', 'config');

    const url = WEB_FCGI_URL + UPLOAD_CONFIG;

    fetch(url, {
        method: 'POST',
        credentials: 'same-origin',
        body: formData
    })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('配置上传成功，设备将重启...');
            } else {
                showToast('上传失败: ' + data.message, 'error');
            }
        })
        .catch(error => {
            console.error('上传失败:', error);
            showToast('上传失败，请重试', 'error');
        });
}

// 恢复出厂设置
function resetConfig() {
    if (!confirm('确定要恢复出厂设置吗？所有配置将丢失！')) {
        return;
    }

    const url = WEB_FCGI_URL + RESET_FACTORY;

    fetch(url, {
        method: 'POST',
        credentials: 'same-origin'
    })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('已恢复出厂设置，设备将重启');
            } else {
                showToast('恢复失败: ' + data.message, 'error');
            }
        })
        .catch(error => {
            console.error('恢复失败:', error);
            showToast('恢复失败', 'error');
        });
}
//重启系统
function reboot() {
    if (!confirm('确定要重启系统吗？')) {
        return;
    }
    const url = WEB_FCGI_URL + SYS_REBOOT;
    fetch(url, {
        method: 'POST',
        credentials: 'same-origin'
    })
        .then(r => r.json())
        .then(data => {
            if (data.success) {
                showToast('系统正在重启...');
                setTimeout(() => {
                    location.reload();
                }, 5000)
            } else {
                showToast('重启失败: ' + data.message, 'error')
            }
        })
        .catch(err => {
            console.error(err)
            showToast('设备正在重启...', 'info')
        });
}
function rebootWithoutConfirm() {
    const url = WEB_FCGI_URL + SYS_REBOOT;
    fetch(url, {
        method: 'POST',
        credentials: 'same-origin'
    })
        .then(r => r.json())
        .then(data => {
            if (data.success) {
                showToast('系统正在重启...');
                setTimeout(() => {
                    location.reload();
                }, 5000);
            } else {
                showToast('重启失败: ' + data.message, 'error');
            }
        })
        .catch(err => {
            console.error(err);
            showToast('设备正在重启...', 'info');
        });
}