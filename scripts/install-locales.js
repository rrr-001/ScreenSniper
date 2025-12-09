const fs = require('fs');
const path = require('path');

// 源目录和目标目录
const sourceDir = path.join(__dirname, '..', 'node_modules', '@screensniper', 'locales');
const targetDir = path.join(__dirname, '..', 'locales');

console.log('🌐 正在安装翻译文件...');

// 确保目标目录存在
if (!fs.existsSync(targetDir)) {
  fs.mkdirSync(targetDir, { recursive: true });
  console.log('✅ 创建 locales 目录');
}

// 检查源目录是否存在
if (!fs.existsSync(sourceDir)) {
  console.error('❌ 错误: 找不到 @screensniper/locales 包');
  console.log('💡 提示: 请先运行 npm install');
  process.exit(1);
}

// 复制所有 JSON 文件
const files = ['zh.json', 'en.json', 'zhHK.json'];
let copiedCount = 0;

files.forEach(file => {
  const source = path.join(sourceDir, file);
  const target = path.join(targetDir, file);
  
  if (fs.existsSync(source)) {
    fs.copyFileSync(source, target);
    console.log(`✅ 复制: ${file}`);
    copiedCount++;
  } else {
    console.warn(`⚠️  警告: 找不到 ${file}`);
  }
});

console.log(`\n🎉 完成! 成功复制 ${copiedCount}/${files.length} 个翻译文件`);
console.log(`📁 目标位置: ${targetDir}\n`);
