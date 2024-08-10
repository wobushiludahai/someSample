import akshare as ak
import pandas as pd

# 获取股票热度

# 设置 pandas 选项以显示所有行和列
# pd.set_option('display.max_rows', None)  # 显示所有行
# pd.set_option('display.max_columns', None)  # 显示所有列

stock_hot_follow_xq_df = ak.stock_hot_follow_xq(symbol="最热门")
print(stock_hot_follow_xq_df)