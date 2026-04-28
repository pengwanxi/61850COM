/************************************************
 * 硬件参数配置模块
 ************************************************/
// WEB_FCGI_URL 定义在 config.js 中
const HARDWARE_GET_CONFIG = "?hw_get_config";
const HARDWARE_SET_CONFIG = "?hw_set_config";
const HARDWARE_SET_TIME = "?hw_set_time";
const HARDWARE_GET_TIME = "?hw_get_time";


// 读取硬件配置
function loadHardwareConfig() {
    console.log('加载硬件配置...');
    const url = WEB_FCGI_URL + HARDWARE_GET_CONFIG;

    fetch(url, {
        method: 'GET',
        credentials: 'same-origin'
    })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                fillHardwareForm(data.config);
                showToast('Hardware配置读取成功');
                console.log('读取的Hardware配置:', data.config);
            } else {
                showToast('读取Hardware配置失败: ' + data.message, 'error');
                console.log('读取Hardware配置失败:', data.message);
            }
        })
        .catch(error => {
            console.error('读取Hardware配置失败:', error);
            showToast('网络错误，配置读取失败', 'error');
            console.log('读取Hardware配置失败:', error);
        });
}

// 保存硬件配置
function saveHardwareConfig() {
    const config = collectHardwareFormData();

    // 验证数据
    if (!validateHardwareConfig(config)) {
        return;
    }

    const url = WEB_FCGI_URL + HARDWARE_SET_CONFIG;

    fetch(url, {
        method: 'POST',
        credentials: 'same-origin',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(config)
    })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('Hardware配置保存成功');
                console.log('保存的Hardware配置:', config);
                showImportantReminder('配置已保存，请重启系统使更改生效！！', {
                    onClose: () => {
                        console.log('用户关闭了重启提醒');
                    }
                });
            } else {
                showToast('保存Hardware配置失败: ' + data.message, 'error');
                console.log('保存的Hardware配置:', config);
            }
        })
        .catch(error => {
            console.error('保存Hardware配置失败:', error);
            showToast('网络错误,Hardware配置未保存', 'error');
            console.log('保存的Hardware配置:', config);
        });
}

// 填充表单数据
function fillHardwareForm(config) {
    console.log('填充表单数据 config:', config);
    // 网口配置
    if (config.network) {
        config.network.forEach((eth, index) => {
            const ipInput = document.getElementById(`eth${index}_ip`);
            const maskInput = document.getElementById(`eth${index}_mask`);
            const gatewayInput = document.getElementById(`eth${index}_gateway`);
            if (ipInput) ipInput.value = eth.ip || '';
            if (maskInput) maskInput.value = eth.mask || '';
            if (gatewayInput) gatewayInput.value = eth.gateway || '';
            // const macInput = document.getElementById(`eth${index}_mac`);
            // if (macInput) macInput.value = eth.mac || '';
        });
    }

    // 串口配置
    // if (config.serial) {
    //     document.getElementById('rs485_baud').value = config.serial.baud || '115200';
    //     document.getElementById('rs485_data').value = config.serial.data || '8';
    //     document.getElementById('rs485_stop').value = config.serial.stop || '1';
    //     document.getElementById('rs485_parity').value = config.serial.parity || 'none';
    // }

    // IEC 61850 配置
    if (config.iec61850) {
        document.getElementById('iec_ied_name').value = config.iec61850.ied_name || '';
        // document.getElementById('iec_ld_inst').value = config.iec61850.ld_inst || '';
        // document.getElementById('iec_mms_port').value = config.iec61850.mms_port || '102';
        // document.getElementById('iec_goose_sub').value = config.iec61850.goose_sub || '1';
        // document.getElementById('iec_sv_sub').value = config.iec61850.sv_sub || '1';
    }
    // comtrade配置
    if (config.comtrade) {
        document.getElementById('dat_data_type').value = config.comtrade.dat_data_type || '';
    }
    // ntp配置
    if (config.ntpclient) {
        document.getElementById('ntp_client_ip').value = config.ntpclient.ServerIP || '';
        document.getElementById('ntp_client_port').value = config.ntpclient.ServerPort || '1';
        document.getElementById('timezone').value = config.ntpclient.timezone || '0';
    }
    //日志配置
    if (config.log) {
        document.getElementById('log_netcard').value = config.log.log_netcard || 'eth0';
        document.getElementById('log_ip').value = config.log.log_ip || '';
        document.getElementById('log_port').value = config.log.log_port || '';
    }
}

// 收集表单数据
function collectHardwareFormData() {
    // 动态收集实际存在的网口表单
    const network = [];
    let index = 0;
    while (true) {
        const ipInput = document.getElementById(`eth${index}_ip`);
        const maskInput = document.getElementById(`eth${index}_mask`);
        const gatewayInput = document.getElementById(`eth${index}_gateway`);
        if (!ipInput || !maskInput || !gatewayInput) break;
        network.push({
            interface: `eth${index}`,
            ip: ipInput.value,
            mask: maskInput.value,
            gateway: gatewayInput.value,
        });
        index++;
    }
    return {
        network,
        iec61850: {
            ied_name: document.getElementById('iec_ied_name').value,
            // ld_inst: document.getElementById('iec_ld_inst').value,
            // mms_port: parseInt(document.getElementById('iec_mms_port').value),
            // goose_sub: document.getElementById('iec_goose_sub').value,
            // sv_sub: document.getElementById('iec_sv_sub').value
        },
        comtrade: {
            dat_data_type: document.getElementById('dat_data_type').value,
        },
        ntpclient: {
            ServerIP: document.getElementById('ntp_client_ip').value,
            ServerPort: document.getElementById('ntp_client_port').value,
            timezone: document.getElementById('timezone').value,
        },
        log: {
            log_netcard: document.getElementById('log_netcard').value,
            log_ip: document.getElementById('log_ip').value,
            log_port: document.getElementById('log_port').value,
        }
    };
}

// 验证配置数据
function validateHardwareConfig(config) {
    // 验证IP地址格式
    const ipRegex = /^(\d{1,3}\.){3}\d{1,3}$/;

    for (let i = 0; i < config.network.length; i++) {
        const eth = config.network[i];
        if (eth.ip && !ipRegex.test(eth.ip)) {
            showToast(`网口 ${i + 1} 的IP地址格式错误`, 'error');
            return false;
        }
        if (eth.mask && !ipRegex.test(eth.mask)) {
            showToast(`网口 ${i + 1} 的子网掩码格式错误`, 'error');
            return false;
        }
        if (eth.gateway && !ipRegex.test(eth.gateway)) {
            showToast(`网口 ${i + 1} 的网关格式错误`, 'error');
            return false;
        }
    }

    // 验证MMS端口
    if (config.iec61850.mms_port < 1 || config.iec61850.mms_port > 65535) {
        showToast('MMS端口必须在 1-65535 之间', 'error');
        return false;
    }

    return true;
}
// 设置当前时间为输入框的值
function setNow() {
    const now = new Date();
    const year = now.getFullYear();
    const month = String(now.getMonth() + 1).padStart(2, '0');
    const day = String(now.getDate()).padStart(2, '0');
    const hours = String(now.getHours()).padStart(2, '0');
    const minutes = String(now.getMinutes()).padStart(2, '0');
    // datetime-local 需要格式 YYYY-MM-DDTHH:mm
    document.getElementById('manual_datetime').value = `${year}-${month}-${day}T${hours}:${minutes}`;
}
//获取当前设备时间
function getSystemTime() {
    const url = WEB_FCGI_URL + HARDWARE_GET_TIME;
    fetch(url, {
        method: 'GET',
        credentials: 'same-origin'
    })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                document.getElementById('current_system_time').textContent = data.message;
                showToast('获取设备时间成功');
                console.log('设备当前时间:', data.message);
            } else {
                showToast('获取设备时间失败', 'error');
            }
        })
        .catch(error => {
            console.error('获取设备时间失败:', error);
            showToast('网络错误，获取设备时间失败', 'error');
        });

}

// 手动设置系统时间
function setManualTime() {
    const datetimeStr = document.getElementById('manual_datetime').value;
    if (!datetimeStr) {
        showToast('请选择日期时间', 'error');
        return;
    }

    // 将 "YYYY-MM-DDTHH:mm" 转换为 "YYYY-MM-DD HH:MM:SS"（秒默认为00）
    const formatted = datetimeStr.replace('T', ' ') + ':00';

    // 可选：验证日期有效性
    const date = new Date(datetimeStr);
    if (isNaN(date.getTime())) {
        showToast('无效的日期时间', 'error');
        return;
    }

    console.log('设置时间：', formatted);

    const url = WEB_FCGI_URL + HARDWARE_SET_TIME;
    fetch(url, {
        method: 'POST',
        credentials: 'same-origin',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ time: formatted })
    })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('系统时间设置成功');
            } else {
                showToast('设置失败: ' + (data.message || '未知错误'), 'error');
            }
        })
        .catch(error => {
            console.error('设置时间失败:', error);
            showToast('网络错误，时间设置失败', 'error');
        });
}