# Schema设计

```SQL
-- 设备表
CREATE TABLE IF NOT EXISTS equipment (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,-- 自动增加
    asset_no      TEXT    NOT NULL UNIQUE,          -- 资产编号，唯一
    name          TEXT    NOT NULL,                 -- 设备名称
    model         TEXT,                             -- 型号
    brand         TEXT,                             -- 品牌
    purchase_date TEXT,                             -- 购置日期 yyyy-MM-dd
    price         REAL    DEFAULT 0,                -- 单价
    location      TEXT,                             -- 存放位置
    status        TEXT    NOT NULL DEFAULT 'available',  -- available/borrowed/pending_repair/repairing/maintaining/scrapped
    keeper        TEXT,                             -- 责任人
    remark        TEXT
);

-- 借还记录表
CREATE TABLE IF NOT EXISTS borrow_record (
    id                   INTEGER PRIMARY KEY AUTOINCREMENT,
    equipment_id         INTEGER NOT NULL,
    borrower             TEXT    NOT NULL,          -- 借用人
    borrow_time          TEXT    NOT NULL,          -- 借出时间
    expected_return_time TEXT,                      -- 预计归还时间
    actual_return_time   TEXT,                      -- 实际归还时间，NULL 表示未还
    status               TEXT    NOT NULL DEFAULT 'borrowed',  -- borrowed/returned
    remark               TEXT,
    FOREIGN KEY (equipment_id) REFERENCES equipment(id) ON DELETE RESTRICT
);

-- 维保记录表
CREATE TABLE IF NOT EXISTS maintenance_record (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    equipment_id     INTEGER NOT NULL,
    maintenance_date TEXT    NOT NULL,
    type             TEXT    NOT NULL,              -- repair/maintain
    status           TEXT    NOT NULL DEFAULT 'pending', -- pending/in_progress/completed
    description      TEXT,
    cost             REAL    DEFAULT 0,
    operator         TEXT,                          -- 经办人
    FOREIGN KEY (equipment_id) REFERENCES equipment(id) ON DELETE CASCADE
);

-- 索引：检索和联表查询会用到
CREATE INDEX IF NOT EXISTS idx_equipment_asset_no      ON equipment(asset_no);
CREATE INDEX IF NOT EXISTS idx_borrow_equipment        ON borrow_record(equipment_id);
CREATE INDEX IF NOT EXISTS idx_maintenance_equipment   ON maintenance_record(equipment_id);
```

几个 **必须全员达成一致的设计决策** ：

1. **状态值用英文常量还是中文** ：统一存英文（`available`/`borrowed`/`pending_repair`/`repairing`/`maintaining`/`scrapped`），界面显示通过 `common/enums.h` 转换。代码层统一使用 `LabAsset::EquipmentStatus`。
2. **设备“借出”时 `status` 怎么变** ：约定为借出时 `equipment.status` 从 `available` 改为 `borrowed`，归还时改回 `available`。这条规则 B 和 A 必须一致，否则统计页（C）的数字会乱。
3. **删除设备时关联记录怎么办** ：`borrow_record` 用 `ON DELETE RESTRICT`（有借还记录就不让删设备），`maintenance_record` 用 `ON DELETE CASCADE`（删设备连带删维保记录）。这是业务规则，要提前说清楚。
4. **日期格式统一** ：全组统一 `"yyyy-MM-dd"`（日期）和 `"yyyy-MM-dd HH:mm:ss"`（时间戳），用 `QDateTime::toString("yyyy-MM-dd HH:mm:ss")` 生成。
5. **维修保养状态联动** ：新增维修任务时，设备当前必须为 `available`，待维修任务将设备置为 `pending_repair`；直接开始的维修任务将设备置为 `repairing`。保养任务创建后直接进入 `in_progress`，设备置为 `maintaining`。未完成任务修改只允许修改日期、说明、费用和经办人；完成后设备恢复 `available`。

6. **旧数据迁移** ：程序启动时会将旧值 `under_maintenance` 迁移为 `maintaining`，将 `damaged` 迁移为 `pending_repair`。旧维保记录迁移后视为 `completed`；处于维修流程但缺少任务的设备会自动补齐当前任务。

7. **维保任务状态** ：`maintenance_record.status` 使用 `pending`、`in_progress`、`completed`。任务状态通过开始、完成操作流转，普通编辑不能任意修改。

## 验证

每人装一个  **DB Browser for SQLite** （免费、轻量）。程序运行一次后，用它打开 `lab_asset.db`，能看到三张表和索引就说明建库成功。后续调试时也能直接看数据，比在代码里打日志快得多。
