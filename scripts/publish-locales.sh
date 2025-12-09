#!/bin/bash

# ScreenSniper 翻译文件发布脚本
# 用于快速发布翻译包到 npm

set -e

echo "📦 ScreenSniper Locales 发布工具"
echo "================================"
echo ""

cd node_modules/@screensniper/locales

# 检查是否登录
echo "🔍 检查 npm 登录状态..."
if ! npm whoami &> /dev/null; then
    echo "❌ 未登录到 npm"
    echo ""
    echo "请先运行以下命令登录："
    echo "  npm login"
    echo ""
    echo "如果没有账号，请访问: https://www.npmjs.com/signup"
    exit 1
fi

CURRENT_USER=$(npm whoami)
echo "✅ 已登录为: $CURRENT_USER"
echo ""

# 显示当前版本
CURRENT_VERSION=$(node -p "require('./package.json').version")
echo "📌 当前版本: $CURRENT_VERSION"
echo ""

# 询问版本类型
echo "请选择版本更新类型："
echo "  1) patch (1.0.0 -> 1.0.1) - 修复 bug"
echo "  2) minor (1.0.0 -> 1.1.0) - 新增功能"
echo "  3) major (1.0.0 -> 2.0.0) - 重大更新"
echo "  4) 跳过版本更新"
echo ""
read -p "请输入选项 (1-4): " version_choice

case $version_choice in
    1)
        npm version patch
        ;;
    2)
        npm version minor
        ;;
    3)
        npm version major
        ;;
    4)
        echo "⏭️  跳过版本更新"
        ;;
    *)
        echo "❌ 无效选项"
        exit 1
        ;;
esac

NEW_VERSION=$(node -p "require('./package.json').version")
echo ""
echo "📦 准备发布版本: $NEW_VERSION"
echo ""

# 显示将要发布的文件
echo "📁 将要发布的文件："
npm pack --dry-run

echo ""
read -p "确认发布? (y/N): " confirm

if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
    echo "❌ 取消发布"
    exit 0
fi

# 发布到 npm
echo ""
echo "🚀 正在发布..."
npm publish --access public

if [ $? -eq 0 ]; then
    echo ""
    echo "🎉 发布成功!"
    echo ""
    echo "📦 包名: @screensniper/locales"
    echo "🏷️  版本: $NEW_VERSION"
    echo "🔗 查看: https://www.npmjs.com/package/@screensniper/locales"
    echo ""
    echo "💡 在主项目中更新:"
    echo "   cd .."
    echo "   npm update @screensniper/locales"
    echo "   npm run install-locales"
else
    echo ""
    echo "❌ 发布失败"
    exit 1
fi
