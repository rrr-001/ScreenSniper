#!/bin/bash

echo "🔨 开始编译 ScreenSniper..."

# 检查并安装翻译文件
if [ -f "package.json" ] && command -v node &> /dev/null; then
    echo "🌐 检查翻译文件..."
    if [ ! -d "node_modules" ]; then
        echo "📦 首次构建，正在安装依赖..."
        npm install
    fi
    npm run install-locales
    echo ""
fi

# 创建构建目录
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# 运行 qmake
echo "📝 运行 qmake..."
qmake ../ScreenSniper.pro

# 编译
echo "🔧 编译项目..."
make

if [ $? -eq 0 ]; then
    echo "✅ 编译成功！"
    echo ""
    echo "运行程序："
    if [ -d "ScreenSniper.app" ]; then
        echo "  ./build/ScreenSniper.app/Contents/MacOS/ScreenSniper"
    else
        echo "  ./build/ScreenSniper"
    fi
else
    echo "❌ 编译失败，请检查错误信息"
    exit 1
fi
