#!/bin/bash

echo "🚀 启动 ScreenSniper..."

# 检查翻译文件是否存在
if [ ! -f "locales/zh.json" ] || [ ! -f "locales/en.json" ]; then
    echo "⚠️  翻译文件缺失，正在从 npm 拉取..."
    if command -v node &> /dev/null; then
        npm install --silent
        npm run install-locales
        echo "✅ 翻译文件已更新"
    else
        echo "❌ 未安装 Node.js，无法自动拉取翻译文件"
        echo "💡 请安装 Node.js 或手动运行: npm run install-locales"
    fi
    echo ""
fi
# 检查是否已编译
if [ ! -f "build/ScreenSniper.app/Contents/MacOS/ScreenSniper" ]; then
    echo "❌ 未找到可执行文件，请先运行 ./build.sh 编译项目"
    exit 1
fi

# 运行应用
open build/ScreenSniper.app
