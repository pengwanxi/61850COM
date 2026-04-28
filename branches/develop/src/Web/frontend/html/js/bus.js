/************************************************
 * 总线装置配置模块 - 表格内联编辑版
 ************************************************/
// WEB_FCGI_URL 定义在 config.js 中
const BUS_GET_CONFIG = "?bus_get_config";
const BUS_SET_CONFIG = "?bus_set_config";
const BUS_GET_STATUS = "?bus_get_status";

let statusTimer = null;          // 仅用于存储定时器ID
let currentStatusValid = false;   // 状态是否有效
let currentStatus = null;         // 实际状态值（0断1通）

// 数据结构
let busConfigData = {
    commRows: []
};

let selectedRowId = null; // 当前选中的通讯行ID
let editingDeviceId = null; // 当前正在编辑的装置ID
let isNewDevice = false; // 标记是否是新添加的装置

// 协议选项 - 只有ModbusMaster
const PROTOCOL_OPTIONS = [
    { value: 'modbusmaster', text: 'Modbus Master' }
];

// 模板选项 - 4个模板（根据总线类型自动选择）
const TEMPLATE_OPTIONS = {
    'modbusmaster': [
        { value: 'EATONPXR20_TCP', text: 'EATONPXR20_TCP' },
        { value: 'EATONPXR20_RTU', text: 'EATONPXR20_RTU' },
        { value: 'EATONPXR25_TCP', text: 'EATONPXR25_TCP' },
        { value: 'EATONPXR25_RTU', text: 'EATONPXR25_RTU' }
    ]
};

// 根据总线类型推荐默认模板
const DEFAULT_TEMPLATE_BY_BUS = {
    'eth0': 'EATONPXR25_TCP',
    'rs485': 'EATONPXR25_RTU'
};

// 模板到协议模块的映射
const TEMPLATE_TO_MODULE = {
    'EATONPXR20_TCP': 'PXR-20',
    'EATONPXR20_RTU': 'PXR-20',
    'EATONPXR25_TCP': 'PXR-25',
    'EATONPXR25_RTU': 'PXR-25'
};

function startStatusPolling() {
    stopStatusPolling();          // 清除已有定时器
    fetchBusStatus();             // 立即获取一次
    statusTimer = setInterval(fetchBusStatus, 1000);
}

function stopStatusPolling() {
    if (statusTimer !== null) {
        clearInterval(statusTimer);
        statusTimer = null;
    }
}

// fetchBusStatus 中设置 currentStatusValid 和 currentStatus
function fetchBusStatus() {
    const url = WEB_FCGI_URL + BUS_GET_STATUS;
    fetch(url)
        .then(response => response.json())
        .then(data => {
            if (data.success && typeof data.link !== 'undefined') {
                currentStatusValid = true;
                currentStatus = data.link;
            } else {
                currentStatusValid = false;
                console.warn('获取通讯状态失败:', data.message);
                showToast('获取通讯状态失败: ' + data.message, 'error');
            }
            updateDeviceStatusDisplay();
        })
        .catch(err => {
            currentStatusValid = false;
            updateDeviceStatusDisplay();
            console.error('获取通讯状态请求异常:', err);
            showToast('获取通讯状态请求异常: ' + err, 'error');
        });
}
//通讯状态更新函数
function updateDeviceStatusDisplay() {
    const statusLight = document.getElementById('statusLight');
    const statusText = document.getElementById('statusText');
    if (!statusLight) return;

    if (!currentStatusValid) {
        // 未知状态：灰灯，显示“未知”
        statusLight.className = 'status-light unknown';
        statusLight.style.backgroundColor = '#9e9e9e';
        statusLight.style.boxShadow = 'none';
        statusText.textContent = '未知';
    } else if (currentStatus === 1) {
        statusLight.className = 'status-light on';
        statusLight.style.backgroundColor = '#4caf50';
        statusLight.style.boxShadow = '0 0 4px #4caf50';
        statusText.textContent = '通讯正常';
    } else {
        statusLight.className = 'status-light off';
        statusLight.style.backgroundColor = '#f44336';
        statusLight.style.boxShadow = '0 0 4px #f44336';
        statusText.textContent = '通讯中断';
    }
}

// 获取协议模块（PXR-20 或 PXR-25）
function getProtocolModule(device) {
    // 优先使用 device.protocolModule（如果已设置）
    if (device.protocolModule) {
        return device.protocolModule;
    }
    // 否则根据模板推断
    if (device.template && TEMPLATE_TO_MODULE[device.template]) {
        return TEMPLATE_TO_MODULE[device.template];
    }
    // 默认返回 PXR-25
    return 'PXR-25';
}

// 根据协议模块获取模板选项HTML
function getTemplateOptionsByModule(module, selectedTemplate) {
    if (!module) {
        // 如果模块未选择，显示所有模板
        return TEMPLATE_OPTIONS['modbusmaster'].map(opt =>
            `<option value="${opt.value}" ${selectedTemplate === opt.value ? 'selected' : ''}>${opt.text}</option>`
        ).join('');
    }

    // 根据模块过滤模板
    const templates = TEMPLATE_OPTIONS['modbusmaster'].filter(opt => {
        return TEMPLATE_TO_MODULE[opt.value] === module;
    });

    return templates.map(opt =>
        `<option value="${opt.value}" ${selectedTemplate === opt.value ? 'selected' : ''}>${opt.text}</option>`
    ).join('');
}

// 根据协议模块更新模板选项
function updateTemplateOptionsByModule(deviceId) {
    const moduleSelect = document.getElementById(`edit_protocolModule_${deviceId}`);
    const templateSelect = document.getElementById(`edit_template_${deviceId}`);
    const module = moduleSelect.value;

    // 清空模板选项
    templateSelect.innerHTML = '<option value="">选择模板</option>';

    if (!module) {
        // 如果模块未选择，不显示模板选项
        return;
    }

    // 根据模块添加对应的模板选项
    const templates = TEMPLATE_OPTIONS['modbusmaster'].filter(opt => {
        return TEMPLATE_TO_MODULE[opt.value] === module;
    });

    templates.forEach(opt => {
        const option = document.createElement('option');
        option.value = opt.value;
        option.textContent = opt.text;
        templateSelect.appendChild(option);
    });

    // 如果有模板选项，默认选择第一个
    if (templates.length > 0) {
        templateSelect.value = templates[0].value;
    }
}

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', function () {
    initBusConfig();
});

// 初始化总线配置
function initBusConfig() {
    console.log('总线配置模块初始化');
    loadBusConfig();
}

// 加载总线配置
function loadBusConfig() {
    const url = WEB_FCGI_URL + BUS_GET_CONFIG;

    fetch(url, {
        method: 'GET',
        credentials: 'same-origin'
    })
        .then(response => response.json())
        .then(data => {
            if (data.success && data.config) {
                busConfigData = data.config;
                renumberRowsAndDevices();
                // 确保所有通讯行的间隔值有效
                busConfigData.commRows.forEach(row => {
                    // 如果后端没有 internalEth/internal485，则根据当前类型初始化
                    if (row.type === 'eth0') {
                        row.internalEth = row.internal !== undefined ? row.internal : 20;
                        if (row.internal485 === undefined) row.internal485 = 300;
                    } else if (row.type === 'rs485') {
                        row.internal485 = row.internal !== undefined ? row.internal : 300;
                        if (row.internalEth === undefined) row.internalEth = 20;
                    } else {
                        // 类型未设置，默认给两个值
                        if (row.internalEth === undefined) row.internalEth = 20;
                        if (row.internal485 === undefined) row.internal485 = 300;
                        row.internal = row.internalEth;
                    }
                    // ==== 根据 module 覆盖 protocolModule 和 template ====
                    if (row.devices) {
                        row.devices.forEach(device => {
                            if (device.module !== undefined) {
                                // 定义 module 到 protocolModule 的映射
                                const moduleToProtocolModule = {
                                    30: 'PXR-20',
                                    31: 'PXR-20',
                                    32: 'PXR-25',
                                    33: 'PXR-25'
                                };
                                // 定义 module 到 template 的映射
                                const moduleToTemplate = {
                                    30: 'EATONPXR20_TCP',
                                    31: 'EATONPXR20_RTU',
                                    32: 'EATONPXR25_TCP',
                                    33: 'EATONPXR25_RTU'
                                };
                                if (moduleToProtocolModule[device.module]) {
                                    device.protocolModule = moduleToProtocolModule[device.module];
                                }
                                if (moduleToTemplate[device.module]) {
                                    device.template = moduleToTemplate[device.module];
                                }
                            }
                        });
                    }
                });

                // renderBusTree();
                if (busConfigData.commRows.length > 0) {
                    selectCommRow(busConfigData.commRows[0].id);
                }
                showToast('Bus配置加载成功');
                console.log('加载的Bus配置:', busConfigData);
            } else {
                busConfigData = { commRows: [] };
                // renderBusTree();
                document.getElementById('busDetail').innerHTML = renderEmptyDetail();
                showToast('暂无配置，请添加通讯行');
                console.log('加载的Bus配置:', busConfigData);
            }
        })
        .catch(error => {
            console.error('加载配置失败:', error);
            busConfigData = { commRows: [] };
            // renderBusTree();
            document.getElementById('busDetail').innerHTML = renderEmptyDetail();
            showToast('加载配置失败，请检查后端服务', 'error');
            console.log('加载的Bus配置:', busConfigData);
        });
}
// 保存总线配置
function saveBusConfig() {
    if (editingDeviceId) {
        saveInlineEdit();
    }

    syncCurrentRowInputs();

    // 收集端口和间隔
    for (let row of busConfigData.commRows) {

        const portInput = document.getElementById(`rowPort_${row.id}`);
        if (portInput) {
            row.port = parseInt(portInput.value) || 502;
        }

        const internalInput = document.getElementById(`rowInternal_${row.id}`);
        if (internalInput) {

            const internalValue = parseInt(internalInput.value);

            row.internal = (internalValue && internalValue >= 10) ? internalValue : 10;
        }
    }

    if (busConfigData.commRows.length === 0) {
        showToast('请至少添加一个通讯行', 'error');
        return;
    }

    const usedPorts = new Map();

    for (let row of busConfigData.commRows) {

        if (!row.type) {
            showToast('请为所有通讯行选择总线类型', 'error');
            return;
        }

        if (!row.devices || row.devices.length === 0) {
            showToast(`通讯行 "${row.typeName}" 至少需要一个装置`, 'error');
            return;
        }

        for (let device of row.devices) {
            if (!device.name || !device.protocol) {
                showToast('请填写所有装置的必填项', 'error');
                return;
            }
        }

        if (usedPorts.has(row.port)) {

            const rowNum = row.id.replace('row_', '');
            const otherRowNum = usedPorts.get(row.port).replace('row_', '');

            showToast(`第${rowNum}个通讯行与第${otherRowNum}个通讯行端口重复（端口${row.port}）`, 'error');
            return;
        }

        usedPorts.set(row.port, row.id);
    }

    // 重新编号设备
    renumberRowsAndDevices();

    // 获取通讯行参数
    busConfigData.commRows.forEach(row => {

        if (row.type === 'eth0') {
            // IP 已经实时同步到 busConfigData

            // const ipInput = document.getElementById(`rowIp_${row.id}`);
            // row.ip = ipInput ? ipInput.value.trim() : "";

            delete row.extra_params;

        }
        else if (row.type === 'rs485') {

            const baudInput = document.getElementById(`rowBaud_${row.id}`);
            const parityInput = document.getElementById(`rowParity_${row.id}`);
            const dataInput = document.getElementById(`rowData_${row.id}`);
            const stopInput = document.getElementById(`rowStop_${row.id}`);

            const baud = baudInput ? baudInput.value : '9600';
            const parity = parityInput ? parityInput.value : 'e';
            const data = dataInput ? dataInput.value : '8';
            const stop = stopInput ? stopInput.value : '1';

            row.extra_params = `${baud},${parity},${data},${stop}`;

            delete row.ip;
        }

    });

    const url = WEB_FCGI_URL + BUS_SET_CONFIG;

    fetch(url, {
        method: 'POST',
        credentials: 'same-origin',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ config: busConfigData })
    })
        .then(response => response.json())
        .then(data => {

            if (data.success) {
                showToast('总线配置保存成功');
                showImportantReminder('配置已保存，请重启系统使更改生效！！', {
                    onClose: () => {
                        console.log('用户关闭了重启提醒');
                    }
                });
            } else {
                showToast('保存失败: ' + data.message, 'error');
            }

            console.log('保存的Bus配置:', busConfigData);

        })
        .catch(error => {

            console.error('保存配置失败:', error);

            showToast('配置已保存（开发模式）');

            console.log('保存的Bus配置:', busConfigData);

        });

}
// 渲染空详情页
function renderEmptyDetail() {
    return `
        <div class="empty-state">
            <div class="empty-state-icon">📋</div>
            <h4>请选择一个通讯行</h4>
            <p>点击左侧列表进行编辑配置</p>
        </div>
    `;
}

// 渲染树形列表 - 简化版，只显示通讯行
function renderBusTree() {
    const treeContainer = document.getElementById('busTree');

    if (busConfigData.commRows.length === 0) {
        treeContainer.innerHTML = '<div class="empty-tip">暂无通讯行，请点击上方按钮添加</div>';
        return;
    }

    let html = '';
    busConfigData.commRows.forEach((row, index) => {
        const isSelected = selectedRowId === row.id;

        // 从row.id提取编号（格式为"row_X"）
        const rowNum = row.id.replace('row_', '');

        html += `
            <div class="tree-item">
                <div class="tree-row ${isSelected ? 'active' : ''}" onclick="selectCommRow('${row.id}')">
                <span class="tree-label" title="${row.typeName || row.type}">
                ${rowNum}. ${row.typeName === 'ETH0、ETH1(Switch Mode)' ? 'ETH2' : (row.typeName || row.type)}
                </span>
                    <span class="tree-badge">${row.devices.length}</span>
                </div>
            </div>
        `;
    });

    treeContainer.innerHTML = html;
}

// 选择通讯行
function selectCommRow(rowId) {
    syncCurrentRowInputs();
    // 如果正在编辑，先询问是否保存
    if (editingDeviceId) {
        if (!confirm('当前有未保存的更改，是否保存？')) {
            editingDeviceId = null;
        } else {
            saveInlineEdit();
        }
    }

    selectedRowId = rowId;
    // renderBusTree();
    renderCommRowDetail(rowId);

    // 停止旧轮询，启动新轮询（仅当有选中行且页面可见时）
    // if (selectedRowId && document.visibilityState === 'visible') {
    //     startStatusPolling();
    // } else {
    //     stopStatusPolling();
    // }
}

function updateRowIp(rowId, value) {

    const row = busConfigData.commRows.find(r => r.id === rowId);

    if (row) {
        row.ip = value.trim();
    }

}
// 渲染通讯行详情（含装置表格）
function renderCommRowDetail(rowId) {
    const row = busConfigData.commRows.find(r => r.id === rowId);
    if (!row) return;

    const detail = document.getElementById('busDetail');

    // 动态生成基本信息区
    let dynamicFieldHtml = '';
    if (row.type === 'eth0') {
        dynamicFieldHtml = `
                <label style="width: 60px; margin-left: 15px;">IP地址:</label>
                <input type="text" id="rowIp_${row.id}" value="${row.ip || '192.168.1.1'}"  oninput="updateRowIp('${row.id}', this.value)" style="width: 150px;">
            `;
    } else if (row.type === 'rs485') {
        // extra_params 格式：9600,n,8,1
        let baud = '', parity = '', data = '', stop = '';
        if (row.extra_params) {
            const arr = row.extra_params.split(',');
            baud = arr[0] || '';
            parity = arr[1] || '';
            data = arr[2] || '';
            stop = arr[3] || '';
        }

        dynamicFieldHtml = `
        <label style="margin-left:15px;">波特率:</label>
        <select id="rowBaud_${row.id}" class="bus-select">
            <option value="2400" ${baud == '2400' ? 'selected' : ''}>2400</option>
            <option value="4800" ${baud == '4800' ? 'selected' : ''}>4800</option>
            <option value="9600" ${baud == '9600' ? 'selected' : ''}>9600</option>
            <option value="19200" ${baud == '19200' ? 'selected' : ''}>19200</option>
            <option value="38400" ${baud == '38400' ? 'selected' : ''}>38400</option>
            <option value="115200" ${baud == '115200' ? 'selected' : ''}>115200</option>
        </select>

        <label>校验:</label>
        <select id="rowParity_${row.id}" class="bus-select-small">
            <option value="n" ${parity == 'n' ? 'selected' : ''}>None</option>
            <option value="e" ${parity == 'e' ? 'selected' : ''}>Even</option>
            <option value="o" ${parity == 'o' ? 'selected' : ''}>Odd</option>
        </select>

        <label>数据位:</label>
        <select id="rowData_${row.id}" class="bus-select-small">
            <option value="4" ${data == '4' ? 'selected' : ''}>4</option>
            <option value="5" ${data == '5' ? 'selected' : ''}>5</option>
            <option value="6" ${data == '6' ? 'selected' : ''}>6</option>
            <option value="7" ${data == '7' ? 'selected' : ''}>7</option>
            <option value="8" ${data == '8' ? 'selected' : ''}>8</option>
        </select>

        <label>停止位:</label>
        <select id="rowStop_${row.id}" class="bus-select-small">
            <option value="1" ${stop == '1' ? 'selected' : ''}>1</option>
            <option value="2" ${stop == '2' ? 'selected' : ''}>2</option>
        </select>
    `;
    }

    // 计算当前应显示的周期值（若有保存值则用保存值，否则用类型默认值）
    let internalValue = row.internal;
    if (!internalValue || internalValue === 0) {
        internalValue = (row.type === 'eth0') ? 20 : 300;
    }

    //删除通讯行代码被移除，暂时不提供删除通讯行功能，位置在下方<h3>处
    // <div class="bus-detail-actions">
    //     <button onclick="deleteCommRow('${row.id}')" class="btn-small btn-delete">删除通讯行</button>
    // </div>
    detail.innerHTML = `
            <div class="bus-detail-header">
                <h3>通讯行配置</h3>
    

            </div>
        
            <div class="form-section">
                <div class="form-section-title">基本信息</div>
                <div class="form-row">
                    <label>总线类型:</label>
                    <select id="rowType_${row.id}" onchange="updateCommRowType('${row.id}')">
                        <option value="">选择总线类型</option>
                        <option value="eth0" ${row.type === 'eth0' ? 'selected' : ''}>LAN3</option>
                        <option value="rs485" ${row.type === 'rs485' ? 'selected' : ''}>RS485</option>
                    </select>
                    ${dynamicFieldHtml}
                    <label style="width: 50px; margin-left: 15px;">端口:</label>
                    <input type="number" id="rowPort_${row.id}" value="${row.port || 502}" min="1" max="65535" style="width: 50px;">
                    <label style="width: 60px; margin-left: 15px;">采集周期:</label>
                    <input type="number" id="rowInternal_${row.id}" value="${internalValue}" min="10" step="10" style="width: 80px;" disabled>
                    <span style="color: #666; font-size: 13px; margin-left: 5px;">ms</span>
                </div>
            </div>
        
        <div class="form-section">
            <div class="form-section-title" style="display: flex; align-items: center; gap: 12px;">
                <span>装置列表 (${row.devices.length}个)</span>
                <div class="bus-status" id="busStatusIndicator" style="font-size: 16px; display: inline-flex; align-items: center; gap: 8px; margin-left: 20px;">
                   <span class="status-light" id="statusLight" style="width: 14px; height: 14px;"></span>
                   <span id="statusText" style="font-weight: normal;">--</span>
                </div>
        </div>
            <table class="device-list-table">
                <thead>
                    <tr>
                        <th width="75">行号</th>
                        <th width="125">地址</th>
                        <th width="125">装置名称</th>
                        <th width="125">装置类型</th>
                        <th width="100">操作</th>
                    </tr>
                </thead>
                <tbody>
                    ${row.devices.map((device) => renderDeviceRow(row.id, device)).join('')}
                </tbody>
            </table>
            
            </div>
            `;
}
//添加装置按钮被移除，暂时不提供批量添加装置功能，代码位置在上方</table>下
// <div class="batch-actions">
//     <button onclick="addDevice('${row.id}')" class="btn-small btn-add">+ 添加装置</button>
// </div>
// 渲染装置行（普通模式或编辑模式）
function renderDeviceRow(rowId, device) {
    if (editingDeviceId === device.id) {
        // 编辑模式：只显示行号、地址、名称、装置类型
        return `
        <tr class="editing-row">
            <td><input type="number" id="edit_order_${device.id}" value="${device.order}" style="width:45px;"></td>
            <td><input type="text" id="edit_address_${device.id}" value="${device.address || ''}" placeholder="地址" style="width:90px;"></td>
            <td><input type="text" id="edit_name_${device.id}" value="${device.name || ''}" placeholder="装置名称" style="width:140px;"></td>
            <td>
                <select id="edit_protocolModule_${device.id}" style="width:110px;">
                    <option value="PXR-20" ${device.protocolModule === 'PXR-20' ? 'selected' : ''}>PXR-20</option>
                    <option value="PXR-25" ${device.protocolModule === 'PXR-25' ? 'selected' : ''}>PXR-25</option>
                </select>
            </td>
            <td>
                <button onclick="saveInlineEdit()" class="btn-small btn-add" title="保存">✓</button>
                <button onclick="cancelInlineEdit()" class="btn-small btn-secondary" title="取消">✕</button>
            </td>
        </tr>
        `;
    } else {
        // 普通模式：显示行号、地址、名称、装置类型
        const moduleToProtocolModule = {
            30: 'PXR-20', 31: 'PXR-20',
            32: 'PXR-25', 33: 'PXR-25'
        };
        const protocolModuleDisplay = (device.module !== undefined && moduleToProtocolModule[device.module])
            ? moduleToProtocolModule[device.module]
            : (device.protocolModule || '-');

        return `
        <tr>
            <td>${device.order}</td>
            <td>${device.address || '-'}</td>
            <td>${device.name}</td>
            <td>${protocolModuleDisplay}</td>
            <td>
                <button onclick="startEditDevice('${rowId}', '${device.id}')" class="btn-small btn-secondary" title="编辑">编辑</button>
            </td>
        </tr>
        `;
    }
}
//移除装置行的删除按钮，暂时不提供删除装置功能，位置在上方<td>内
// <button onclick="deleteDevice('${rowId}', '${device.id}')" class="btn-small btn-delete" title="删除">删除</button>

// 更新模板选项（根据协议类型）
function updateTemplateOptions(deviceId) {
    const protocolSelect = document.getElementById(`edit_protocol_${deviceId}`);
    const templateSelect = document.getElementById(`edit_template_${deviceId}`);
    const protocol = protocolSelect.value;

    // 清空模板选项
    templateSelect.innerHTML = '<option value="">选择模板</option>';

    // 根据协议添加对应的模板选项
    const templates = TEMPLATE_OPTIONS[protocol] || [];
    templates.forEach(opt => {
        const option = document.createElement('option');
        option.value = opt.value;
        option.textContent = opt.text;
        templateSelect.appendChild(option);
    });

    // 自动选择对应的模块
    const moduleSelect = document.getElementById(`edit_protocolModule_${deviceId}`);
    if (protocol === 'modbustcp') {
        moduleSelect.value = 'PXR-25';
    } else if (protocol === 'modbusrtu') {
        // 保持当前选择或默认选择PXR-25
        if (!moduleSelect.value) {
            moduleSelect.value = 'PXR-25';
        }
    }
}

// 开始编辑装置
function startEditDevice(rowId, deviceId) {
    // 如果正在编辑其他行，先保存
    if (editingDeviceId && editingDeviceId !== deviceId) {
        saveInlineEdit();
    }

    editingDeviceId = deviceId;
    isNewDevice = false; // 编辑现有装置，不是新添加的
    renderCommRowDetail(rowId);
}

// 验证装置名称是否重复（全局唯一）
function isDeviceNameDuplicate(name, excludeDeviceId) {
    for (let row of busConfigData.commRows) {
        for (let device of row.devices) {
            if (device.id !== excludeDeviceId && device.name === name) {
                return true;
            }
        }
    }
    return false;
}

// 验证同一通讯行内地址是否重复
function isDeviceAddressDuplicate(address, rowId, excludeDeviceId) {
    const row = busConfigData.commRows.find(r => r.id === rowId);
    if (!row) return false;

    for (let device of row.devices) {
        if (device.id !== excludeDeviceId && device.address === address) {
            return true;
        }
    }
    return false;
}

// 保存内联编辑
function saveInlineEdit() {
    if (!editingDeviceId || !selectedRowId) return;

    const row = busConfigData.commRows.find(r => r.id === selectedRowId);
    const device = row.devices.find(d => d.id === editingDeviceId);
    if (!device) return;

    const newName = document.getElementById(`edit_name_${editingDeviceId}`).value.trim();
    const newAddress = document.getElementById(`edit_address_${editingDeviceId}`).value.trim();

    // 验证
    if (!newName) {
        showToast('装置名称不能为空', 'error');
        return;
    }
    if (isDeviceNameDuplicate(newName, editingDeviceId)) {
        showToast('装置名称已存在，请使用唯一的名称', 'error');
        return;
    }
    if (!newAddress || newAddress.trim() === '') {
        showToast('装置地址不能为空', 'error');
        return;
    }
    if (newAddress && isDeviceAddressDuplicate(newAddress, selectedRowId, editingDeviceId)) {
        showToast('该通讯行内已存在相同地址的装置', 'error');
        return;
    }

    device.order = parseInt(document.getElementById(`edit_order_${editingDeviceId}`).value) || 1;
    device.address = newAddress;
    device.name = newName;

    // 保存装置类型（协议模块）
    const moduleSelect = document.getElementById(`edit_protocolModule_${editingDeviceId}`);
    if (moduleSelect) {
        device.protocolModule = moduleSelect.value;
        // 根据 protocolModule 自动设置 template 和 module（保证后端数据一致性）
        if (device.protocolModule === 'PXR-20') {
            device.template = (row.type === 'eth0') ? 'EATONPXR20_TCP' : 'EATONPXR20_RTU';
            device.module = (row.type === 'eth0') ? 30 : 31;
        } else { // PXR-25
            device.template = (row.type === 'eth0') ? 'EATONPXR25_TCP' : 'EATONPXR25_RTU';
            device.module = (row.type === 'eth0') ? 32 : 33;
        }
    }

    // 保证协议字段存在
    if (!device.protocol) {
        device.protocol = 'modbusmaster';
        device.protocolName = 'Modbus Master';
    }

    // 重新排序
    row.devices.sort((a, b) => a.order - b.order);

    editingDeviceId = null;
    isNewDevice = false;
    renderCommRowDetail(selectedRowId);
    // renderBusTree();
    showToast('装置信息已保存');
}

// 取消内联编辑
function cancelInlineEdit() {
    // 如果是新添加的装置且点击取消，则删除该装置
    if (isNewDevice && editingDeviceId && selectedRowId) {
        const row = busConfigData.commRows.find(r => r.id === selectedRowId);
        if (row) {
            row.devices = row.devices.filter(d => d.id !== editingDeviceId);
        }
    }

    editingDeviceId = null;
    isNewDevice = false;
    if (selectedRowId) {
        renderCommRowDetail(selectedRowId);
        // renderBusTree();
    }
}
// 同步当前通讯行输入框到数据模型
function syncCurrentRowInputs() {

    if (!selectedRowId) return;

    const row = busConfigData.commRows.find(r => r.id === selectedRowId);
    if (!row) return;

    // 端口
    const portInput = document.getElementById(`rowPort_${row.id}`);
    if (portInput) {
        row.port = parseInt(portInput.value) || row.port;
    }

    // 采集周期
    const internalInput = document.getElementById(`rowInternal_${row.id}`);
    if (internalInput) {

        const val = parseInt(internalInput.value);
        row.internal = (val && val >= 10) ? val : 10;

    }

    if (row.type === 'eth0') {

        const ipInput = document.getElementById(`rowIp_${row.id}`);
        if (ipInput) {
            row.ip = ipInput.value.trim();
        }

    }
    else if (row.type === 'rs485') {

        const baud = document.getElementById(`rowBaud_${row.id}`);
        const parity = document.getElementById(`rowParity_${row.id}`);
        const data = document.getElementById(`rowData_${row.id}`);
        const stop = document.getElementById(`rowStop_${row.id}`);

        if (baud && parity && data && stop) {

            row.extra_params =
                `${baud.value},${parity.value},${data.value},${stop.value}`;

        }
    }

}
// 添加通讯行
function addCommRow() {
    // 使用顺序号：数组长度+1
    const newId = busConfigData.commRows.length + 1;
    const newRow = {
        id: 'row_' + newId,
        type: '',
        typeName: '',
        port: 502,
        internal: 20,          // 默认网口周期
        internalEth: 20,       // 网口独立周期
        internal485: 300,      // RS485独立周期
        devices: []
    };

    busConfigData.commRows.push(newRow);
    // renderBusTree();
    selectCommRow(newRow.id);
    showToast('通讯行已添加，请选择总线类型');
}

// 添加装置到当前选中的通讯行
function addDeviceToCurrentRow() {
    if (!selectedRowId) {
        if (busConfigData.commRows.length === 0) {
            showToast('请先添加通讯行', 'error');
            return;
        }
        // 默认添加到第一个通讯行
        addDevice(busConfigData.commRows[0].id);
    } else {
        addDevice(selectedRowId);
    }
}

// 添加装置
function addDevice(rowId) {
    syncCurrentRowInputs();

    const row = busConfigData.commRows.find(r => r.id === rowId);
    if (!row) return;

    // 如果正在编辑，先保存
    if (editingDeviceId) {
        saveInlineEdit();
    }

    // 获取通讯行序号
    const rowIndex = busConfigData.commRows.findIndex(r => r.id === rowId) + 1;
    const maxOrder = row.devices.reduce((max, d) => Math.max(max, d.order || 0), 0);
    const newOrder = maxOrder + 1;

    // 生成默认名称：通讯行序号_总线类型_装置序号#
    const busType = row.type || 'unknown';
    const busTypePrefix = busType === 'rs485' || busType === '485' ? 'rs485' : 'eth0';
    const defaultName = `${rowIndex}_${busTypePrefix}_${newOrder}#`;
    const defaultProtocol = busType === 'rs485' || busType === '485' ? 'modbusmaster' : 'modbusmaster';
    const defaultProtocolName = 'Modbus Master';
    const defaultModule = busType === 'rs485' || busType === '485' ? 'PXR-20' : 'PXR-25';
    const defaultTemplate = busType === 'rs485' || busType === '485' ? 'EATONPXR20_RTU' : 'EATONPXR25_TCP';

    // 计算全局装置序号（从ID字符串中提取数字）
    let globalDeviceIndex = 1;
    for (const r of busConfigData.commRows) {
        for (const d of r.devices) {
            // 从 'dev_X' 格式中提取数字
            const devNum = parseInt(d.id.toString().replace('dev_', ''));
            if (!isNaN(devNum) && devNum >= globalDeviceIndex) {
                globalDeviceIndex = devNum + 1;
            }
        }
    }

    // 模板到 module 的映射
    const templateToModule = {
        'EATONPXR20_TCP': 30,
        'EATONPXR20_RTU': 31,
        'EATONPXR25_TCP': 32,
        'EATONPXR25_RTU': 33
    };

    const newDevice = {
        id: 'dev_' + globalDeviceIndex,
        order: newOrder,
        address: '',
        name: defaultName,
        protocol: defaultProtocol,
        protocolName: defaultProtocolName,
        protocolModule: defaultModule,
        template: defaultTemplate,
        module: templateToModule[defaultTemplate] || 32  // 根据默认模板设置 module
    };

    row.devices.push(newDevice);
    editingDeviceId = newDevice.id; // 自动进入编辑模式
    isNewDevice = true; // 标记为新添加的装置
    renderCommRowDetail(rowId);
    // renderBusTree();
    showToast('装置已添加，请修改详细信息');
}

// 更新通讯行类型
function updateCommRowType(rowId) {
    const row = busConfigData.commRows.find(r => r.id === rowId);
    if (!row) return;

    // 1. 同步当前输入框的值到 row（避免丢失未保存的修改）
    syncCurrentRowInputs();

    const select = document.getElementById(`rowType_${rowId}`);
    const newType = select.value;
    const newTypeName = select.options[select.selectedIndex].text;

    // 保存当前 internal 到对应类型（基于切换前的类型）
    if (row.type === 'eth0') {
        row.internalEth = row.internal;
    } else if (row.type === 'rs485') {
        row.internal485 = row.internal;
    }

    // 2. 设置新类型（不再删除原有属性）
    row.type = newType;
    row.typeName = newTypeName;

    // 根据新类型设置 internal
    if (newType === 'eth0') {
        // 如果之前保存过 internalEth，则使用，否则使用默认值20
        row.internal = row.internalEth !== undefined ? row.internalEth : 20;
    } else if (newType === 'rs485') {
        row.internal = row.internal485 !== undefined ? row.internal485 : 300;
    }

    // 处理端口等默认值（不覆盖已有值）
    if (newType === 'eth0') {
        if (row.port === undefined) row.port = 502;
        if (!row.ip) row.ip = '';
    } else if (newType === 'rs485') {
        if (row.port === undefined) row.port = 1;
        if (!row.extra_params) row.extra_params = '9600,e,8,1';
    }


    // 4. 更新界面
    // renderBusTree();
    renderCommRowDetail(rowId);
    showToast('通讯行类型已更新');
}

// 删除通讯行
function deleteCommRow(rowId) {
    if (!confirm('确定要删除此通讯行及其所有装置吗？')) {
        return;
    }

    busConfigData.commRows = busConfigData.commRows.filter(r => r.id !== rowId);

    if (selectedRowId === rowId) {
        selectedRowId = null;
        editingDeviceId = null;
        document.getElementById('busDetail').innerHTML = renderEmptyDetail();
    }

    // 先重新编号，再渲染
    renumberRowsAndDevices();
    // renderBusTree();
    showToast('通讯行已删除');
}
// 重新编号通讯行和装置（全局设备行号连续）
function renumberRowsAndDevices() {

    let globalDeviceOrder = 0;

    busConfigData.commRows.forEach((row) => {
        if (!row.devices) row.devices = [];
        // 按 order 排序
        row.devices.sort((a, b) => (a.order || 0) - (b.order || 0));

        row.devices.forEach((device, devIndex) => {

            // 全局装置行号
            device.order = globalDeviceOrder++;

            // 保证设备ID存在
            if (!device.id) {
                device.id = 'dev_' + Date.now() + '_' + devIndex;
            }

            // 只在 name 为空时生成默认名称
            if (!device.name || device.name.trim() === '') {

                const busType = row.type === 'rs485' ? 'rs485' : 'eth0';

                device.name = `${row.id.replace('row_', '')}_${busType}_${devIndex + 1}#`;
            }

        });

    });

}
// 删除装置
function deleteDevice(rowId, deviceId) {
    syncCurrentRowInputs();

    if (!confirm('确定要删除此装置吗？')) {
        return;
    }

    const row = busConfigData.commRows.find(r => r.id === rowId);
    row.devices = row.devices.filter(d => d.id !== deviceId);

    if (editingDeviceId === deviceId) {
        editingDeviceId = null;
    }

    renumberRowsAndDevices();
    renderCommRowDetail(rowId);
    // renderBusTree();
    showToast('装置已删除');
}

// 页面可见性变化监听
// document.addEventListener('visibilitychange', function () {
//     if (document.visibilityState === 'visible') {
//         window.startBusStatusPollingIfNeeded();
//     } else {
//         window.stopBusStatusPollingIfNeeded();
//     }
// });

// 页面卸载时停止轮询
window.addEventListener('beforeunload', function () {
    stopStatusPolling();
});

// 停止状态轮询（供外部调用）
window.stopBusStatusPollingIfNeeded = function () {
    stopStatusPolling();
};

// 启动状态轮询（供外部调用，仅在当前卡片激活时启动）
window.startBusStatusPollingIfNeeded = function () {
    // 仅当当前选中的通讯行存在且页面可见时启动
    if (selectedRowId && document.visibilityState === 'visible') {
        startStatusPolling();
    }
};