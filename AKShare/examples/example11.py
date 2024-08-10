import akshare as ak
import pandas as pd

# 获取基金持股

# 设置 pandas 选项以显示所有行和列
# pd.set_option('display.max_rows', None)  # 显示所有行
# pd.set_option('display.max_columns', None)  # 显示所有列

stock_report_fund_hold_df = ak.stock_report_fund_hold(symbol="基金持仓", date="20240630")
print(stock_report_fund_hold_df)

# 基金持股明细
# symbol="005827"; 基金代码
stock_report_fund_hold_detail_df = ak.stock_report_fund_hold_detail(symbol="005827", date="20231231")
print(stock_report_fund_hold_detail_df)