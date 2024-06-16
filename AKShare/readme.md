# AKShare
## 摘要
AKShare学习使用记录
[网址](https://akshare.akfamily.xyz/data/stock/stock.html#id126)
## 环境安装
- 
```
sudo apt install python3-venv
python3 -m venv myenv
source myenv/bin/activate
pip3 install package-name

pip3 freeze > requirements.txt
```
- 使用vscode配置
ctrl+shift+p，然后输入：python create env然后一路选择即可
## 运行
```
python3 run.py
```
## 相关
- akshare
- pandas
- backtrader
```
https://zhuanlan.zhihu.com/p/677056187
```
- PyBroker
- 詹姆斯·哈里斯·西蒙斯
## github
```
https://github.com/Rockyzsu/stock
https://github.com/akfamily/akshare
https://github.com/mpquant/Ashare
```

## 注意
- AKShare 运行时，文件名、文件夹名不能是：akshare
- wsl 运行plot必须使用以下代码：
```
# 显示图例
plt.legend()
 
# 显示图形
plt.show()
```