# 🚀 ROS2 哨兵機器人完整使用流程

## 📋 系統狀態

✅ **所有包編譯成功**  
✅ **消息通訊鏈路完整**  
✅ **測試模式開關已添加**

---

## 🎯 兩種工作模式

### 模式 1: 測試模式（看到就打）

**適用場景**: 快速功能測試，無需決策模塊

**配置**: [`src/detector/config/auto_aim_config.yaml`](../../src/detector/config/auto_aim_config.yaml:109)
```yaml
test_mode:
  enable: true  # 設為 true 啟用測試模式
```

**啟動方式**:
```bash
cd ~/ros2_ly_ws_sentary
source install/setup.bash

# 選擇一種模式啟動
ros2 launch detector auto_aim.launch.py    # 自瞄模式
ros2 launch detector outpost.launch.py     # 前哨模式
ros2 launch detector buff.launch.py        # 能量機關模式
```

**工作流程**:
```
相機 → detector/outpost/buff → simple_bridge_node → gimbal_driver → 下位機
                                      ↓
                              自動開火（看到就打）
```

**特點**:
- ✅ 檢測到有效目標立即開火
- ✅ 無需 behavior_tree 決策
- ✅ 適合快速測試瞄準和開火功能
- ⚠️ 沒有智能決策（不考慮血量、彈藥等）

---

### 模式 2: 正常模式（智能決策）

**適用場景**: 正式比賽，需要完整決策邏輯

**配置**: [`src/detector/config/auto_aim_config.yaml`](../../src/detector/config/auto_aim_config.yaml:109)
```yaml
test_mode:
  enable: false  # 設為 false 等待 behavior_tree
```

**啟動方式**:
```bash
cd ~/ros2_ly_ws_sentary
source install/setup.bash

# 終端 1: 啟動感知模塊
ros2 launch detector auto_aim.launch.py

# 終端 2: 啟動決策模塊
ros2 launch behavior_tree behavior_tree.launch.py
```

**工作流程**:
```
相機 → detector → tracker → predictor → behavior_tree → gimbal_driver → 下位機
                                              ↓
                                    智能決策（考慮遊戲狀態）
```

**特點**:
- ✅ 根據血量、彈藥、遊戲狀態決策
- ✅ 支持目標優先級選擇
- ✅ 支持模式自動切換（自瞄/前哨/能量機關）
- ⚠️ 需要 behavior_tree 決策模塊完整實現

---

## 🔧 上車前配置檢查

### 必須修改的參數

**文件**: [`src/detector/config/auto_aim_config.yaml`](../../src/detector/config/auto_aim_config.yaml)

#### 1. 圖像源（第 45 行）
```yaml
# 測試時（使用視頻）
"detector_config/use_video": true

# 上車時（使用相機）
"detector_config/use_video": false  # ← 必須改為 false
```

#### 2. 測試模式（第 110 行）
```yaml
# 測試時（看到就打）
test_mode:
  enable: true

# 正式比賽（智能決策）
test_mode:
  enable: false  # ← 正式比賽改為 false
```

#### 3. 虛擬設備（第 110 行）
```yaml
# 測試時（無下位機）
io_config:
  use_virtual_device: true

# 上車時（連接下位機）
io_config:
  use_virtual_device: false  # ← 必須改為 false
```

---

## 🎮 控制下位機的機制

### 開火指令流程

**simple_bridge_node** ([`src/detector/simple_bridge_node.cpp:43`](../../src/detector/simple_bridge_node.cpp:43)):
```cpp
// 發射指令
std_msgs::msg::UInt8 fire;
fire.data = 1;  // 1 = 開火
fire_pub_->publish(fire);  // 發布到 /ly/control/firecode
```

**gimbal_driver** ([`src/gimbal_driver/main.cpp:90`](../../src/gimbal_driver/main.cpp:90)):
```cpp
// 訂閱開火指令
GenSub<ly_control_firecode>([](GimbalControlData& g, const std_msgs::msg::UInt8& m) {
    *reinterpret_cast<std::uint8_t*>(&g.FireCode) = m.data;
});

// 發送到下位機 (line 358)
Device.Write(data);  // 通過串口發送
```

### 角度控制流程

**simple_bridge_node** ([`src/detector/simple_bridge_node.cpp:36`](../../src/detector/simple_bridge_node.cpp:36)):
```cpp
// 角度指令
gimbal_driver::msg::GimbalAngles angles;
angles.yaw = msg->yaw;
angles.pitch = msg->pitch;
angles_pub_->publish(angles);  // 發布到 /ly/control/angles
```

**gimbal_driver** ([`src/gimbal_driver/main.cpp:84`](../../src/gimbal_driver/main.cpp:84)):
```cpp
// 訂閱角度指令
GenSub<ly_control_angles>([](GimbalControlData& g, const gimbal_driver::msg::GimbalAngles& m) {
    g.GimbalAngles.Yaw = static_cast<float>(m.yaw);
    g.GimbalAngles.Pitch = static_cast<float>(m.pitch);
});

// 發送到下位機
Device.Write(data);  // 通過串口發送
```

**結論**: ✅ **是的，直接控制下位機的雲台和開火**

---

## 📊 消息通訊鏈路

### 自瞄模式完整鏈路

```
相機硬件
  ↓
gimbal_driver (讀取圖像 + 雲台角度)
  ↓ 發布: /ly/gimbal/angles
detector (檢測裝甲板)
  ↓ 發布: /ly/detector/armors
tracker_solver (追蹤)
  ↓ 發布: /ly/tracker/results
predictor (預測 + 彈道解算)
  ↓ 發布: /ly/predictor/target
  
【測試模式】
simple_bridge_node (test_mode.enable: true)
  ↓ 發布: /ly/control/angles
  ↓ 發布: /ly/control/firecode
  
【正常模式】
behavior_tree (test_mode.enable: false)
  ↓ 發布: /ly/control/angles
  ↓ 發布: /ly/control/firecode
  
gimbal_driver (接收控制指令)
  ↓ 串口通訊
下位機 (執行雲台控制 + 開火)
```

### 前哨模式完整鏈路

```
相機硬件
  ↓
gimbal_driver
  ↓
outpost_hitter (檢測 + 預測 + 解算)
  ↓ 發布: /ly/outpost/target
simple_bridge_node / behavior_tree
  ↓ 發布: /ly/control/angles + /ly/control/firecode
gimbal_driver
  ↓
下位機
```

### 能量機關模式完整鏈路

```
相機硬件
  ↓
gimbal_driver
  ↓
buff_hitter (檢測 + 預測 + 解算)
  ↓ 發布: /ly/buff/target
simple_bridge_node / behavior_tree
  ↓ 發布: /ly/control/angles + /ly/control/firecode
gimbal_driver
  ↓
下位機
```

---

## 🔄 模式切換方法

### 從測試模式切換到正常模式

1. **修改配置文件**:
```bash
vim ~/ros2_ly_ws_sentary/src/detector/config/auto_aim_config.yaml
```

2. **修改參數**:
```yaml
test_mode:
  enable: false  # 改為 false
```

3. **重新啟動**:
```bash
# 不需要重新編譯，直接重啟即可
ros2 launch detector auto_aim.launch.py
```

4. **啟動決策模塊**:
```bash
# 新終端
ros2 launch behavior_tree behavior_tree.launch.py
```

### 從正常模式切換到測試模式

1. **停止 behavior_tree**:
```bash
# 在 behavior_tree 終端按 Ctrl+C
```

2. **修改配置**:
```yaml
test_mode:
  enable: true  # 改為 true
```

3. **重新啟動感知模塊**:
```bash
ros2 launch detector auto_aim.launch.py
```

---

## ✅ 編譯和啟動命令

### 首次編譯

```bash
cd ~/ros2_ly_ws_sentary

# 編譯所有包
colcon build

# 或單獨編譯
colcon build --packages-select detector gimbal_driver tracker_solver predictor outpost_hitter buff_hitter

# 加載環境
source install/setup.bash
```

### 修改代碼後重新編譯

```bash
cd ~/ros2_ly_ws_sentary

# 只編譯修改的包（例如 detector）
colcon build --packages-select detector

# 加載環境
source install/setup.bash
```

### 啟動系統

**測試模式**:
```bash
# 確保 test_mode.enable: true
ros2 launch detector auto_aim.launch.py
```

**正常模式**:
```bash
# 確保 test_mode.enable: false

# 終端 1
ros2 launch detector auto_aim.launch.py

# 終端 2
ros2 launch behavior_tree behavior_tree.launch.py
```

---

## 🐛 調試命令

### 查看 Topic 列表
```bash
ros2 topic list
```

### 查看消息內容
```bash
# 查看檢測結果
ros2 topic echo /ly/detector/armors

# 查看預測目標
ros2 topic echo /ly/predictor/target

# 查看控制指令
ros2 topic echo /ly/control/angles
ros2 topic echo /ly/control/firecode
```

### 查看節點狀態
```bash
ros2 node list
ros2 node info /simple_bridge
```

### 查看參數
```bash
ros2 param list /simple_bridge
ros2 param get /simple_bridge test_mode.enable
```

---

## 📚 相關文檔

- [`消息通訊鏈路文檔.md`](消息通訊鏈路文檔.md) - 詳細的消息流程
- [`上車前最終檢查清單.md`](上車前最終檢查清單.md) - 部署前檢查項目
- [`系統行為說明_更新.md`](系統行為說明_更新.md) - 系統行為詳解
- [`TEST_GUIDE.md`](TEST_GUIDE.md) - 測試指南
- [`CONFIG_SETUP_GUIDE.md`](CONFIG_SETUP_GUIDE.md) - 配置指南

---

## ⚠️ 重要提醒

1. **上車前必須修改**:
   - `use_video: false` (使用相機)
   - `use_virtual_device: false` (連接下位機)
   - `test_mode.enable: false` (使用決策模塊，如果需要)

2. **測試時注意**:
   - `test_mode: true` 會立即開火，注意安全
   - 確保下位機已連接（或使用虛擬設備測試）

3. **決策模塊**:
   - 正式比賽需要啟動 behavior_tree
   - behavior_tree 需要完整實現才能正常工作

4. **串口權限**:
   ```bash
   sudo chmod 666 /dev/ttyACM0
   ```
