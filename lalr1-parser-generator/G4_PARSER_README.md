# ANTLR4 (.g4) to LALR(1) Parser Generator

这个修改版本的LALR(1)解析器生成器现在支持直接解析ANTLR4的.g4语法文件。

## 新功能

### G4语法解析支持
- 直接解析ANTLR4 .g4语法文件
- 自动转换EBNF语法糖到标准BNF
- 支持词法规则和语法规则的分离处理
- 检测语法是否为LALR(1)兼容

### 新增的命令行选项

```bash
# 基本用法 - 处理G4文件
./lalr1_tool grammar.g4

# 分析语法属性
./lalr1_tool --analyze grammar.g4

# 转换为BNF格式
./lalr1_tool --convert-bnf output.bnf grammar.g4

# 显示详细信息
./lalr1_tool --verbose --show-states --show-table grammar.g4

# 显示FIRST/FOLLOW集合
./lalr1_tool --show-sets grammar.g4
```

## 构建和安装

```bash
cd lalr1-parser-generator
mkdir build && cd build
cmake ..
make
```

## 示例用法

### 1. 分析JSON语法
```bash
./lalr1_tool examples/SimpleJSON.g4
```

### 2. 转换G4到BNF
```bash
./lalr1_tool --convert-bnf json.bnf examples/SimpleJSON.g4
```

### 3. 详细分析
```bash
./lalr1_tool --analyze --verbose --show-all examples/SimpleJSON.g4
```

## G4语法支持状态

### 当前支持的特性
- ✅ 基本产生式规则 (`rule: alternative1 | alternative2;`)
- ✅ 词法规则 (`TOKEN: pattern;`)
- ✅ 字符串字面量 (`'literal'`)
- ✅ 注释移除 (`//` 和 `/* */`)
- ✅ 基本EBNF转换

### 部分支持的特性
- 🔶 复杂EBNF语法糖 (`*`, `+`, `?`, `()`)
- 🔶 字符类 (`[a-z]`, `[^abc]`)
- 🔶 Fragment规则
- 🔶 Skip规则 (`-> skip`)

### 计划中的特性
- ⏳ 完整的正则表达式支持
- ⏳ 语义动作处理
- ⏳ 优先级和结合性
- ⏳ 模式匹配和重写

## 文件结构

```
lalr1-parser-generator/
├── g4_parser.h          # G4解析器接口
├── g4_parser.cpp        # G4解析器实现
├── lalr1_tool.cpp       # 增强的命令行工具
├── examples/
│   └── SimpleJSON.g4    # 示例JSON语法
└── ...                  # 其他核心文件
```

## 已知限制

1. **EBNF支持**: 目前对复杂的EBNF语法糖支持有限，可能需要手动转换
2. **正则表达式**: 词法规则中的复杂正则表达式可能需要简化
3. **语义动作**: 不支持ANTLR4的语义动作和属性
4. **错误恢复**: 基本的错误报告，错误恢复功能有限

## 故障排除

### 常见问题

**Q: 语法解析失败**
A: 检查语法文件格式，确保遵循ANTLR4标准语法

**Q: 不是LALR(1)语法**
A: 使用`--analyze`选项查看冲突详情，可能需要重构语法

**Q: EBNF转换错误**
A: 对于复杂的EBNF，可能需要手动展开为标准BNF格式

### 调试技巧

```bash
# 启用详细输出
./lalr1_tool --verbose grammar.g4

# 查看转换的BNF
./lalr1_tool --convert-bnf debug.bnf grammar.g4

# 分析语法结构
./lalr1_tool --analyze --show-sets grammar.g4
```

## 贡献

欢迎提交问题报告和改进建议！

## 许可证

本项目遵循与原grammars-v4项目相同的许可证。
