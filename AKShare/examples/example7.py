import akshare as ak
import pandas as pd

# 获取北向数据

# 设置 pandas 选项以显示所有行和列
pd.set_option('display.max_rows', None)  # 显示所有行
pd.set_option('display.max_columns', None)  # 显示所有列

# 板块数据
stock_hsgt_board_rank_em_df = ak.stock_hsgt_board_rank_em(symbol="北向资金增持行业板块排行", indicator="今日")
print(stock_hsgt_board_rank_em_df)

# 个股数据
stock_em_hsgt_hold_stock_df = ak.stock_hsgt_hold_stock_em(market="北向", indicator="今日排行")
print(stock_em_hsgt_hold_stock_df)

# 每日个股统计
stock_hsgt_stock_statistics_em_df = ak.stock_hsgt_stock_statistics_em(symbol="北向持股", start_date="20240809", end_date="20240809")
print(stock_hsgt_stock_statistics_em_df)