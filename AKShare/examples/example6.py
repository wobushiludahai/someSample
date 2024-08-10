import akshare as ak
import pandas as pd

# 设置 pandas 选项以显示所有行和列
pd.set_option('display.max_rows', None)  # 显示所有行
pd.set_option('display.max_columns', None)  # 显示所有列

# 获取主营业务
stock_zyjs_ths_df = ak.stock_zyjs_ths(symbol="300498")
print(stock_zyjs_ths_df)

# 获取主营构成
stock_zygc_em_df = ak.stock_zygc_em(symbol="SZ300498")
print(stock_zygc_em_df)
