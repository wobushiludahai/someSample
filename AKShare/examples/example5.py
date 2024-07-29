import akshare as ak
import pandas as pd

# 设置 pandas 选项以显示所有行和列
pd.set_option('display.max_rows', None)  # 显示所有行
pd.set_option('display.max_columns', None)  # 显示所有列

# 获取某个股票的实时行情数据
def get_stock_realtime(stock_code):
    # stock_code 的格式一般是 "sh600000" 或 "sz000001"，需要根据实际情况填写
    df = ak.stock_zh_a_spot_em()
    stock_data = df[df['代码'] == stock_code]
    return stock_data

# 示例: 获取上证指数的实时行情数据
stock_code = "300498"  # 上证指数的代码
realtime_data = get_stock_realtime(stock_code)
print(realtime_data)